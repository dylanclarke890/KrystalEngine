#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo::html
{
  namespace
  {
    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-tree-order
    void EnqueueUpgradeInShadowIncludingTreeOrder(dom::ContainerNode &node,
                                                  CustomElementInterface &elementInterface,
                                                  CustomElementRegistry &registry) noexcept
    {
      for (auto &descendant : dom::ShadowIncludingDescendantRange(node))
      {
        auto *element = DynamicDowncast<dom::Element>(descendant);
        if (element != nullptr && element->IsCustomElementUpgradeCandidate()
            && element->CustomElementRegistry().get() == &registry
            && element->TagQualifiedName().LocalName() == elementInterface.Name().LocalName()
            && element->NamespaceURI() == elementInterface.Name().NamespaceURI())
        {
          element->EnqueueToUpgrade(elementInterface);
        }
      }
    }

    void UpgradeElementIfPossible(CustomElementRegistry &registry, dom::Element &element) noexcept
    {
      if (element.IsCustomElementUpgradeCandidate() && element.CustomElementRegistry().get() == &registry)
      {
        CustomElementReactionQueue::TryToUpgradeElement(element);
      }
    }
  }

  CustomElementRegistry::CustomElementRegistry(dom::Document &document) noexcept
      : _document(CreateWeakPtr(&document))
  {
  }

  CustomElementRegistry::CustomElementRegistry() noexcept : _isScoped(true)
  {
  }

  CustomElementRegistry::~CustomElementRegistry() noexcept = default;

  dom::Document *CustomElementRegistry::Document() const noexcept
  {
    return _document.get();
  }

  dom::ExceptionOr<void> CustomElementRegistry::Define(dom::DOMStringAtom name,
                                                       CustomElementFactory &&factory,
                                                       CustomElementOptions &&options) noexcept
  {
    CEReactionsScope ceReactions;

    if (!dom::NameValidation::IsValidCustomElementName(name.View()))
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    if (_elementDefinitionIsRunning)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    SetForScope change(_elementDefinitionIsRunning, true);

    if (FindInterface(name) != nullptr)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    AddElementDefinition(CustomElementInterface::Create(name, krys::move(factory), krys::move(options)));
    return {};
  }

  void CustomElementRegistry::AddElementDefinition(Ref<CustomElementInterface> &&elementInterface) noexcept
  {
    auto localName = elementInterface->Name().LocalName();
    krys_debug_assert(!_nameMap.contains(localName));
    _nameMap.emplace(localName, elementInterface);

    if (auto *document = Document()) // Global custom element registry
    {
      EnqueueUpgradeInShadowIncludingTreeOrder(*document, *elementInterface, *this);
    }

    for (auto &document : _scopedDocumentSet)
    {
      if (document)
      {
        EnqueueUpgradeInShadowIncludingTreeOrder(*document, *elementInterface, *this);
      }
    }

    if (auto it = _whenDefinedMap.find(localName); it != _whenDefinedMap.end())
    {
      auto callbacks = krys::move(it->second);
      _whenDefinedMap.erase(it);
      EventLoop::Current().QueueMicrotask(
        [callbacks = krys::move(callbacks), elementInterface = elementInterface]() mutable
        {
          for (auto &callback : callbacks)
          {
            callback(*elementInterface);
          }
        });
    }
  }

  CustomElementInterface *CustomElementRegistry::FindInterface(const dom::QualifiedName &name) const noexcept
  {
    if (name.NamespaceURI() != infra::Namespaces::HTML)
    {
      return nullptr;
    }
    return FindInterface(name.LocalName());
  }

  CustomElementInterface *CustomElementRegistry::FindInterface(dom::DOMStringAtom name) const noexcept
  {
    auto it = _nameMap.find(name);
    return it != _nameMap.end() ? it->second.get() : nullptr;
  }

  CustomElementInterface *CustomElementRegistry::Get(dom::DOMStringAtom name) const noexcept
  {
    return FindInterface(name);
  }

  dom::DOMStringAtom
    CustomElementRegistry::GetName(const CustomElementInterface &elementInterface) const noexcept
  {
    auto *found = FindInterface(elementInterface.Name().LocalName());
    return found == &elementInterface ? elementInterface.Name().LocalName() : dom::DOMStringAtom::Null();
  }

  dom::ExceptionOr<void> CustomElementRegistry::WhenDefined(dom::DOMStringAtom name,
                                                            WhenDefinedCallback &&callback) noexcept
  {
    if (!dom::NameValidation::IsValidCustomElementName(name.View()))
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    if (auto *elementInterface = FindInterface(name))
    {
      EventLoop::Current().QueueMicrotask(
        [callback = krys::move(callback), elementInterface = ShareRef(*elementInterface)]() mutable
        { callback(*elementInterface); });
      return {};
    }

    _whenDefinedMap[name].push_back(krys::move(callback));
    return {};
  }

  void CustomElementRegistry::Upgrade(dom::Node &root) noexcept
  {
    CEReactionsScope ceReactions;

    auto *containerNode = DynamicDowncast<dom::ContainerNode>(root);
    if (containerNode == nullptr)
    {
      return;
    }

    if (auto *element = DynamicDowncast<dom::Element>(*containerNode);
        element != nullptr && element->IsCustomElementUpgradeCandidate())
    {
      CustomElementReactionQueue::TryToUpgradeElement(*element);
    }

    for (auto &descendant : dom::ShadowIncludingDescendantRange(*containerNode))
    {
      if (auto *element = DynamicDowncast<dom::Element>(descendant);
          element != nullptr && element->IsCustomElementUpgradeCandidate())
      {
        CustomElementReactionQueue::TryToUpgradeElement(*element);
      }
    }
  }

  dom::ExceptionOr<void> CustomElementRegistry::Initialize(dom::Node &root) noexcept
  {
    CEReactionsScope ceReactions;

    if (!IsScoped() && (Is<dom::Document>(root) || this != root.NodeDocument().CustomElementRegistry().get()))
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    if (auto *document = DynamicDowncast<dom::Document>(root); document != nullptr)
    {
      if (document->CustomElementRegistry() == nullptr)
      {
        document->SetCustomElementRegistry(ShareRef(*this));
      }
    }
    else if (auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(root); shadowRoot != nullptr)
    {
      if (shadowRoot->CustomElementRegistry() == nullptr)
      {
        shadowRoot->SetCustomElementRegistry(ShareRef(*this));
      }
    }

    auto updateRegistryIfNeeded = [&](dom::Element &element)
    {
      if (element.CustomElementRegistry() == nullptr)
      {
        element.SetCustomElementRegistry(ShareRef(*this));
        if (IsScoped())
        {
          AppendToScopedDocumentSet(element.NodeDocument());
        }
      }
    };

    if (auto *element = DynamicDowncast<dom::Element>(root))
    {
      updateRegistryIfNeeded(*element);
      UpgradeElementIfPossible(*this, *element);
    }

    if (auto *containerRoot = DynamicDowncast<dom::ContainerNode>(root))
    {
      for (auto &descendant : dom::DescendantRange(*containerRoot))
      {
        if (auto *element = DynamicDowncast<dom::Element>(descendant))
        {
          updateRegistryIfNeeded(*element);
          UpgradeElementIfPossible(*this, *element);
        }
      }
    }

    return {};
  }

  void CustomElementRegistry::AppendToScopedDocumentSet(dom::Document &document) noexcept
  {
    krys_debug_assert(IsScoped());
    for (auto &member : _scopedDocumentSet)
    {
      if (member.get() == &document)
      {
        return;
      }
    }
    std::erase_if(_scopedDocumentSet, [](const WeakPtr<dom::Document> &member) { return !member; });
    _scopedDocumentSet.push_back(CreateWeakPtr(&document));
  }
}
