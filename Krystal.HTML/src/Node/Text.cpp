#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  Text::Text(Document &document, DOMString &&data, NodeType type, NodeFlags flags) noexcept
      : CharacterData(document, Krys::Move(data), type, flags | NodeFlags::IsTextNode)
  {
  }

  /// @see https://dom.spec.whatwg.org/#concept-text-split
  Ref<Text> Text::SplitText(size_t offset) noexcept
  {
    // TODO (IMPL):
    return Ref<Text>::WithRef(*this);
  }

  /// @see https://dom.spec.whatwg.org/#dom-text-wholetext
  DOMString Text::WholeText() const noexcept
  {
    // TODO (IMPL):
    return Data();
  }

  /// @see https://dom.spec.whatwg.org/#exclusive-text-node
  DOMString Text::NodeName() const noexcept
  {
    return u8"#text";
  }
}