#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class Document;

  class Text : public CharacterData
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Text);

  public:
    Text(Document &document, DOMString &&data, NodeType type = NodeType::TEXT_NODE,
         NodeFlag flags = NodeFlag::None) noexcept;

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept override;

#pragma endregion

#pragma region Text

    KRYS_NODISCARD ExceptionOr<Ref<Text>> SplitText(size_t offset) noexcept;

    KRYS_NODISCARD DOMString WholeText() const noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Text)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsTextNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();