#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Element;

  /// @brief The state of an element's `dir` attribute.
  /// @see https://html.spec.whatwg.org/#attr-dir
  enum class TextDirectionState : uint8
  {
    LTR,
    RTL,
    Auto,
    Undefined,
  };

  /// @brief The state a `dir` attribute's @p value (null if the attribute is missing) puts it in.
  KRYS_NODISCARD TextDirectionState ParseTextDirectionState(Maybe<DOMStringView> value) noexcept;

  /// @brief The state of @p element's `dir` attribute.
  KRYS_NODISCARD TextDirectionState ElementTextDirectionState(const Element &element) noexcept;

  /// @brief Whether @p element's `dir` attribute is `ltr`, `rtl` or `auto`.
  KRYS_NODISCARD bool ElementHasValidTextDirectionState(const Element &element) noexcept;

  /// @brief Whether @p element's directionality is its auto directionality: its `dir` attribute is `auto`, or
  /// it's a `<bdi>` without a valid one.
  KRYS_NODISCARD bool ElementHasAutoTextDirectionState(const Element &element) noexcept;
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::TextDirectionState, 4u);
