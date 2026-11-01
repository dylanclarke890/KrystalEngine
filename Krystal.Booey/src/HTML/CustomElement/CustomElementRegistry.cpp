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
    /// @see https://html.spec.whatwg.org/#upgrade-particular-elements-within-a-document
    void EnqueueUpgradeInShadowIncludingTreeOrder(dom::ContainerNode &node,
                                                  CustomElementInterface &elementInterface,
                                                  CustomElementRegistry &registry) noexcept
    {
      // 1. Let upgradeCandidates be all elements that are shadow-including descendants of document, whose
      //    custom element registry is registry, whose namespace is the HTML namespace, and whose local name
      //    is localName, in shadow-including tree order. [...]
      // 2. For each element element of upgradeCandidates: enqueue a custom element upgrade reaction given
      //    element and definition.
      // (Only undefined elements, as the others would return from upgrading.)
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

    /// @brief Tries to upgrade @p element if its registry is @p registry (and it's undefined, which only an
    /// element that can be upgraded is).
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

  // https://html.spec.whatwg.org/#dom-customelementregistry-define
  dom::ExceptionOr<void> CustomElementRegistry::Define(dom::DOMStringAtom name,
                                                       CustomElementFactory &&factory,
                                                       CustomElementOptions &&options) noexcept
  {
    CEReactionsScope ceReactions;

    // 1. If IsConstructor(constructor) is false, then throw a TypeError. (Skipped: the factory constructs.)

    // 2. If name is not a valid custom element name, then throw a "SyntaxError" DOMException.
    if (!dom::NameValidation::IsValidCustomElementName(name.View()))
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    // 3. If this's custom element definition set contains an item with name name, then throw a
    //    "NotSupportedError" DOMException.
    if (FindInterface(name) != nullptr)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // 4. If this's custom element definition set contains an item with constructor constructor, then throw a
    //    "NotSupportedError" DOMException. (Skipped: factories can't be compared.)
    // 5. Let localName be name.
    // 6. Let extends be options["extends"] if it exists; otherwise null.
    // 7. If extends is not null: [...] (Skipped: there are no customized built-in elements.)

    // 8. If this's element definition is running is true, then throw a "NotSupportedError" DOMException.
    if (_elementDefinitionIsRunning)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // 9. Set this's element definition is running to true.
    // 14. Run the following steps while catching any exceptions: [...] Then, regardless of whether the above
    //     steps threw an exception or not: set this's element definition is running to false.
    SetForScope change(_elementDefinitionIsRunning, true);

    // 10. Let formAssociated be false.
    // 11. Let disableInternals be false.
    // 12. Let disableShadow be false.
    // 13. Let observedAttributes be an empty sequence<DOMString>.
    // 14. [...] (The options give what the constructor's properties would, and the lifecycle callbacks are
    // the
    //     behaviour's virtual methods.)
    // 15. Let definition be a new custom element definition with name name, local name localName, constructor
    //     constructor, observed attributes observedAttributes, lifecycle callbacks lifecycleCallbacks,
    //     form-associated formAssociated, disable internals disableInternals, and disable shadow
    //     disableShadow.
    AddElementDefinition(CustomElementInterface::Create(name, krys::move(factory), krys::move(options)));
    return {};
  }

  void CustomElementRegistry::AddElementDefinition(Ref<CustomElementInterface> &&elementInterface) noexcept
  {
    // 16. Append definition to this's custom element definition set.
    auto localName = elementInterface->Name().LocalName();
    krys_debug_assert(!_nameMap.contains(localName));
    _nameMap.emplace(localName, elementInterface);

    // 18. Otherwise, upgrade particular elements within a document given this, this's relevant global
    // object's
    //     associated Document, definition, localName, and name.
    if (auto *document = Document()) // Global custom element registry
    {
      EnqueueUpgradeInShadowIncludingTreeOrder(*document, *elementInterface, *this);
    }

    // 17. If this's is scoped is true, then for each document of this's scoped document set: upgrade
    //     particular elements within a document given this, document, definition, and localName.
    for (auto &document : _scopedDocumentSet)
    {
      if (document)
      {
        EnqueueUpgradeInShadowIncludingTreeOrder(*document, *elementInterface, *this);
      }
    }

    // 19. If this's when-defined promise map[name] exists:
    // 19a. Resolve this's when-defined promise map[name] with constructor.
    // 19b. Remove this's when-defined promise map[name].
    // (Its callbacks are called in a microtask, as a resolved promise's reactions would be.)
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

  // https://html.spec.whatwg.org/#dom-customelementregistry-get
  CustomElementInterface *CustomElementRegistry::Get(dom::DOMStringAtom name) const noexcept
  {
    // 1. If this's custom element definition set contains an item with name name, then return that item's
    //    constructor.
    // 2. Return undefined.
    return FindInterface(name);
  }

  // https://html.spec.whatwg.org/#dom-customelementregistry-getname
  // (Given the definition rather than its constructor.)
  dom::DOMStringAtom
    CustomElementRegistry::GetName(const CustomElementInterface &elementInterface) const noexcept
  {
    // 1. If this's custom element definition set contains an item with constructor constructor, then return
    //    that item's name.
    // 2. Return null.
    auto *found = FindInterface(elementInterface.Name().LocalName());
    return found == &elementInterface ? elementInterface.Name().LocalName() : dom::DOMStringAtom::Null();
  }

  // https://html.spec.whatwg.org/#dom-customelementregistry-whendefined
  // (The promise is a callback, called in a microtask once the name is defined.)
  dom::ExceptionOr<void> CustomElementRegistry::WhenDefined(dom::DOMStringAtom name,
                                                            WhenDefinedCallback &&callback) noexcept
  {
    // 1. If name is not a valid custom element name, then return a promise rejected with a "SyntaxError"
    //    DOMException.
    if (!dom::NameValidation::IsValidCustomElementName(name.View()))
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    // 2. If this's custom element definition set contains an item with name name, then return a promise
    //    resolved with that item's constructor.
    if (auto *elementInterface = FindInterface(name))
    {
      EventLoop::Current().QueueMicrotask(
        [callback = krys::move(callback), elementInterface = ShareRef(*elementInterface)]() mutable
        { callback(*elementInterface); });
      return {};
    }

    // 3. If this's when-defined promise map[name] does not exist, then set this's when-defined promise
    //    map[name] to a new promise.
    // 4. Return this's when-defined promise map[name].
    _whenDefinedMap[name].push_back(krys::move(callback));
    return {};
  }

  // https://html.spec.whatwg.org/#dom-customelementregistry-upgrade
  void CustomElementRegistry::Upgrade(dom::Node &root) noexcept
  {
    CEReactionsScope ceReactions;

    auto *containerNode = DynamicDowncast<dom::ContainerNode>(root);
    if (containerNode == nullptr)
    {
      return;
    }

    // 1. For each shadow-including inclusive descendant candidate of root, in shadow-including tree order:
    // 1a. If candidate is not an Element node, then continue.
    // 1b. If candidate's custom element registry is not this, then continue.
    // 1c. Try to upgrade candidate.
    if (auto *element = DynamicDowncast<dom::Element>(*containerNode))
    {
      UpgradeElementIfPossible(*this, *element);
    }

    for (auto &descendant : dom::ShadowIncludingDescendantRange(*containerNode))
    {
      if (auto *element = DynamicDowncast<dom::Element>(descendant))
      {
        UpgradeElementIfPossible(*this, *element);
      }
    }
  }

  // https://html.spec.whatwg.org/#dom-customelementregistry-initialize
  dom::ExceptionOr<void> CustomElementRegistry::Initialize(dom::Node &root) noexcept
  {
    CEReactionsScope ceReactions;

    // 1. If this's is scoped is false and either root is a Document node or root's node document's custom
    //    element registry is not this, then throw a "NotSupportedError" DOMException.
    if (!IsScoped() && (Is<dom::Document>(root) || this != root.NodeDocument().CustomElementRegistry().get()))
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // 2. If root is a Document node whose custom element registry is null, then set root's custom element
    //    registry to this.
    // 3. Otherwise, if root is a ShadowRoot node whose custom element registry is null, then set root's
    // custom
    //    element registry to this.
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

    // 4. For each inclusive descendant inclusiveDescendant of root, in tree order:
    // 4a. If inclusiveDescendant is not an Element node, then continue.
    // 4b. If inclusiveDescendant's custom element registry is null:
    // 4b.i. Set inclusiveDescendant's custom element registry to this.
    // 4b.ii. If this's is scoped is true, then append inclusiveDescendant's node document to this's scoped
    //        document set.
    // 4c. If inclusiveDescendant's custom element registry is not this, then continue.
    // 4d. Try to upgrade inclusiveDescendant.
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
