#include "Krystal.Booey/Dom/Nodes/Comment.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"

namespace krys::boo
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
