#pragma once

#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Document;

  class Comment : public CharacterData
  {
  public:
    Comment(Document &document, DOMString &&data = u8"") noexcept;

    KRYS_NODISCARD DOMString NodeName() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Comment)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.GetNodeType() == Krys::HTML::NodeType::COMMENT_NODE;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()