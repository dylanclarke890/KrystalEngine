#include "Krystal.Web/Html/Elements/HTMLObjectElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/Internals/PresentationalHints.hpp"

namespace krys::web
{
  HTMLObjectElement::HTMLObjectElement(Document &document) noexcept
      : HTMLPlugInElement(document, html::HTMLElementInterface::Object)
  {
  }

  HTMLObjectElement::~HTMLObjectElement() noexcept
  {
    ClearForm();
  }

  bool HTMLObjectElement::AppendFormData(html::DOMFormData &) noexcept
  {
    return false;
  }

  void HTMLObjectElement::OnInsert() noexcept
  {
    HTMLPlugInElement::OnInsert();
    html::FormListedElement::ElementInsertedIntoAncestor(*this, IsConnected());
    if (!IsConnected())
    {
      ResetFormOwner();
    }
  }

  void HTMLObjectElement::OnPostConnection() noexcept
  {
    HTMLPlugInElement::OnPostConnection();
    ResetFormOwner();
  }

  void HTMLObjectElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLPlugInElement::OnRemove(isSubtreeRoot, oldAncestor);
    html::FormListedElement::ElementRemovedFromAncestor(*this, oldAncestor.IsConnected());
  }

  void HTMLObjectElement::OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLPlugInElement::OnMove(isSubtreeRoot, oldAncestor);
    html::FormListedElement::ElementRemovedFromAncestor(*this, false);
    html::FormListedElement::ElementInsertedIntoAncestor(*this, IsConnected());
    ResetFormOwner();
  }

  void HTMLObjectElement::OnAdopt(Document &oldDocument) noexcept
  {
    HTMLPlugInElement::OnAdopt(oldDocument);
    html::FormListedElement::DidMoveToNewDocument();
  }

  void HTMLObjectElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                             Maybe<DOMStringView> value,
                                             const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLPlugInElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == null)
    {
      html::FormListedElement::ParseAttribute(localName, value);
    }
  }

#pragma region HTMLObjectElement

  USVString HTMLObjectElement::Data() const noexcept
  {
    return html::ReflectUrl<USVString>(*this, u8"data");
  }

  void HTMLObjectElement::Data(USVString value) noexcept
  {
    return html::ReflectUrl<USVString>(*this, u8"data", krys::move(value));
  }

  DOMString HTMLObjectElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLObjectElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

  DOMString HTMLObjectElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLObjectElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  DOMString HTMLObjectElement::Width() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLObjectElement::Width(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"width", krys::move(value));
  }

  DOMString HTMLObjectElement::Height() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLObjectElement::Height(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"height", krys::move(value));
  }

#pragma endregion

#pragma region HTMLObjectElement Obsolete members

  DOMString HTMLObjectElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLObjectElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

  DOMString HTMLObjectElement::Archive() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"archive").value_or(u8"");
  }

  void HTMLObjectElement::Archive(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"archive", krys::move(value));
  }

  DOMString HTMLObjectElement::Code() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"code").value_or(u8"");
  }

  void HTMLObjectElement::Code(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"code", krys::move(value));
  }

  bool HTMLObjectElement::Declare() const noexcept
  {
    return html::Reflect<bool>(*this, u8"declare");
  }

  void HTMLObjectElement::Declare(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"declare", krys::move(value));
  }

  uint32 HTMLObjectElement::HSpace() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLObjectElement::HSpace(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"hspace", krys::move(value));
  }

  DOMString HTMLObjectElement::Standby() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"standby").value_or(u8"");
  }

  void HTMLObjectElement::Standby(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"standby", krys::move(value));
  }

  uint32 HTMLObjectElement::VSpace() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLObjectElement::VSpace(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"vspace", krys::move(value));
  }

  DOMString HTMLObjectElement::CodeBase() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"codebase").value_or(u8"");
  }

  void HTMLObjectElement::CodeBase(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"codebase", krys::move(value));
  }

  DOMString HTMLObjectElement::CodeType() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"codetype").value_or(u8"");
  }

  void HTMLObjectElement::CodeType(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"codetype", krys::move(value));
  }

  DOMString HTMLObjectElement::UseMap() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLObjectElement::UseMap(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"usemap", krys::move(value));
  }

  DOMString HTMLObjectElement::Border() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLObjectElement::Border(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"border", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLObjectElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                             const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"border"))
    {
      return true;
    }
    return HTMLPlugInElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLObjectElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                                 const DOMStringAtom &namespaceURI,
                                                                 DOMStringView value,
                                                                 css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
    {
      HTMLPlugInElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"border")
    {
      ApplyBorderAttributeToStyle(value, style);
      return;
    }
    HTMLPlugInElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
