#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Booey/Style/Invalidation/AttributeChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Invalidation/ClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Invalidation/IdChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassStateInvalidation.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <utility>

namespace krys::boo::dom
{
  namespace
  {
    /// @brief Invalidates the style that a change to one of an element's attributes affects, before the
    /// change (when constructed) and after it (when destroyed): rules with attribute selectors, the element's
    /// classes and ID, and the pseudo-class states derived from the attribute.
    class AttributeChangeStyleInvalidation
    {
      KRYS_NON_COPY_MOVABLE(AttributeChangeStyleInvalidation);

    private:
      Maybe<style::AttributeChangeInvalidation> _attributeInvalidation;
      Maybe<style::ClassChangeInvalidation> _classInvalidation;
      Maybe<style::IdChangeInvalidation> _idInvalidation;
      style::PseudoClassChangeInvalidations _pseudoClassInvalidations;

    public:
      AttributeChangeStyleInvalidation(Element &element, const Attr &attribute, Maybe<DOMStringView> oldValue,
                                       Maybe<DOMStringView> newValue) noexcept
      {
        if (oldValue == newValue || !element.NeedsStyleInvalidation())
        {
          return;
        }

        auto &name = attribute.GetQualifiedName();
        _attributeInvalidation.emplace(element, name, oldValue, newValue);

        if (name.NamespaceURI() == DOMStringAtom::Null())
        {
          if (name.LocalName().View() == u8"class")
          {
            auto oldClasses = element.ClassNames();
            auto newClasses = element.ClassNamesForValue(newValue);
            _classInvalidation.emplace(element, oldClasses, newClasses);
          }
          else if (name.LocalName().View() == u8"id")
          {
            DOMStringAtom oldId(element.IdForStyleResolution());
            auto newIdValue = newValue.value_or(DOMStringView());
            DOMStringAtom newId(element.NodeDocument().InQuirksMode() ? text::ToASCIILower(newIdValue)
                                                                      : DOMString(newIdValue));
            if (oldId != newId)
            {
              _idInvalidation.emplace(element, oldId, newId);
            }
          }
        }

        style::InvalidatePseudoClassesForAttributeChange(_pseudoClassInvalidations, element, name, oldValue,
                                                         newValue);
      }
    };
  }

  bool ElementAlgorithms::IsDefined(const Element &element) noexcept
  {
    return element._customElementState == CustomElementState::Uncustomized
           || element._customElementState == CustomElementState::Custom;
  }

  bool ElementAlgorithms::IsCustom(const Element &element) noexcept
  {
    return element._customElementState == CustomElementState::Custom;
  }

