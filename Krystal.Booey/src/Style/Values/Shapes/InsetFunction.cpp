/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/Shapes/InsetFunction.hpp"

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Types/TinyLRUCache.hpp"

namespace krys::boo::style
{
  // MARK: - Path Caching

  struct RoundedInsetPathPolicy : public TinyLRUCachePolicy<FloatRoundedRect, boo::Path>
  {
  public:
    KRYS_NODISCARD static bool IsKeyNull(const FloatRoundedRect &rect) noexcept
    {
      return rect.IsEmpty();
    }

    KRYS_NODISCARD static boo::Path CreateValueForKey(const FloatRoundedRect &rect) noexcept
    {
      boo::Path path;
      path.AddRoundedRect(rect, PathRoundedRect::Strategy::PreferBezier);
      return path;
    }
  };

  static const boo::Path &CachedRoundedInsetPath(const FloatRoundedRect &rect)
  {
    static NeverDestroyed<TinyLRUCache<FloatRoundedRect, boo::Path, 4, RoundedInsetPathPolicy>> cache;
    return cache.get().Get(rect);
  }

  // MARK: - Path

  boo::Path PathComputation<Inset>::operator()(const Inset &value, const FloatRect &boundingBox) noexcept
  {
    auto boundingSize = boundingBox.Size();

    auto left = Evaluate<float>(value.Insets.Left(), boundingSize.Width(), style::ZoomNeeded {});
    auto top = Evaluate<float>(value.Insets.Top(), boundingSize.Height(), style::ZoomNeeded {});
    auto rect = FloatRect {
      left + boundingBox.X(), top + boundingBox.Y(),
      std::max<float>(boundingSize.Width() - left
                        - Evaluate<float>(value.Insets.Right(), boundingSize.Width(), style::ZoomNeeded {}),
                      0),
      std::max<float>(boundingSize.Height() - top
                        - Evaluate<float>(value.Insets.Bottom(), boundingSize.Height(), style::ZoomNeeded {}),
                      0)};

    auto radii = Evaluate<FloatRoundedRect::Radii>(value.Radii, boundingSize, style::ZoomNeeded {});
    radii.Scale(CalcBorderRadiiConstraintScaleFor(rect, radii));

    return CachedRoundedInsetPath(FloatRoundedRect {rect, radii});
  }
}
