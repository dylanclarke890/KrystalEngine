#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"

namespace Krys::HTML
{
  Comment::Comment(Document &document, DOMString &&data) noexcept
      : CharacterData(document, Krys::Move(data), NodeType::COMMENT_NODE)
  {
  }
}