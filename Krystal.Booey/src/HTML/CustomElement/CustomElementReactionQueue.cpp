#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo::html
{
  uint32 CustomElementReactionDisallowedScope::s_customElementReactionDisallowedCount = 0;

  CustomElementReactionStack *CustomElementReactionStack::s_currentProcessingStack = nullptr;

  namespace
  {
    /// @brief The backup element queue, and whether a microtask to process it is queued.
    /// FIXME: One per similar-origin window agent, once there are agents (as the event loop).
    struct BackupElementQueue
    {
      CustomElementQueue Queue;
      bool ProcessingBackupElementQueue {false};
    };

    BackupElementQueue &GetBackupElementQueueState() noexcept
    {
      static BackupElementQueue state;
      return state;
    }

    /// @see https://html.spec.whatwg.org/#enqueue-an-element-on-the-appropriate-element-queue (step 2)
    CustomElementQueue &BackupElementQueueForEnqueue() noexcept
    {
      auto &state = GetBackupElementQueueState();
      if (!state.ProcessingBackupElementQueue)
      {
        state.ProcessingBackupElementQueue = true;
        EventLoop::Current().QueueMicrotask(
          []
          {
            auto &state = GetBackupElementQueueState();
            state.ProcessingBackupElementQueue = false;
            CustomElementReactionQueue::ProcessBackupQueue(state.Queue);
          });
      }
      return state.Queue;
    }
  }

  CustomElementReactionQueueItem::CustomElementReactionQueueItem(Type type, Payload &&payload) noexcept
      : _type(type), _payload(krys::move(payload))
  {
  }

  CustomElementReactionQueueItem::FormAssociatedPayload::~FormAssociatedPayload() noexcept = default;

  CustomElementReactionQueueItem::CustomElementReactionQueueItem(CustomElementReactionQueueItem &&) noexcept =
    default;

  CustomElementReactionQueueItem &
    CustomElementReactionQueueItem::operator=(CustomElementReactionQueueItem &&) noexcept = default;

  CustomElementReactionQueueItem::~CustomElementReactionQueueItem() noexcept = default;

  void CustomElementReactionQueueItem::Invoke(dom::Element &element,
                                              CustomElementInterface &elementInterface) noexcept
  {
    switch (_type)
    {
      case Type::Invalid: krys_debug_assert_msg(false, "an invalid reaction was enqueued"); break;
      case Type::ElementUpgrade:
        krys_debug_assert(!_payload.has_value());
        elementInterface.UpgradeElement(element);
        break;
      case Type::Connected:
        krys_debug_assert(!_payload.has_value());
        elementInterface.InvokeConnectedCallback(element);
        break;
      case Type::Disconnected:
        krys_debug_assert(!_payload.has_value());
        elementInterface.InvokeDisconnectedCallback(element);
        break;
      case Type::ConnectedMove:
        krys_debug_assert(!_payload.has_value());
        elementInterface.InvokeConnectedMoveCallback(element);
        break;
      case Type::Adopted:
      {
        krys_debug_assert(_payload.has_value() && std::holds_alternative<AdoptedPayload>(*_payload));
        auto &payload = std::get<AdoptedPayload>(*_payload);
        elementInterface.InvokeAdoptedCallback(element, *payload.OldDocument, *payload.NewDocument);
        break;
      }
      case Type::AttributeChanged:
      {
        krys_debug_assert(_payload.has_value() && std::holds_alternative<AttributeChangedPayload>(*_payload));
        auto &payload = std::get<AttributeChangedPayload>(*_payload);
        elementInterface.InvokeAttributeChangedCallback(element, payload.Name, payload.OldValue,
                                                        payload.NewValue);
        break;
      }
      case Type::FormAssociated:
        krys_debug_assert(_payload.has_value() && std::holds_alternative<FormAssociatedPayload>(*_payload));
        elementInterface.InvokeFormAssociatedCallback(element,
                                                      std::get<FormAssociatedPayload>(*_payload).Form.get());
        break;
      case Type::FormReset:
        krys_debug_assert(!_payload.has_value());
        elementInterface.InvokeFormResetCallback(element);
        break;
      case Type::FormDisabled:
        krys_debug_assert(_payload.has_value() && std::holds_alternative<FormDisabledPayload>(*_payload));
        elementInterface.InvokeFormDisabledCallback(element, std::get<FormDisabledPayload>(*_payload));
        break;
    }
  }

  CustomElementQueue::~CustomElementQueue() noexcept
  {
    krys_debug_assert(IsEmpty());
  }

  void CustomElementQueue::Add(dom::Element &element) noexcept
  {
    krys_debug_assert(!_invoking);
    // FIXME: Avoid inserting the same element multiple times.
    _elements.push_back(ShareRef(element));
  }

  void CustomElementQueue::InvokeAll() noexcept
  {
    krys_assert(!_invoking);
    SetForScope invoking(_invoking, true);
    // It's possible for more elements to be enqueued if some methods were missing their reaction scopes.
    // Invoke callbacks slightly later here instead of crashing or ignoring those cases.
    for (size_t i = 0; i < _elements.size(); ++i)
    {
      Ref element = _elements[i];
      auto *queue = element->ReactionQueue();
      krys_debug_assert(queue != nullptr);
      queue->InvokeAll(*element);
    }
    _elements.clear();
  }

  void CustomElementQueue::ProcessQueue() noexcept
  {
    InvokeAll();
  }

  List<Ref<dom::Element>> CustomElementQueue::TakeElements() noexcept
  {
    krys_assert(!_invoking);
    return std::exchange(_elements, {});
  }

  CustomElementReactionQueue::CustomElementReactionQueue(CustomElementInterface &elementInterface) noexcept
      : _interface(ShareRef(elementInterface))
  {
  }

  CustomElementReactionQueue::~CustomElementReactionQueue() noexcept = default;

  void CustomElementReactionQueue::Clear() noexcept
  {
    _items.clear();
  }

  bool CustomElementReactionQueue::HasJustUpgradeReaction() const noexcept
  {
    return _items.size() == 1 && _items[0].GetType() == Item::Type::ElementUpgrade;
  }

  void CustomElementReactionQueue::EnqueueElementUpgrade(dom::Element &element,
                                                         bool alreadyScheduledToUpgrade) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    if (alreadyScheduledToUpgrade)
    {
      krys_debug_assert(queue.HasJustUpgradeReaction());
    }
    else
    {
      queue._items.emplace_back(Item::Type::ElementUpgrade);
    }
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::TryToUpgradeElement(dom::Element &element) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.IsCustomElementUpgradeCandidate());
    auto registry = element.CustomElementRegistry();
    if (registry == nullptr)
    {
      return;
    }

    auto *elementInterface = registry->FindInterface(element.TagQualifiedName());
    if (elementInterface == nullptr)
    {
      return;
    }

    element.EnqueueToUpgrade(*elementInterface);
  }

  void CustomElementReactionQueue::EnqueueConnectedCallbackIfNeeded(dom::Element &element) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.IsDefinedCustomElement());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    queue._items.emplace_back(Item::Type::Connected);
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::EnqueueDisconnectedCallbackIfNeeded(dom::Element &element) noexcept
  {
    krys_debug_assert(element.IsDefinedCustomElement());
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    queue._items.emplace_back(Item::Type::Disconnected);
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::EnqueueConnectedMoveCallbackIfNeeded(dom::Element &element) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.IsDefinedCustomElement());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    queue._items.emplace_back(Item::Type::ConnectedMove);
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::EnqueueAdoptedCallbackIfNeeded(dom::Element &element,
                                                                  dom::Document &oldDocument,
                                                                  dom::Document &newDocument) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.IsDefinedCustomElement());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    queue._items.emplace_back(Item::Type::Adopted,
                              Item::AdoptedPayload {ShareRef(oldDocument), ShareRef(newDocument)});
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::EnqueueAttributeChangedCallbackIfNeeded(
    dom::Element &element, const dom::QualifiedName &attributeName, dom::DOMStringAtom oldValue,
    dom::DOMStringAtom newValue) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.IsDefinedCustomElement());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    if (!queue._interface->ObservesAttribute(attributeName.LocalName()))
    {
      return;
    }
    queue._items.emplace_back(Item::Type::AttributeChanged,
                              Item::AttributeChangedPayload {attributeName, oldValue, newValue});
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::EnqueueFormAssociatedCallbackIfNeeded(dom::Element &element,
                                                                         HTMLFormElement *form) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    krys_debug_assert(queue.IsFormAssociated());
    queue._items.emplace_back(Item::Type::FormAssociated, Item::FormAssociatedPayload {ShareRefPtr(form)});
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::EnqueueFormResetCallbackIfNeeded(dom::Element &element) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    krys_debug_assert(queue.IsFormAssociated());
    queue._items.emplace_back(Item::Type::FormReset);
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::EnqueueFormDisabledCallbackIfNeeded(dom::Element &element,
                                                                       bool isDisabled) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();
    krys_debug_assert(queue.IsFormAssociated());
    queue._items.emplace_back(Item::Type::FormDisabled, Item::FormDisabledPayload {isDisabled});
    EnqueueElementOnAppropriateElementQueue(element);
  }

  void CustomElementReactionQueue::EnqueuePostUpgradeReactions(dom::Element &element) noexcept
  {
    krys_debug_assert(CustomElementReactionDisallowedScope::IsReactionAllowed());
    krys_debug_assert(element.IsCustomElementUpgradeCandidate());
    if (!element.HasAttributes() && !element.IsConnected())
    {
      return;
    }

    krys_debug_assert(element.ReactionQueue() != nullptr);
    auto &queue = *element.ReactionQueue();

    element.SynchronizeAllAttributes();
    for (auto &attribute : element.AttributeList())
    {
      if (queue._interface->ObservesAttribute(attribute->LocalName()))
      {
        queue._items.emplace_back(Item::Type::AttributeChanged,
                                  Item::AttributeChangedPayload {attribute->GetQualifiedName(),
                                                                 dom::DOMStringAtom::Null(),
                                                                 dom::DOMStringAtom(attribute->Value())});
      }
    }

    if (element.IsConnected())
    {
      queue._items.emplace_back(Item::Type::Connected);
    }
  }

  bool CustomElementReactionQueue::ObservesStyleAttribute() const noexcept
  {
    return _interface->ObservesAttribute(u8"style");
  }

  bool CustomElementReactionQueue::IsElementInternalsDisabled() const noexcept
  {
    return _interface->IsElementInternalsDisabled();
  }

  bool CustomElementReactionQueue::IsFormAssociated() const noexcept
  {
    return _interface->IsFormAssociated();
  }

  bool CustomElementReactionQueue::IsElementInternalsAttached() const noexcept
  {
    return _elementInternalsAttached;
  }

  void CustomElementReactionQueue::SetElementInternalsAttached() noexcept
  {
    _elementInternalsAttached = true;
  }

  void CustomElementReactionQueue::InvokeAll(dom::Element &element) noexcept
  {
    while (!_items.empty())
    {
      auto items = std::exchange(_items, {});
      for (auto &item : items)
      {
        item.Invoke(element, *_interface);
      }
    }
  }

  void CustomElementReactionQueue::EnqueueElementsOnAppropriateElementQueue(
    const List<Ref<dom::Element>> &elements) noexcept
  {
    for (auto &element : elements)
    {
      EnqueueElementOnAppropriateElementQueue(*element);
    }
  }

  void CustomElementReactionQueue::EnqueueElementOnAppropriateElementQueue(dom::Element &element) noexcept
  {
    krys_debug_assert(element.ReactionQueue() != nullptr);
    if (CustomElementReactionStack::s_currentProcessingStack == nullptr)
    {
      BackupElementQueueForEnqueue().Add(element);
      return;
    }

    CustomElementReactionStack::s_currentProcessingStack->_queue.Add(element);
  }

  void CustomElementReactionQueue::ProcessBackupQueue(CustomElementQueue &backupElementQueue) noexcept
  {
    backupElementQueue.ProcessQueue();
  }
}
