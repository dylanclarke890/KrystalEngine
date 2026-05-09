#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  Comment::Comment(Document &document, DOMString &&data) noexcept
      : CharacterData(document, Krys::Move(data), NodeType::COMMENT_NODE)
  {
  }
}