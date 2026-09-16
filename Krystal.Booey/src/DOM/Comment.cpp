#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  Comment::Comment(Document &document, DOMString &&data) noexcept
      : CharacterData(document, krys::move(data), NodeType::COMMENT_NODE)
  {
  }
}