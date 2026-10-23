#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::html
{
  class CustomElementInterface;
  class HTMLFormElement;

  /// @brief A custom element reaction: an upgrade, or a callback with its arguments.
  /// @see https://html.spec.whatwg.org/#custom-element-reactions
  class CustomElementReactionQueueItem
  {
    KRYS_NON_COPYABLE(CustomElementReactionQueueItem);

  public:
    enum class Type : uint8
    {
      Invalid,
      ElementUpgrade,
      Connected,
      Disconnected,
      ConnectedMove,
      Adopted,
      AttributeChanged,
      FormAssociated,
      FormReset,
      FormDisabled,
    };

    struct AdoptedPayload
    {
      Ref<dom::Document> OldDocument;
      Ref<dom::Document> NewDocument;
    };

    struct AttributeChangedPayload
    {
      dom::QualifiedName Name;
      dom::DOMStringAtom OldValue;
      dom::DOMStringAtom NewValue;
    };

    struct FormAssociatedPayload
    {
      RefPtr<HTMLFormElement> Form;

      ~FormAssociatedPayload() noexcept;
    };

    using FormDisabledPayload = bool;
    using Payload =
      Maybe<Variant<AdoptedPayload, AttributeChangedPayload, FormAssociatedPayload, FormDisabledPayload>>;

  private:
    Type _type {Type::Invalid};
    Payload _payload;

  public:
    CustomElementReactionQueueItem() noexcept = default;
    CustomElementReactionQueueItem(CustomElementReactionQueueItem &&) noexcept;
    CustomElementReactionQueueItem &operator=(CustomElementReactionQueueItem &&) noexcept;
    CustomElementReactionQueueItem(Type type, Payload &&payload = null) noexcept;
    ~CustomElementReactionQueueItem() noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    void Invoke(dom::Element &element, CustomElementInterface &elementInterface) noexcept;
  };

  /// @see https://html.spec.whatwg.org/#element-queue
  class CustomElementQueue
  {
    KRYS_NON_COPY_MOVABLE(CustomElementQueue);

  private:
    List<Ref<dom::Element>> _elements;
    bool _invoking {false};

  public:
    CustomElementQueue() noexcept = default;
    ~CustomElementQueue() noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _elements.empty();
    }

    void Add(dom::Element &element) noexcept;

    /// @brief Invokes the reactions of the queue's elements, in order, and empties it.
    /// @see https://html.spec.whatwg.org/#invoke-custom-element-reactions
    void ProcessQueue() noexcept;

    KRYS_NODISCARD List<Ref<dom::Element>> TakeElements() noexcept;

  private:
    void InvokeAll() noexcept;
  };

  /// @brief An element's custom element reaction queue, with the definition it was enqueued to upgrade to or
  /// was defined by.
  /// @see https://html.spec.whatwg.org/#custom-element-reaction-queue
  class CustomElementReactionQueue
  {
    KRYS_NON_COPY_MOVABLE(CustomElementReactionQueue);

    using Item = CustomElementReactionQueueItem;

  private:
    const Ref<CustomElementInterface> _interface;
    List<Item> _items;
    bool _elementInternalsAttached {false};

  public:
    explicit CustomElementReactionQueue(CustomElementInterface &elementInterface) noexcept;
    ~CustomElementReactionQueue() noexcept;

    /// @see https://html.spec.whatwg.org/#enqueue-a-custom-element-upgrade-reaction
    static void EnqueueElementUpgrade(dom::Element &element, bool alreadyScheduledToUpgrade) noexcept;

    /// @see https://html.spec.whatwg.org/#concept-try-upgrade
    static void TryToUpgradeElement(dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#enqueue-a-custom-element-callback-reaction
    static void EnqueueConnectedCallbackIfNeeded(dom::Element &element) noexcept;
    static void EnqueueDisconnectedCallbackIfNeeded(dom::Element &element) noexcept;
    static void EnqueueConnectedMoveCallbackIfNeeded(dom::Element &element) noexcept;
    static void EnqueueAdoptedCallbackIfNeeded(dom::Element &element, dom::Document &oldDocument,
                                               dom::Document &newDocument) noexcept;
    static void EnqueueAttributeChangedCallbackIfNeeded(dom::Element &element,
                                                        const dom::QualifiedName &attributeName,
                                                        dom::DOMStringAtom oldValue,
                                                        dom::DOMStringAtom newValue) noexcept;
    static void EnqueueFormAssociatedCallbackIfNeeded(dom::Element &element, HTMLFormElement *form) noexcept;
    static void EnqueueFormDisabledCallbackIfNeeded(dom::Element &element, bool isDisabled) noexcept;
    static void EnqueueFormResetCallbackIfNeeded(dom::Element &element) noexcept;

    /// @brief Enqueues the callbacks an upgrade implies: an attribute change for each observed attribute the
    /// element has, then the connection if it's connected.
    /// @see https://html.spec.whatwg.org/#concept-upgrade-an-element (steps 4 and 5)
    static void EnqueuePostUpgradeReactions(dom::Element &element) noexcept;

    KRYS_NODISCARD bool ObservesStyleAttribute() const noexcept;
    KRYS_NODISCARD bool IsElementInternalsDisabled() const noexcept;
    KRYS_NODISCARD bool IsElementInternalsAttached() const noexcept;
    void SetElementInternalsAttached() noexcept;
    KRYS_NODISCARD bool IsFormAssociated() const noexcept;

    /// @brief The definition the element is defined by or is to be upgraded to.
    KRYS_NODISCARD CustomElementInterface &Interface() const noexcept
    {
      return *_interface;
    }

    void InvokeAll(dom::Element &element) noexcept;
    void Clear() noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _items.empty();
    }

    KRYS_NODISCARD bool HasJustUpgradeReaction() const noexcept;

    /// @see https://html.spec.whatwg.org/#backup-element-queue
    static void ProcessBackupQueue(CustomElementQueue &backupElementQueue) noexcept;

    static void EnqueueElementsOnAppropriateElementQueue(const List<Ref<dom::Element>> &elements) noexcept;

  private:
    /// @see https://html.spec.whatwg.org/#enqueue-an-element-on-the-appropriate-element-queue
    static void EnqueueElementOnAppropriateElementQueue(dom::Element &element) noexcept;
  };

  /// @brief While one is alive (in debug builds), no custom element reactions may be enqueued: the scopes of
  /// algorithms during which a callback would see the tree half changed.
  class CustomElementReactionDisallowedScope
  {
    KRYS_NON_COPY_MOVABLE(CustomElementReactionDisallowedScope);

  private:
    static uint32 s_customElementReactionDisallowedCount;

  public:
    CustomElementReactionDisallowedScope() noexcept
    {
      ++s_customElementReactionDisallowedCount;
    }

    ~CustomElementReactionDisallowedScope() noexcept
    {
      krys_debug_assert(s_customElementReactionDisallowedCount != 0);
      --s_customElementReactionDisallowedCount;
    }

    KRYS_NODISCARD static bool IsReactionAllowed() noexcept
    {
      return s_customElementReactionDisallowedCount == 0;
    }

    /// @brief Allows reactions again within a disallowed scope.
    class AllowedScope
    {
      KRYS_NON_COPY_MOVABLE(AllowedScope);

    private:
      uint32 _originalCount;

    public:
      AllowedScope() noexcept : _originalCount(s_customElementReactionDisallowedCount)
      {
        s_customElementReactionDisallowedCount = 0;
      }

      ~AllowedScope() noexcept
      {
        s_customElementReactionDisallowedCount = _originalCount;
      }
    };
  };

  /// @brief An element queue pushed onto the custom element reactions stack for the scope's lifetime; its
  /// reactions are invoked when it's popped.
  /// @see https://html.spec.whatwg.org/#custom-element-reactions-stack
  class CustomElementReactionStack : public CustomElementReactionDisallowedScope::AllowedScope
  {
    KRYS_NON_COPY_MOVABLE(CustomElementReactionStack);

    friend class CustomElementReactionQueue;
    friend class CustomElementReactionStackSuspension;

  private:
    static CustomElementReactionStack *s_currentProcessingStack;

    CustomElementQueue _queue;
    CustomElementReactionStack *const _previousProcessingStack;

  public:
    CustomElementReactionStack() noexcept : _previousProcessingStack(s_currentProcessingStack)
    {
      s_currentProcessingStack = this;
    }

    ~CustomElementReactionStack() noexcept
    {
      if (!_queue.IsEmpty()) KRYS_UNLIKELY
      {
        _queue.ProcessQueue();
      }
      s_currentProcessingStack = _previousProcessingStack;
    }

    /// @brief Whether an element queue is on the stack: reactions enqueued now go to it, rather than to the
    /// backup element queue.
    KRYS_NODISCARD static bool HasCurrentProcessingStack() noexcept
    {
      return s_currentProcessingStack != nullptr;
    }

    KRYS_NODISCARD List<Ref<dom::Element>> TakeElements() noexcept
    {
      return _queue.TakeElements();
    }
  };

  /// @brief The `[CEReactions]` extended attribute, for the C++ methods it marks: pushes an element queue
  /// unless one is on the stack, so that the reactions a call causes are invoked before it returns. Methods
  /// the host calls while Booey is running (in a reaction or an event listener) are given their own queue by
  /// `CustomElementReactionStackSuspension`.
  /// @see https://html.spec.whatwg.org/#cereactions
  class CEReactionsScope
  {
    KRYS_NON_COPY_MOVABLE(CEReactionsScope);

  private:
    Maybe<CustomElementReactionStack> _stack;

  public:
    CEReactionsScope() noexcept
    {
      if (!CustomElementReactionStack::HasCurrentProcessingStack())
      {
        _stack.emplace();
      }
    }
  };

  /// @brief Takes the custom element reactions stack's queues away while Booey calls host code (a custom
  /// element callback, an event listener), as script runs with its own: the `[CEReactions]` methods it calls
  /// invoke their reactions before they return.
  class CustomElementReactionStackSuspension
  {
    KRYS_NON_COPY_MOVABLE(CustomElementReactionStackSuspension);

  private:
    CustomElementReactionStack *_suspendedStack;

  public:
    CustomElementReactionStackSuspension() noexcept
        : _suspendedStack(CustomElementReactionStack::s_currentProcessingStack)
    {
      CustomElementReactionStack::s_currentProcessingStack = nullptr;
    }

    ~CustomElementReactionStackSuspension() noexcept
    {
      CustomElementReactionStack::s_currentProcessingStack = _suspendedStack;
    }
  };
}
