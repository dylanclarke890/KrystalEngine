#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"

namespace Krys::HTML
{
  class Document;

  class Text : public CharacterData
  {
  public:
    Text(Document &document, DOMString &&data = u8"", NodeType type = NodeType::TEXT_NODE,
         NodeFlag flags = NodeFlag::None) noexcept;

    Ref<Text> SplitText(size_t offset) noexcept;
    DOMString WholeText() const noexcept;

    KRYS_NODISCARD DOMString NodeName() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Text)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsTextNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()