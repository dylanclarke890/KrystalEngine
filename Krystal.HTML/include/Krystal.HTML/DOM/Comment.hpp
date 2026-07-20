#pragma once

#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#interface-comment
  class Comment : public CharacterData
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Comment);

  public:
#pragma region Comment - https://dom.spec.whatwg.org/#comment

    /// @see https://dom.spec.whatwg.org/#dom-comment-comment
    Comment(Document &document, DOMString &&data = u8"") noexcept;

#pragma endregion

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return u8"#comment";
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Comment)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsCommentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();