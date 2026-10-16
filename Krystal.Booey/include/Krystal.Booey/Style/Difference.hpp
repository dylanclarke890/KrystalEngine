#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include <compare>

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::style
{
  // The difference between two styles.  The following values are used:
  // - DifferenceResult::Equal - The two styles are identical
  // - DifferenceResult::RecompositeLayer - The layer needs its position and transform updated, but no repaint
  // - DifferenceResult::Repaint - The object just needs to be repainted.
  // - DifferenceResult::RepaintIfText - The object needs to be repainted if it contains text.
  // - DifferenceResult::RepaintLayer - The layer and its descendant layers needs to be repainted.
  // - DifferenceResult::LayoutOutOfFlowMovementOnly - Only the position of this out-of-flow box has been
  // updated
  // - DifferenceResult::Overflow - Only overflow needs to be recomputed
  // - DifferenceResult::OverflowAndOutOfFlowMovement - Both out-of-flow movement and overflow updates are
  // required.
  // - DifferenceResult::Layout - A full layout is required.
  enum class DifferenceResult : uint8
  {
    Equal,
    RecompositeLayer,
    Repaint,
    RepaintIfText,
    RepaintLayer,
    LayoutOutOfFlowMovementOnly,
    Overflow,
    OverflowAndOutOfFlowMovement,
    Layout,
    NewStyle
  };

  // When some style properties change, different amounts of work have to be done depending on
  // context (e.g. whether the property is changing on an element which has a compositing layer).
  // A simple DifferenceResult does not provide enough information so we return a bit mask of
  // DifferenceContextSensitiveProperties from GetDifference() as well.
  enum class DifferenceContextSensitiveProperty : uint8
  {
    Transform,
    Opacity,
    Filter,
    ClipRect,
    ClipPath,
    WillChange
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::style::DifferenceContextSensitiveProperty, 6uz);

namespace krys::boo::style
{
  /// @brief How much has to be updated for a renderer whose style changed.
  struct Difference
  {
    DifferenceResult Result {DifferenceResult::Equal};
    EnumSet<DifferenceContextSensitiveProperty> ContextSensitiveProperties {};

    constexpr Difference() noexcept = default;
    constexpr Difference(DifferenceResult result,
                         EnumSet<DifferenceContextSensitiveProperty> contextSensitiveProperties = {}) noexcept
        : Result(result), ContextSensitiveProperties(contextSensitiveProperties)
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const Difference &) const noexcept = default;
    KRYS_NODISCARD constexpr bool operator==(DifferenceResult other) const noexcept
    {
      return Result == other;
    }

    KRYS_NODISCARD constexpr std::strong_ordering operator<=>(const Difference &other) const noexcept
    {
      return Result <=> other.Result;
    }
    KRYS_NODISCARD constexpr std::strong_ordering operator<=>(DifferenceResult other) const noexcept
    {
      return Result <=> other;
    }
  };

  /// @brief What has to be updated for a renderer whose style changes from @p a to @p b.
  KRYS_NODISCARD Difference GetDifference(const RenderStyle &a, const RenderStyle &b) noexcept;

  /// @brief Whether a layer has to be repainted for a style change from @p a to @p b.
  KRYS_NODISCARD bool DifferenceRequiresLayerRepaint(const RenderStyle &a, const RenderStyle &b,
                                                     bool isComposited) noexcept;

  /// @brief Whether the borders of @p a and @p b paint the same.
  KRYS_NODISCARD bool BorderIsEquivalentForPainting(const RenderStyle &a, const RenderStyle &b) noexcept;
}
