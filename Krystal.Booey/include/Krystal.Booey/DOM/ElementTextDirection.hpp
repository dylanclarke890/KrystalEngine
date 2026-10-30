#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  enum class TextDirection : bool;
}

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

  /// @brief @p element's auto directionality: from its value if it's a form control whose value decides it,
  /// otherwise from the first of its text with a strong character. Null if nothing decides it.
  /// @see https://html.spec.whatwg.org/#auto-directionality
  KRYS_NODISCARD Maybe<TextDirection> ComputeAutoDirectionality(const Element &element) noexcept;

  /// @brief @p element's directionality if it's its auto directionality; null otherwise.
  KRYS_NODISCARD Maybe<TextDirection> ComputeTextDirectionIfDirIsAuto(const Element &element) noexcept;

  /// @brief Updates the directionality of @p element and of what it decides after its `dir` attribute
  /// changed to @p textDirectionState.
  void TextDirectionStateChanged(Element &element, TextDirectionState textDirectionState) noexcept;

  /// @brief Updates the directionality of @p element, its descendants and, if it decides theirs, its
  /// ancestors whose directionality is auto.
  /// @param initiator The element whose change led here, which is left as it is.
  void UpdateEffectiveTextDirectionState(Element &element, TextDirectionState textDirectionState,
                                         Element *initiator = nullptr) noexcept;

  /// @brief Gives @p direction to @p element's descendants (and their shadow trees) that take their
  /// directionality from it.
  void UpdateEffectiveTextDirectionOfDescendants(Element &element, Maybe<TextDirection> direction,
                                                 Element *initiator = nullptr) noexcept;

  /// @brief Updates the directionality of the nearest inclusive ancestor of @p element whose directionality
  /// is auto.
  void UpdateEffectiveTextDirectionOfAncestors(Element &element, Element *initiator) noexcept;
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::TextDirectionState, 4u);
