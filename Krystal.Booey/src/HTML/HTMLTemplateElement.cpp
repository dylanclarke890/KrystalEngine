#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  // FIXME: Implement the HTMLTemplateElement constructor in a spec compliant way.

  HTMLTemplateElement::HTMLTemplateElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Template),
        _content(CreateRef<dom::DocumentFragment>(document))
  {
    // The template contents' host is the template element.
    _content->SetHost(*this);
  }

#pragma region HTMLTemplateElement

  RefPtr<dom::DocumentFragment> HTMLTemplateElement::Content() const noexcept
  {
    assert(!Is<dom::ShadowRoot>(_content.get()));
    return _content;
  }

  void HTMLTemplateElement::SetTemplateContents(dom::ShadowRoot &shadow) noexcept
  {
    _content = ShareRef<dom::DocumentFragment>(shadow);
  }

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
    return Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootmode").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootMode(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootmode", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootDelegatesFocus() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"shadowrootdelegatesfocus");
  }

  void HTMLTemplateElement::ShadowRootDelegatesFocus(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"shadowrootdelegatesfocus", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootSlotAssignment() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootSlotAssignment(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootClonable() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"shadowrootclonable");
  }

  void HTMLTemplateElement::ShadowRootClonable(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"shadowrootclonable", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootSerialisable() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"shadowrootserializable");
  }

  void HTMLTemplateElement::ShadowRootSerialisable(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"shadowrootserializable", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootCustomElementRegistry() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootCustomElementRegistry(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry", krys::move(value));
  }

#pragma endregion
}