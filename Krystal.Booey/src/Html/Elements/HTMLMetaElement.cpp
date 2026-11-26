#include "Krystal.Booey/Html/Elements/HTMLMetaElement.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"

namespace krys::boo
{
  KRYS_NODISCARD static bool IsNameColorScheme(DOMStringView nameValue) noexcept
  {
    return text::IsASCIICaselessEqual(nameValue, u8"color-scheme");
  }

#pragma region HTMLMetaElement

  HTMLMetaElement::HTMLMetaElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Meta)
  {
  }

  DOMString HTMLMetaElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMetaElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  DOMString HTMLMetaElement::HttpEquiv() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"http-equiv").value_or(u8"");
  }

  void HTMLMetaElement::HttpEquiv(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"http-equiv", krys::move(value));
  }

  DOMString HTMLMetaElement::Content() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"content").value_or(u8"");
  }

  void HTMLMetaElement::Content(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"content", krys::move(value));
  }

  DOMString HTMLMetaElement::Media() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLMetaElement::Media(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"media", krys::move(value));
  }

#pragma endregion

#pragma region HTMLMetaElement Obsolete members

  DOMString HTMLMetaElement::Scheme() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"scheme").value_or(u8"");
  }

  void HTMLMetaElement::Scheme(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"scheme", krys::move(value));
  }

#pragma endregion

  void HTMLMetaElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    Process();
  }

  void HTMLMetaElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                           Maybe<DOMStringView> value,
                                           const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }
    if (localName == u8"name")
    {
      Process(oldValue);
    }
    else if (localName == u8"http-equiv" || localName == u8"content")
    {
      Process();
    }
  }

  void HTMLMetaElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    auto name = GetAttribute(u8"name");
    if (oldAncestor.IsConnected() && name && IsNameColorScheme(*name))
    {
      oldAncestor.NodeDocument().MetaElementColorSchemeChanged();
    }
  }

  void HTMLMetaElement::Process(Maybe<DOMStringView> oldName) noexcept
  {
    // Only a meta element in the document tree affects the document.
    if (IsInDocumentTree())
    {
      auto name = GetAttribute(u8"name");
      if ((name && IsNameColorScheme(*name)) || (oldName && IsNameColorScheme(*oldName)))
      {
        NodeDocument().MetaElementColorSchemeChanged();
      }
    }

    // When a meta element is inserted into the document, if its http-equiv attribute is present and
    // represents one of the pragma directives, run its algorithm.
    if (!IsConnected())
    {
      return;
    }
    auto httpEquiv = GetAttribute(u8"http-equiv");
    auto content = GetAttribute(u8"content");
    if (!httpEquiv || !content)
    {
      return;
    }
    NodeDocument().ProcessMetaHttpEquiv(*httpEquiv, *content);
  }
}
