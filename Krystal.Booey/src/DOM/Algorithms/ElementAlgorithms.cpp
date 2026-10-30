#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Booey/Style/AttributeChangeInvalidation.hpp"
#include "Krystal.Booey/Style/ClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/IdChangeInvalidation.hpp"
#include "Krystal.Booey/Style/PseudoClassStateInvalidation.hpp"
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
          if (name.LocalName().View() == Attributes::Class)
          {
            auto oldClasses = element.ClassNames();
            auto newClasses = element.ClassNamesForValue(newValue);
            _classInvalidation.emplace(element, oldClasses, newClasses);
          }
          else if (name.LocalName().View() == Attributes::Id)
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
    switch (where)
    {
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
      case InsertAdjacentWhere::AfterBegin:
      {
        auto result = MutationAlgorithms::PreInsert(node, element, element.FirstChild());
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
      case InsertAdjacentWhere::BeforeEnd:
      {
        auto result = MutationAlgorithms::PreInsert(node, element, nullptr);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        return &result.Value();
      }
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
      default: return Exception {ExceptionCode::SyntaxError};
    }
  }

  void ElementAlgorithms::HandleAttributeChanges(Attr &attribute, Element &element,
                                                 Maybe<DOMStringView> oldValue,
                                                 Maybe<DOMStringView> newValue) noexcept
  {
    MutationObserverAlgorithms::QueueMutationRecord(
      MutationRecordType::Attributes, ShareRef(element), attribute.LocalName(), attribute.NamespaceURI(),
      oldValue.has_value() ? DOMStringAtom(*oldValue) : DOMStringAtom::Null(), {}, {}, nullptr, nullptr);

    if (element.IsDefinedCustomElement())
    {
      html::CustomElementReactionQueue::EnqueueAttributeChangedCallbackIfNeeded(
        element, attribute.GetQualifiedName(), oldValue ? DOMStringAtom(*oldValue) : DOMStringAtom::Null(),
        newValue ? DOMStringAtom(*newValue) : DOMStringAtom::Null());
    }

    ExtensibilityHooks::ElementAttributeChanged(element, attribute.LocalName(), oldValue, newValue,
                                                attribute.NamespaceURI());
  }

  void ElementAlgorithms::ChangeAttribute(Attr &attribute, DOMString &&value) noexcept
  {
    attribute._ownerElement->SynchronizeAllAttributes();
    DOMString oldValue;
    {
      AttributeChangeStyleInvalidation styleInvalidation(*attribute._ownerElement, attribute,
                                                         attribute._value, value);
      oldValue = std::exchange(attribute._value, krys::move(value));
    }

    HandleAttributeChanges(attribute, *attribute._ownerElement, oldValue, attribute._value);
  }

  void ElementAlgorithms::AppendAttribute(Attr &attribute, Element &element) noexcept
  {
    {
      AttributeChangeStyleInvalidation styleInvalidation(element, attribute, null, attribute._value);
      element._attributes.push_back(ShareRef(attribute));
    }

    attribute._ownerElement = CreateWeakPtr(&element);
    attribute.SetNodeDocument(element.NodeDocument());

    HandleAttributeChanges(attribute, element, null, attribute._value);
  }

  void ElementAlgorithms::RemoveAttribute(Attr &attribute) noexcept
  {
    assert(attribute._ownerElement);
    RefPtr<Element> element = attribute._ownerElement.lock();
    assert(element);
    element->SynchronizeAllAttributes();

    Ref<Attr> attributeRef = ShareRef(attribute);
    {
      AttributeChangeStyleInvalidation styleInvalidation(*element, attribute, attribute._value, null);
      element->_attributes.erase(std::remove_if(element->_attributes.begin(), element->_attributes.end(),
                                                [&](const Ref<Attr> &attr)
                                                { return attr.get() == &attribute; }));
    }

    attribute._ownerElement.reset();

    HandleAttributeChanges(attribute, *element, attribute._value, null);
  }

  void ElementAlgorithms::ReplaceAttribute(Attr &oldAttribute, Attr &newAttribute) noexcept
  {
    assert(oldAttribute._ownerElement);
    RefPtr<Element> element = oldAttribute._ownerElement.lock();
    assert(element);
    element->SynchronizeAllAttributes();

    {
      AttributeChangeStyleInvalidation styleInvalidation(*element, oldAttribute, oldAttribute._value,
                                                         newAttribute._value);
      std::replace_if(
        element->_attributes.begin(), element->_attributes.end(),
        [&](const Ref<Attr> &attr) { return attr.get() == &oldAttribute; }, ShareRef(newAttribute));
    }

    newAttribute._ownerElement = CreateWeakPtr(element.get());
    newAttribute.SetNodeDocument(element->NodeDocument());
    oldAttribute._ownerElement.reset();

    HandleAttributeChanges(oldAttribute, *element, oldAttribute._value, newAttribute._value);
  }

  Attr *ElementAlgorithms::GetAttributeByName(DOMStringAtom qualifiedName, const Element &element) noexcept
  {
    if (element.NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(element.NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    element.SynchronizeAllAttributes();
    auto it = std::find_if(element._attributes.begin(), element._attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
    return it != element._attributes.end() ? it->get() : nullptr;
  }

  Attr *ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom namespaceURI, DOMStringAtom localName,
                                                   const Element &element) noexcept
  {
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    element.SynchronizeAllAttributes();
    auto it =
      std::find_if(element._attributes.begin(), element._attributes.end(), [&](const Ref<Attr> &attr)
                   { return attr->LocalName() == localName && attr->NamespaceURI() == namespaceURI; });
    return it != element._attributes.end() ? it->get() : nullptr;
  }

  DOMString ElementAlgorithms::GetAttributeValue(const Element &element, DOMStringAtom localName,
                                                 DOMStringAtom namespaceURI) noexcept
  {
    Attr *attr = GetAttributeByNamespace(namespaceURI, localName, element);
    if (attr == nullptr)
    {
      return u8"";
    }

    return attr->Value();
  }

  ExceptionOr<RefPtr<Attr>> ElementAlgorithms::SetAttribute(Attr &attr, Element &element) noexcept
  {
    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with attr’s local name, attr’s namespace, element, and attr’s value.

    if (attr._ownerElement != nullptr && attr._ownerElement.get() != &element)
    {
      return Exception {ExceptionCode::InUseAttributeError};
    }

    auto oldAttr = ShareRefPtr(GetAttributeByNamespace(attr.NamespaceURI(), attr.LocalName(), element));
    if (&attr == oldAttr)
    {
      return oldAttr;
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Set attr’s value to verifiedValue.

    if (oldAttr != nullptr)
    {
      ReplaceAttribute(*oldAttr, attr);
    }
    else
    {
      AppendAttribute(attr, element);
    }

    return oldAttr;
  }

  void ElementAlgorithms::SetAttributeValue(Element &element, DOMStringAtom localName, DOMString &&value,
                                            DOMStringAtom prefix, DOMStringAtom namespaceURI) noexcept
  {
    Attr *attribute = GetAttributeByNamespace(namespaceURI, localName, element);
    if (attribute == nullptr)
    {
      auto attr = AdoptRef<Attr>(*new Attr(
        element.NodeDocument(), QualifiedName {namespaceURI, prefix, localName}, krys::move(value)));
      AppendAttribute(*attr, element);
    }
    else
    {
      ChangeAttribute(*attribute, krys::move(value));
    }
  }

  RefPtr<Attr> ElementAlgorithms::RemoveAttributeByName(DOMStringAtom qualifiedName,
                                                        Element &element) noexcept
  {
    auto attr = ShareRefPtr(GetAttributeByName(qualifiedName, element));
    if (attr != nullptr)
    {
      RemoveAttribute(*attr);
    }

    return attr;
  }

  RefPtr<Attr> ElementAlgorithms::RemoveAttributeByNamespace(DOMStringAtom namespaceURI,
                                                             DOMStringAtom localName,
                                                             Element &element) noexcept
  {
    auto attr = ShareRefPtr(GetAttributeByNamespace(namespaceURI, localName, element));
    if (attr != nullptr)
    {
      RemoveAttribute(*attr);
    }

    return attr;
  }

  ExceptionOr<void> ElementAlgorithms::AttachShadowRoot(Element &element, ShadowRootMode mode,
                                                        Clonable clonable, Serialisable serializable,
                                                        DelegatesFocus delegatesFocus,
                                                        SlotAssignmentMode slotAssignment,
                                                        html::CustomElementRegistry *registry) noexcept
  {
    if (element.NamespaceURI() != infra::Namespaces::HTML)
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (!NameValidation::IsValidShadowHostName(element.LocalName().View()))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // NOTE: customized built-in elements aren't supported, so an element's is value doesn't matter.
    if (NameValidation::IsValidCustomElementName(element.LocalName().View()))
    {
      auto *definition = html::CustomElementAlgorithms::LookupCustomElementDefinition(
        element.CustomElementRegistry().get(), element.NamespaceURI(), element.LocalName());
      if (definition != nullptr && definition->IsShadowDisabled())
      {
        return Exception {ExceptionCode::NotSupportedError};
      }
    }

    if (TreeQueries::IsShadowHost(element))
    {
      auto &currentShadowRoot = element._shadowRoot;
      if (!currentShadowRoot->_declarative || currentShadowRoot->Mode() != mode)
      {
        return Exception {ExceptionCode::NotSupportedError};
      }

      while (currentShadowRoot->FirstChild())
      {
        auto result = currentShadowRoot->RemoveChild(*currentShadowRoot->FirstChild());
        if (result.HasException())
        {
          return result.ReleaseException();
        }
      }

      currentShadowRoot->_declarative = false;
      return {};
    }

    auto shadow = CreateRef<ShadowRoot>(element.NodeDocument(), ShareRefPtr(registry));
    shadow->_host = CreateWeakPtr(&element);
    shadow->_mode = mode;
    shadow->_delegatesFocus = delegatesFocus;

    if (element._customElementState == CustomElementState::Precustomized
        || element._customElementState == CustomElementState::Custom)
    {
      shadow->_availableToElementInternals = true;
    }

    shadow->_slotAssignment = slotAssignment;
    shadow->_declarative = false;
    shadow->_clonable = clonable;
    shadow->_serializable = serializable;

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