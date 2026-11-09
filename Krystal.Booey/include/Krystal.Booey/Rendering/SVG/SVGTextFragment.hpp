/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief A text fragment of a RenderSVGInlineText which can be rendered at once.
  struct SVGTextFragment
  {
    enum class TransformType : bool
    {
      TransformRespectingTextLength,
      TransformIgnoringTextLength
    };

    /// @brief The first rendered character starts at this byte offset of the RenderSVGInlineText's text.
    uint32 CharacterOffset {0};
    uint32 MetricsListOffset {0};
    /// @brief In bytes.
    uint32 Length : 31 {0};
    bool IsTextOnPath : 1 {false};

    float X {0};
    float Y {0};
    float Width {0};
    float Height {0};

    /// @brief Includes rotation/glyph-orientation-(horizontal|vertical) transforms, as well as orientation
    /// related shifts (see SVGTextLayoutEngine, which builds this transformation).
    AffineTransform Transform;

    /// @brief Contains lengthAdjust related transformations, which are not allowd to influence the
    /// SVGTextQuery code.
    AffineTransform LengthAdjustTransform;

    void
      BuildFragmentTransform(AffineTransform &result,
                             TransformType type = TransformType::TransformRespectingTextLength) const noexcept
    {
      if (type == TransformType::TransformIgnoringTextLength)
      {
        result = Transform;
        TransformAroundOrigin(result);
        return;
      }

      if (IsTextOnPath)
      {
        BuildTransformForTextOnPath(result);
      }
      else
      {
        BuildTransformForTextOnLine(result);
      }
    }

  private:
    void TransformAroundOrigin(AffineTransform &result) const noexcept
    {
      // Returns (translate(x, y) * result) * translate(-x, -y).
      result.SetE(result.E() + X);
      result.SetF(result.F() + Y);
      result.Translate(-X, -Y);
    }

    void BuildTransformForTextOnPath(AffineTransform &result) const noexcept
    {
      // For text-on-path layout, multiply the transform with the lengthAdjustTransform before orienting the
      // resulting transform.
      result = LengthAdjustTransform.IsIdentity() ? Transform : Transform * LengthAdjustTransform;
      if (!result.IsIdentity())
      {
        TransformAroundOrigin(result);
      }
    }

    void BuildTransformForTextOnLine(AffineTransform &result) const noexcept
    {
      // For text-on-line layout, orient the transform first, then multiply the lengthAdjustTransform with the
      // oriented transform.
      if (Transform.IsIdentity())
      {
        result = LengthAdjustTransform;
        return;
      }

      result = Transform;
      TransformAroundOrigin(result);

      if (!LengthAdjustTransform.IsIdentity())
      {
        result = LengthAdjustTransform * result;
      }
    }
  };
}