  ExceptionOr<Node *> ElementAlgorithms::InsertAdjacent(Element &element, InsertAdjacentWhere where,
                                                        Node &node) noexcept
  {
    // Run the steps associated with the first ASCII case-insensitive match for where (matched by the caller):
    switch (where)
    {
      // "beforebegin": If element's parent is null, return null. Return the result of pre-inserting node into
      // element's parent before element.
      case InsertAdjacentWhere::BeforeBegin:
      {
        if (element.ParentNode() == nullptr)
        {
          return nullptr;
        }

        auto result = MutationAlgorithms::PreInsert(node, *element.ParentNode(), &element);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      // "afterbegin": Return the result of pre-inserting node into element before element's first child.
      case InsertAdjacentWhere::AfterBegin:
      {
        auto result = MutationAlgorithms::PreInsert(node, element, element.FirstChild());
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      // "beforeend": Return the result of pre-inserting node into element before null.
      case InsertAdjacentWhere::BeforeEnd:
      {
        auto result = MutationAlgorithms::PreInsert(node, element, nullptr);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      // "afterend": If element's parent is null, return null. Return the result of pre-inserting node into
      // element's parent before element's next sibling.
      case InsertAdjacentWhere::AfterEnd:
      {
        auto parent = element.ParentNode();
        if (parent == nullptr)
        {
          return nullptr;
        }

        auto result = MutationAlgorithms::PreInsert(node, *parent, element.NextSibling());
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      // Otherwise: Throw a "SyntaxError" DOMException.
      default: return Exception {ExceptionCode::SyntaxError};
    }
  }

  void ElementAlgorithms::HandleAttributeChanges(Attr &attribute, Element &element,
                                                 Maybe<DOMStringView> oldValue,
                                                 Maybe<DOMStringView> newValue) noexcept
  {
    element.InvalidateLiveCollectionCachesForAttribute(attribute.LocalName());

    // 1. Queue a mutation record of "attributes" for element with attribute's local name, attribute's
    //    namespace, oldValue, « », « », null, and null.
    MutationObserverAlgorithms::QueueMutationRecord(
      MutationRecordType::Attributes, ShareRef(element), attribute.LocalName(), attribute.NamespaceURI(),
      oldValue.has_value() ? DOMStringAtom(*oldValue) : DOMStringAtom::Null(), {}, {}, nullptr, nullptr);

    // 2. If element is custom, then enqueue a custom element callback reaction with element, callback name
    //    "attributeChangedCallback", and « attribute's local name, oldValue, newValue, attribute's namespace
    //    ».
    if (element.IsDefinedCustomElement())
    {
      html::CustomElementReactionQueue::EnqueueAttributeChangedCallbackIfNeeded(
        element, attribute.GetQualifiedName(), oldValue ? DOMStringAtom(*oldValue) : DOMStringAtom::Null(),
        newValue ? DOMStringAtom(*newValue) : DOMStringAtom::Null());
    }

    // 3. Run the attribute change steps with element, attribute's local name, oldValue, newValue, and
    //    attribute's namespace.
    ExtensibilityHooks::ElementAttributeChanged(element, attribute.LocalName(), oldValue, newValue,
                                                attribute.NamespaceURI());

    // Again, as the steps update what filters read, such as an input's type, and could have read them.
    element.InvalidateLiveCollectionCachesForAttribute(attribute.LocalName());
  }

  void ElementAlgorithms::ChangeAttribute(Attr &attribute, DOMString value) noexcept
  {
    attribute._ownerElement->SynchronizeAllAttributes();
    DOMString oldValue;
    {
      AttributeChangeStyleInvalidation styleInvalidation(*attribute._ownerElement, attribute,
                                                         attribute._value, value);
      // 1. Let oldValue be attribute's value.
      // 2. Set attribute's value to value.
      oldValue = std::exchange(attribute._value, krys::move(value));
    }

    // 3. Handle attribute changes for attribute with attribute's element, oldValue, and value.
    HandleAttributeChanges(attribute, *attribute._ownerElement, oldValue, attribute._value);
  }

  void ElementAlgorithms::AppendAttribute(Attr &attribute, Element &element) noexcept
  {
    {
      AttributeChangeStyleInvalidation styleInvalidation(element, attribute, null, attribute._value);
      // 1. Append attribute to element's attribute list.
      element._attributes.push_back(ShareRef(attribute));
    }

    // 2. Set attribute's element to element.
    attribute._ownerElement = CreateWeakPtr(&element);
    // 3. Set attribute's node document to element's node document.
    attribute.SetNodeDocument(element.NodeDocument());

    // 4. Handle attribute changes for attribute with element, null, and attribute's value.
    HandleAttributeChanges(attribute, element, null, attribute._value);
  }

  void ElementAlgorithms::RemoveAttribute(Attr &attribute) noexcept
  {
    // 1. Let element be attribute's element.
    RefPtr<Element> element = attribute._ownerElement.lock();
    krys_debug_assert(element);
    element->SynchronizeAllAttributes();

    Ref<Attr> attributeRef = ShareRef(attribute);
    {
      AttributeChangeStyleInvalidation styleInvalidation(*element, attribute, attribute._value, null);
      // 2. Remove attribute from element's attribute list.
      std::erase_if(element->_attributes, [&](const Ref<Attr> &attr) { return attr.get() == &attribute; });
    }

    // 3. Set attribute's element to null.
    attribute._ownerElement.reset();

    // 4. Handle attribute changes for attribute with element, attribute's value, and null.
    HandleAttributeChanges(attribute, *element, attribute._value, null);
  }

  void ElementAlgorithms::ReplaceAttribute(Attr &oldAttribute, Attr &newAttribute) noexcept
  {
    // 1. Let element be oldAttribute's element.
    RefPtr<Element> element = oldAttribute._ownerElement.lock();
    krys_debug_assert(element);
    element->SynchronizeAllAttributes();

    {
      AttributeChangeStyleInvalidation styleInvalidation(*element, oldAttribute, oldAttribute._value,
                                                         newAttribute._value);
      // 2. Replace oldAttribute by newAttribute in element's attribute list.
      std::replace_if(
        element->_attributes.begin(), element->_attributes.end(),
        [&](const Ref<Attr> &attr) { return attr.get() == &oldAttribute; }, ShareRef(newAttribute));
    }

    // 3. Set newAttribute's element to element.
    newAttribute._ownerElement = CreateWeakPtr(element.get());
    // 4. Set newAttribute's node document to element's node document.
    newAttribute.SetNodeDocument(element->NodeDocument());
    // 5. Set oldAttribute's element to null.
    oldAttribute._ownerElement.reset();

    // 6. Handle attribute changes for oldAttribute with element, oldAttribute's value, and newAttribute's
    //    value.
    HandleAttributeChanges(oldAttribute, *element, oldAttribute._value, newAttribute._value);
  }

  Attr *ElementAlgorithms::GetAttributeByName(DOMStringAtom qualifiedName, const Element &element) noexcept
  {
    // 1. If element is in the HTML namespace and its node document is an HTML document, then set
    //    qualifiedName to qualifiedName in ASCII lowercase.
    if (element.NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(element.NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    // 2. Return the first attribute in element's attribute list whose qualified name is qualifiedName;
    //    otherwise null.
    element.SynchronizeAllAttributes();
    auto it = std::find_if(element._attributes.begin(), element._attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
    return it != element._attributes.end() ? it->get() : nullptr;
  }

  Attr *ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom namespaceURI, DOMStringAtom localName,
                                                   const Element &element) noexcept
  {
    // 1. If namespace is the empty string, then set it to null.
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    // 2. Return the attribute in element's attribute list whose namespace is namespace and local name is
    //    localName, if any; otherwise null.
    element.SynchronizeAllAttributes();
    auto it =
      std::find_if(element._attributes.begin(), element._attributes.end(), [&](const Ref<Attr> &attr)
                   { return attr->LocalName() == localName && attr->NamespaceURI() == namespaceURI; });
    return it != element._attributes.end() ? it->get() : nullptr;
  }

  DOMString ElementAlgorithms::GetAttributeValue(const Element &element, DOMStringAtom localName,
                                                 DOMStringAtom namespaceURI) noexcept
  {
    // 1. Let attr be the result of getting an attribute given namespace, localName, and element.
    Attr *attr = GetAttributeByNamespace(namespaceURI, localName, element);
    // 2. If attr is null, then return the empty string.
    if (attr == nullptr)
    {
      return u8"";
    }

    // 3. Return attr's value.
    return attr->Value();
  }

  ExceptionOr<RefPtr<Attr>> ElementAlgorithms::SetAttribute(Attr &attr, Element &element) noexcept
  {
    // 1. (Skipped: Trusted Types aren't supported, so attr's value is used as it is.)

    // 2. If attr's element is neither null nor element, throw an "InUseAttributeError" DOMException.
    if (attr._ownerElement != nullptr && attr._ownerElement.get() != &element)
    {
      return Exception {ExceptionCode::InUseAttributeError};
    }

    // 3. Let oldAttr be the result of getting an attribute given attr's namespace, attr's local name, and
    //    element.
    auto oldAttr = ShareRefPtr(GetAttributeByNamespace(attr.NamespaceURI(), attr.LocalName(), element));
    // 4. If oldAttr is attr, return attr.
    if (&attr == oldAttr)
    {
      return oldAttr;
    }

    // 5. (Skipped: attr's value is already its verified value; see step 1.)

    // 6. If oldAttr is non-null, then replace oldAttr with attr.
    if (oldAttr != nullptr)
    {
      ReplaceAttribute(*oldAttr, attr);
    }
    // 7. Otherwise, append attr to element.
    else
    {
      AppendAttribute(attr, element);
    }

    // 8. Return oldAttr.
    return oldAttr;
  }

  void ElementAlgorithms::SetAttributeValue(Element &element, DOMStringAtom localName, DOMString value,
                                            DOMStringAtom prefix, DOMStringAtom namespaceURI) noexcept
  {
    // 1. Let attribute be the result of getting an attribute given namespace, localName, and element.
    Attr *attribute = GetAttributeByNamespace(namespaceURI, localName, element);
    // 2. If attribute is null, then append the result of creating an attribute given element's node
    //    document, localName, namespace, prefix, and value to element, and then return.
    if (attribute == nullptr)
    {
      auto attr = AdoptRef<Attr>(*new Attr(
        element.NodeDocument(), QualifiedName {namespaceURI, prefix, localName}, krys::move(value)));
      AppendAttribute(*attr, element);
    }
    // 3. Change attribute to value.
    else
    {
      ChangeAttribute(*attribute, krys::move(value));
    }
  }

  RefPtr<Attr> ElementAlgorithms::RemoveAttributeByName(DOMStringAtom qualifiedName,
                                                        Element &element) noexcept
  {
    // 1. Let attr be the result of getting an attribute given qualifiedName and element.
    auto attr = ShareRefPtr(GetAttributeByName(qualifiedName, element));
    // 2. If attr is non-null, then remove attr.
    if (attr != nullptr)
    {
      RemoveAttribute(*attr);
    }

    // 3. Return attr.
    return attr;
  }

  RefPtr<Attr> ElementAlgorithms::RemoveAttributeByNamespace(DOMStringAtom namespaceURI,
                                                             DOMStringAtom localName,
                                                             Element &element) noexcept
  {
    // 1. Let attr be the result of getting an attribute given namespace, localName, and element.
    auto attr = ShareRefPtr(GetAttributeByNamespace(namespaceURI, localName, element));
    // 2. If attr is non-null, then remove attr.
    if (attr != nullptr)
    {
      RemoveAttribute(*attr);
    }

    // 3. Return attr.
    return attr;
  }

  ExceptionOr<void> ElementAlgorithms::AttachShadowRoot(Element &element, ShadowRootMode mode,
                                                        Clonable clonable, Serialisable serializable,
                                                        DelegatesFocus delegatesFocus,
                                                        SlotAssignmentMode slotAssignment,
                                                        Maybe<DOMString> referenceTarget,
                                                        html::CustomElementRegistry *registry) noexcept
  {
    // 1. If element's namespace is not the HTML namespace, then throw a "NotSupportedError" DOMException.
    if (element.NamespaceURI() != infra::Namespaces::HTML)
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // 2. If element's local name is not a valid shadow host name, then throw a "NotSupportedError"
    //    DOMException.
    if (!NameValidation::IsValidShadowHostName(element.LocalName().View()))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // 3. If element's local name is a valid custom element name, or element's is value is non-null:
    //    (Customized built-in elements aren't supported, so only the local name is checked.)
    if (NameValidation::IsValidCustomElementName(element.LocalName().View()))
    {
      // 3a. Let definition be the result of looking up a custom element definition given element's custom
      //     element registry, its namespace, its local name, and its is value.
      auto *definition = html::CustomElementAlgorithms::LookupCustomElementDefinition(
        element.CustomElementRegistry().get(), element.NamespaceURI(), element.LocalName());
      // 3b. If definition is non-null and definition's disable shadow is true, then throw a
      //     "NotSupportedError" DOMException.
      if (definition != nullptr && definition->IsShadowDisabled())
      {
        return Exception {ExceptionCode::NotSupportedError};
      }
    }

    // 4. If element is a shadow host:
    if (TreeQueries::IsShadowHost(element))
    {
      // 4a. Let currentShadowRoot be element's shadow root.
      auto &currentShadowRoot = element._shadowRoot;
      // 4b. If any of the following are true: currentShadowRoot's declarative is false; or
      // currentShadowRoot's
      //     mode is not mode, then throw a "NotSupportedError" DOMException.
      if (!currentShadowRoot->_declarative || currentShadowRoot->Mode() != mode)
      {
        return Exception {ExceptionCode::NotSupportedError};
      }

      // 4c. Remove all of currentShadowRoot's children, in tree order.
      while (currentShadowRoot->FirstChild())
      {
        auto result = currentShadowRoot->RemoveChild(*currentShadowRoot->FirstChild());
        if (result.HasException())
        {
          return result.ReleaseException();
        }
      }

      // 4d. Set currentShadowRoot's declarative to false.
      currentShadowRoot->_declarative = false;
      // 4e. Return.
      return {};
    }

    // 5. Let shadow be the result of creating a node that implements ShadowRoot, given element's node
    //    document.
    // 15. Set shadow's custom element registry to registry.
    auto shadow = CreateRef<ShadowRoot>(element.NodeDocument(), ShareRefPtr(registry));
    // 6. Set shadow's host to element.
    shadow->_host = CreateWeakPtr(&element);
    // 7. Set shadow's mode to mode.
    shadow->_mode = mode;
    // 8. Set shadow's delegates focus to delegatesFocus.
    shadow->_delegatesFocus = delegatesFocus;

    // 9. If element's custom element state is "precustomized" or "custom", then set shadow's available to
    //    element internals to true.
    if (element._customElementState == CustomElementState::Precustomized
        || element._customElementState == CustomElementState::Custom)
    {
      shadow->_availableToElementInternals = true;
    }

    // 10. Set shadow's declarative to false.
    shadow->_declarative = false;
    // 11. Set shadow's serializable to serializable.
    shadow->_serializable = serializable;
    // 12. Set shadow's slot assignment to slotAssignment.
    shadow->_slotAssignment = slotAssignment;
    // 13. Set shadow's clonable to clonable.
    shadow->_clonable = clonable;
    // 14. Set shadow's reference target to referenceTarget.
    shadow->_referenceTarget = krys::move(referenceTarget);

    // 16. Set element's shadow root to shadow.
    AddShadowRoot(element, shadow);

    return {};
  }

  ShadowRoot &ElementAlgorithms::AttachUserAgentShadowRoot(Element &element) noexcept
  {
    krys_debug_assert(!element._shadowRoot);

    auto shadow = CreateRef<ShadowRoot>(element.NodeDocument());
    shadow->_host = CreateWeakPtr(&element);
    shadow->_mode = ShadowRootMode::UserAgent;
    shadow->_slotAssignment = SlotAssignmentMode::Named;

    AddShadowRoot(element, shadow);

    return *shadow;
  }

  void ElementAlgorithms::AddShadowRoot(Element &element, const Ref<ShadowRoot> &shadow) noexcept
  {
    RenderTreeUpdater::TearDownRenderersForShadowRootInsertion(element);

    element._shadowRoot = shadow;

    if (element.IsConnected())
    {
      ExtensibilityHooks::NodeInserted(*shadow, false);
    }

    if (shadow->Mode() == ShadowRootMode::UserAgent)
    {
      element.DidAddUserAgentShadowRoot(*shadow);
    }

    element.InvalidateStyleAndRenderersForSubtree();
  }

  UniquePtr<DOMTokenList>
    ElementAlgorithms::CreateDOMTokenList(Element &element, DOMStringAtom attributeName,
                                          IsSupportedTokenFunction &&isSupportedToken) noexcept
  {
    return UniquePtr<DOMTokenList>(new DOMTokenList(element, attributeName, krys::move(isSupportedToken)));
  }
}