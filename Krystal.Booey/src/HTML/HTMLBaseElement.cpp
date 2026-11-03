#include "Krystal.Booey/HTML/HTMLBaseElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLBaseElement::HTMLBaseElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Base)
  {
  }

#pragma region HTMLBaseElement

  dom::USVString HTMLBaseElement::Href() const noexcept
  {
    // 1. Let document be element's node document.
    auto &document = NodeDocument();

    // 2. Let url be the value of the href attribute of this element, if it has one, and the empty string
    //    otherwise.
    auto url = GetAttribute(u8"href").value_or(dom::DOMString());

    // 3. Let urlRecord be the result of parsing url with document's fallback base URL, and document's
    //    character encoding. (Thus, the base element isn't affected by other base elements or itself.)
    auto urlRecord = document.CompleteURL(url, document.FallbackBaseURL());

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
    return ReflectSetter<dom::USVString>(*this, u8"href", krys::move(value));
  }

  dom::DOMString HTMLBaseElement::Target() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLBaseElement::Target(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
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

  void HTMLBaseElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);

    if (oldAncestor.IsConnected())
    {
      NodeDocument().ProcessBaseElement();
    }
  }

  void HTMLBaseElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    if (IsConnected())
    {
      NodeDocument().ProcessBaseElement();
    }
  }

  void HTMLBaseElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    if ((localName == u8"href" || localName == u8"target") && namespaceURI == dom::DOMStringAtom::Null()
        && IsConnected())
    {
      NodeDocument().ProcessBaseElement();
    }
  }
}
