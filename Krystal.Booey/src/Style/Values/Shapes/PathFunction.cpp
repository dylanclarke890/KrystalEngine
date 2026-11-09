/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/PathValue.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/TinyLRUCache.hpp"

namespace krys::boo::style
{
  // MARK: - Path Caching

  struct SVGPathTransformedByteStream
  {
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return RawStream.IsEmpty();
    }

    KRYS_NODISCARD boo::Path GetPath() const noexcept
    {
      auto path = svg::BuildPathFromByteStream(RawStream);
      if (Zoom != 1)
      {
        path.Transform(AffineTransform().Scale(Zoom));
      }
      path.Translate(ToFloatSize(Offset));
      return path;
    }

    KRYS_NODISCARD bool operator==(const SVGPathTransformedByteStream &) const noexcept = default;

    svg::SVGPathByteStream RawStream;
    float Zoom;
    FloatPoint Offset;
  };

  struct TransformedByteStreamPathPolicy : TinyLRUCachePolicy<SVGPathTransformedByteStream, boo::Path>
  {
    KRYS_NODISCARD static bool IsKeyNull(const SVGPathTransformedByteStream &stream) noexcept
    {
      return stream.IsEmpty();
    }

    KRYS_NODISCARD static boo::Path CreateValueForKey(const SVGPathTransformedByteStream &stream) noexcept
    {
      return stream.GetPath();
    }
  };

  KRYS_NODISCARD static const boo::Path &CachedTransformedByteStreamPath(const svg::SVGPathByteStream &stream,
                                                                         float zoom,
                                                                         const FloatPoint &offset) noexcept
  {
    static NeverDestroyed<
      TinyLRUCache<SVGPathTransformedByteStream, boo::Path, 4, TransformedByteStreamPathPolicy>>
      cache;
    return cache.get().Get(SVGPathTransformedByteStream {stream, zoom, offset});
  }

  // MARK: - Conversion

  KRYS_NODISCARD static svg::SVGPathByteStream CopySVGPathByteStream(const svg::SVGPathByteStream &source,
                                                                     PathConversion conversion) noexcept
  {
    if (conversion == PathConversion::ForceAbsolute)
    {
      // Only returns the resulting absolute path if the conversion succeeds.
      if (auto result = svg::ConvertSVGPathByteStreamToAbsoluteCoordinates(source))
      {
        return *result;
      }
    }
    return source;
  }

  css::Path::PathData ToCSS<Path::PathData>::operator()(const Path::PathData &value, const RenderStyle &,
                                                        PathConversion conversion) noexcept
  {
    return {CopySVGPathByteStream(value.ByteStream, conversion)};
  }

  Path::PathData ToStyle<css::Path::PathData>::operator()(const css::Path::PathData &value,
                                                          const BuilderState &) noexcept
  {
    return {CopySVGPathByteStream(value.ByteStream, PathConversion::None)};
  }

  css::Path ToCSS<Path>::operator()(const Path &value, const RenderStyle &style,
                                    PathConversion conversion) noexcept
  {
    return {.FillRule = ConvertToCSS(value.FillRule, style),
            .Data = ConvertToCSS(value.Data, style, conversion)};
  }

  Path ToStyle<css::Path>::operator()(const css::Path &value, const BuilderState &state,
                                      Maybe<float> zoom) noexcept
  {
    return {.FillRule = ConvertToStyle(value.FillRule, state),
            .Data = ConvertToStyle(value.Data, state),
            .Zoom = zoom.value_or(state.Style().UsedZoom())};
  }

  Ref<css::Value> CSSValueCreation<PathFunction>::operator()(css::ValuePool &, const RenderStyle &style,
                                                             const PathFunction &value,
                                                             PathConversion conversion) noexcept
  {
    return css::PathValue::Create(ConvertToCSS(value, style, conversion));
  }

  // MARK: - Serialization

  void Serialiser<Path>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                    const RenderStyle &style, const Path &value,
                                    PathConversion conversion) noexcept
  {
    css::Serialise(builder, context, ConvertToCSS(value, style, conversion));
  }

  // MARK: - Path

  boo::Path PathComputation<Path>::operator()(const Path &value, const FloatRect &boundingBox) noexcept
  {
    return CachedTransformedByteStreamPath(value.Data.ByteStream, value.Zoom, boundingBox.Location());
  }

  // MARK: - Wind Rule

  boo::WindRule WindRuleComputation<Path>::operator()(const Path &value) noexcept
  {
    return (!value.FillRule || std::holds_alternative<css::keywords::Nonzero>(*value.FillRule))
             ? WindRule::NonZero
             : WindRule::EvenOdd;
  }

  // MARK: - Blending

  bool Blending<Path>::CanBlend(const Path &a, const Path &b) noexcept
  {
    return ComputeWindRule(a) == ComputeWindRule(b)
           && svg::CanBlendSVGPathByteStreams(a.Data.ByteStream, b.Data.ByteStream);
  }

  Path Blending<Path>::Blend(const Path &a, const Path &b, const BlendingContext &context) noexcept
  {
    svg::SVGPathByteStream resultingPathBytes;
    (void)svg::BuildAnimatedSVGPathByteStream(a.Data.ByteStream, b.Data.ByteStream, resultingPathBytes,
                                              static_cast<float>(context.Progress));

    return {.FillRule = a.FillRule, .Data = {krys::move(resultingPathBytes)}, .Zoom = a.Zoom};
  }
}
