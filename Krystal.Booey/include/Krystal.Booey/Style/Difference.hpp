/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
