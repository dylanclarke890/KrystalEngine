/*
 * Copyright (C) 2006-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Core/Colour/ColourTextStream.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  Ref<Gradient> Gradient::Create(Data &&data, ColourInterpolationMethod colorInterpolationMethod,
                                 GradientSpreadMethod spreadMethod, GradientColorStops &&stops,
                                 bool isTransient) noexcept
  {
    return AdoptRef(*new Gradient(krys::move(data), colorInterpolationMethod, spreadMethod, krys::move(stops),
                                  isTransient));
  }

  Gradient::Gradient(Data &&data, ColourInterpolationMethod colorInterpolationMethod,
                     GradientSpreadMethod spreadMethod, GradientColorStops &&stops, bool isTransient) noexcept
      : _data {krys::move(data)}, _colorInterpolationMethod {colorInterpolationMethod},
        _spreadMethod {spreadMethod}, _stops {krys::move(stops)}, _isTransient {isTransient}
  {
  }

  Gradient::~Gradient() noexcept = default;

  void Gradient::AdjustParametersForTiledDrawing(FloatSize &size, FloatRect &srcRect,
                                                 const FloatSize &spacing) noexcept
  {
    if (srcRect.IsEmpty())
    {
      return;
    }

    if (!spacing.IsZero())
    {
      return;
    }

    krys::SwitchOn(
      _data,
      [&](const LinearData &data)
      {
        if (data.Point0.X() == data.Point1.X())
        {
          size.SetWidth(1);
          srcRect.SetWidth(1);
          srcRect.SetX(0);
          return;
        }
        if (data.Point0.Y() != data.Point1.Y())
        {
          return;
        }

        size.SetHeight(1);
        srcRect.SetHeight(1);
        srcRect.SetY(0);
      },
      [](const RadialData &) {}, [](const ConicData &) {});
  }

  bool Gradient::IsZeroSize() const noexcept
  {
    return krys::SwitchOn(
      _data, [](const LinearData &data)
      { return data.Point0.X() == data.Point1.X() && data.Point0.Y() == data.Point1.Y(); },
      [](const RadialData &data)
      {
        return data.Point0.X() == data.Point1.X() && data.Point0.Y() == data.Point1.Y()
               && data.StartRadius == data.EndRadius;
      },
      [](const ConicData &) { return false; });
  }

  void Gradient::AddColorStop(GradientColorStop &&stop) noexcept
  {
    _stops.AddColorStop(krys::move(stop));
    _cachedHash = 0;
  }

  uint32 Gradient::Hash() const noexcept
  {
    if (!_cachedHash)
    {
      auto combine = [](size_t hash, float value)
      {
        return UnsignedIntegerPairHash(hash, FloatingPointHash<float>::Hash(value));
      };
      auto point = [&](size_t hash, const FloatPoint &value)
      {
        return combine(combine(hash, value.X()), value.Y());
      };

      size_t hash = std::visit(
        CreateVisitor(
          [&](const LinearData &data) { return point(point(1u, data.Point0), data.Point1); },
          [&](const RadialData &data)
          {
            return combine(
              combine(combine(point(point(2u, data.Point0), data.Point1), data.StartRadius), data.EndRadius),
              data.AspectRatio);
          },
          [&](const ConicData &data) { return combine(point(3u, data.Point0), data.AngleRadians); }),
        _data);
      hash = UnsignedIntegerPairHash(hash,
                                     DefaultHash<ColourInterpolationMethod>::Hash(_colorInterpolationMethod));
      hash = UnsignedIntegerPairHash(hash, static_cast<size_t>(_spreadMethod));
      for (const auto &stop : _stops.GetSorted().Stops())
      {
        hash = combine(hash, stop.Offset);
        hash = UnsignedIntegerPairHash(hash, DefaultHash<Color>::Hash(stop.Color));
      }
      // Zero means the hash isn't computed yet.
      _cachedHash = static_cast<uint32>(hash) | 1u;
    }
    return _cachedHash;
  }

  TextStream &operator<<(TextStream &ts, const Gradient &gradient) noexcept
  {
    std::visit(CreateVisitor(
                 [&](const Gradient::LinearData &data)
                 {
                   ts.DumpProperty(u8"p0", data.Point0);
                   ts.DumpProperty(u8"p1", data.Point1);
                 },
                 [&](const Gradient::RadialData &data)
                 {
                   ts.DumpProperty(u8"p0", data.Point0);
                   ts.DumpProperty(u8"p1", data.Point1);
                   ts.DumpProperty(u8"start-radius", data.StartRadius);
                   ts.DumpProperty(u8"end-radius", data.EndRadius);
                   ts.DumpProperty(u8"aspect-ratio", data.AspectRatio);
                 },
                 [&](const Gradient::ConicData &data)
                 {
                   ts.DumpProperty(u8"p0", data.Point0);
                   ts.DumpProperty(u8"angle-radians", data.AngleRadians);
                 }),
               gradient.GetData());
    ts.DumpProperty(u8"color-interpolation-method", gradient.GetColorInterpolationMethod());
    ts.DumpProperty(u8"spread-method", gradient.SpreadMethod());
    ts.DumpProperty(u8"stops", gradient.Stops());
    return ts;
  }
}
