#include "Krystal.Web/Html/Elements/HTMLBaseElement.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"

namespace krys::web
{
  HTMLBaseElement::HTMLBaseElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Base)
  {
  }

#pragma region HTMLBaseElement

  dom::USVString HTMLBaseElement::Href() const noexcept
  {
    // 1. Let document be element's node document.
    auto &document = NodeDocument();

    // 2. Let url be the value of the href attribute of this element, if it has one, and the empty string
    //    otherwise.
    auto url = GetAttribute(u8"href").value_or(DOMString());

    // 3. Let urlRecord be the result of parsing url with document's fallback base URL, and document's
    //    character encoding. (Thus, the base element isn't affected by other base elements or itself.)
    auto urlRecord = document.CompleteUrl(url, document.FallbackBaseUrl());

    // 4. If urlRecord is failure, return url.
    if (!urlRecord.IsValid())
    {
      return dom::USVString(krys::move(url));
    }

    // 5. Return the serialization of urlRecord.
    return dom::USVString(urlRecord.String());
  }

  void HTMLBaseElement::Href(dom::USVString value) noexcept
  {
    return html::ReflectSetter<dom::USVString>(*this, u8"href", krys::move(value));
  }

  DOMString HTMLBaseElement::Target() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLBaseElement::Target(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"target", krys::move(value));
  }

#pragma endregion

  void HTMLBaseElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();

    if (IsConnected())
    {
      NodeDocument().ProcessBaseElement();
    }
  }

  void HTMLBaseElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);

    if (oldAncestor.IsConnected())
    {
      NodeDocument().ProcessBaseElement();
    }
  }

  void HTMLBaseElement::OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    if (IsConnected())
    {
      NodeDocument().ProcessBaseElement();
    }
  }

  void HTMLBaseElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                           Maybe<DOMStringView> value,
                                           const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    if ((localName == u8"href" || localName == u8"target") && namespaceURI == null && IsConnected())
    {
      NodeDocument().ProcessBaseElement();
    }
  }
}
