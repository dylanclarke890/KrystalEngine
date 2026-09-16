#pragma once

#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::Comment)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsCommentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();