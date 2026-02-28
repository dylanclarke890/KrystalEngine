#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/Document.hpp"

namespace Krys::HTML
{
  Comment::Comment(Document &document, DOMString &&data) noexcept
      : CharacterData(document, Krys::Move(data), NodeType::COMMENT_NODE)
  {
  }

  DOMString Comment::NodeName() const noexcept
  {
    return u8"#comment";
  }
}