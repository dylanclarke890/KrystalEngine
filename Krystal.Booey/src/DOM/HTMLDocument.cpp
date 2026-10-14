#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  bool HTMLDocument::IsCaseSensitiveAttribute(const QualifiedName &attributeName) noexcept
  {
    // https://html.spec.whatwg.org/multipage/semantics-other.html#case-sensitivity-of-selectors
    static constexpr DOMStringView names[] {
      u8"accept-charset", u8"accept",    u8"align",     u8"alink",      u8"axis",     u8"bgcolor",
      u8"charset",        u8"checked",   u8"clear",     u8"codetype",   u8"color",    u8"compact",
      u8"declare",        u8"defer",     u8"dir",       u8"direction",  u8"disabled", u8"enctype",
      u8"face",           u8"frame",     u8"hreflang",  u8"http-equiv", u8"lang",     u8"language",
      u8"link",           u8"media",     u8"method",    u8"multiple",   u8"nohref",   u8"noresize",
      u8"noshade",        u8"nowrap",    u8"readonly",  u8"rel",        u8"rev",      u8"rules",
      u8"scope",          u8"scrolling", u8"selected",  u8"shape",      u8"target",   u8"text",
      u8"type",           u8"valign",    u8"valuetype", u8"vlink",
    };
    auto &prefix = attributeName.NamespacePrefix();
    auto isPossibleHTMLAttr = (prefix == DOMStringAtom::Null() || prefix.View().empty())
                              && attributeName.NamespaceURI() == DOMStringAtom::Null();
    return !isPossibleHTMLAttr
           || std::ranges::find(names, attributeName.LocalName().View()) == std::end(names);
  }
}
