#include "Krystal.Web/Dom/Nodes/Comment.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web
{
  Comment::Comment(Document &document, DOMString data) noexcept
      : CharacterData(document, krys::move(data), dom::NodeType::COMMENT_NODE)
  {
    // Create a comment node:
    // 1. Let comment be the result of creating a node that implements Comment, given document.
    // 2. Set comment's data to data.
    // 3. Return comment.
  }
}
