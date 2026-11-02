#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#attr-template-shadowrootmode
  template <>
  struct EnumeratedAttributeTraits<"shadowrootmode", HTMLTemplateElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {
      KeywordStateMapping {u8"open", AttributeState::Open},
      KeywordStateMapping {u8"closed", AttributeState::Closed},
    };

    constexpr static AttributeState MissingValueDefault = AttributeState::None;
    constexpr static AttributeState InvalidValueDefault = AttributeState::None;
  };

  /// @see https://html.spec.whatwg.org/#attr-template-shadowrootslotassignment
  template <>
  struct EnumeratedAttributeTraits<"shadowrootslotassignment", HTMLTemplateElement>
  {
    static inline KeywordStateMappingArray<2> StateMappings = {
      KeywordStateMapping {u8"named", AttributeState::Named},
      KeywordStateMapping {u8"manual", AttributeState::Manual},
    };

    constexpr static AttributeState MissingValueDefault = AttributeState::Named;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Named;
  };

  // FIXME: Implement the HTMLTemplateElement constructor in a spec compliant way.

  // https://html.spec.whatwg.org/#template-contents
  HTMLTemplateElement::HTMLTemplateElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Template),
        // 1. Let document be the template element's node document's appropriate template contents owner
        //    document.
        // (Skipped: there's no inert template contents owner document.)
        // 2. Create a DocumentFragment object whose node document is document and host is the template
        // element.
        // 3. Set the template element's template contents to the newly created DocumentFragment object.
        _content(CreateRef<dom::DocumentFragment>(document))
  {
    _content->SetHost(*this);
  }

#pragma region HTMLTemplateElement

  // https://html.spec.whatwg.org/#dom-template-content
  RefPtr<dom::DocumentFragment> HTMLTemplateElement::Content() const noexcept
  {
    // 1. Assert: this's template contents is not a ShadowRoot node.
    krys_assert(!Is<dom::ShadowRoot>(_content.get()));

    // 2. Return this's template contents.
    return _content;
  }

  dom::DOMString HTMLTemplateElement::HtmlFor() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLTemplateElement::HtmlFor(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"for", krys::move(value));
  }

  void HTMLTemplateElement::SetTemplateContents(dom::ShadowRoot &shadow) noexcept
  {
    _content = ShareRef<dom::DocumentFragment>(shadow);
  }

  // https://html.spec.whatwg.org/#the-template-element:concept-node-adopt-ext
  void HTMLTemplateElement::OnAdopt(dom::Document &oldDocument) noexcept
  {
    HTMLElement::OnAdopt(oldDocument);

    // 1. Let document be node's node document's appropriate template contents owner document.
    // (The template contents are in the template's own document.)
    // 2. Adopt node's template contents (a DocumentFragment object) into document.
    // (Not a declarative shadow root, which is a host's.)
    if (!Is<dom::ShadowRoot>(*_content))
    {
      (void)dom::DocumentAlgorithms::AdoptNode(*_content, NodeDocument());
    }
  }

  // https://html.spec.whatwg.org/#the-template-element:concept-node-clone-ext
  void HTMLTemplateElement::OnClone(dom::Node &copy, bool subtree) noexcept
  {
    HTMLElement::OnClone(copy, subtree);

    // 1. If subtree is false, then return.
    if (!subtree)
    {
      return;
    }

    // 2. For each child of node's template contents's children, in tree order: clone a node given child with
    //    document set to copy's template contents's node document, subtree set to true, and parent set to
    //    copy's template contents.
    auto &copyContents = Downcast<HTMLTemplateElement>(copy).TemplateContents();
    for (auto &child : dom::ChildNodeRange(TemplateContents()))
    {
      (void)dom::NodeAlgorithms::CloneNode(child, &copyContents.NodeDocument(), true, &copyContents);
    }
  }

  dom::DOMString HTMLTemplateElement::ShadowRootMode() const noexcept
  {
    return EnumeratedAttribute<"shadowrootmode", HTMLTemplateElement>::ResolveCanonicalKeyword<
      dom::DOMString>(Reflect<dom::DOMString>(*this, u8"shadowrootmode"));
  }

  void HTMLTemplateElement::ShadowRootMode(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"shadowrootmode", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootDelegatesFocus() const noexcept
  {
    return Reflect<bool>(*this, u8"shadowrootdelegatesfocus");
  }

  void HTMLTemplateElement::ShadowRootDelegatesFocus(bool value) noexcept
  {
    Reflect<bool>(*this, u8"shadowrootdelegatesfocus", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootSlotAssignment() const noexcept
  {
    return EnumeratedAttribute<"shadowrootslotassignment", HTMLTemplateElement>::ResolveCanonicalKeyword<
      dom::DOMString>(Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment"));
  }

  void HTMLTemplateElement::ShadowRootSlotAssignment(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootClonable() const noexcept
  {
    return Reflect<bool>(*this, u8"shadowrootclonable");
  }

  void HTMLTemplateElement::ShadowRootClonable(bool value) noexcept
  {
    Reflect<bool>(*this, u8"shadowrootclonable", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootSerialisable() const noexcept
  {
    return Reflect<bool>(*this, u8"shadowrootserializable");
  }

  void HTMLTemplateElement::ShadowRootSerialisable(bool value) noexcept
  {
    Reflect<bool>(*this, u8"shadowrootserializable", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootCustomElementRegistry() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootCustomElementRegistry(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry", krys::move(value));
  }

#pragma endregion
}