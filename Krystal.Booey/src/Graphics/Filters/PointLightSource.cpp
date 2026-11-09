/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010 Zoltan Herczeg <zherczeg@webkit.org>
 * Copyright (C) 2011 University of Szeged
 * Copyright (C) 2011 Renata Hodovan <reni@webkit.org>
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Filters/PointLightSource.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<PointLightSource> PointLightSource::Create(const FloatPoint3D &position) noexcept
  {
    return AdoptRef(*new PointLightSource(position));
  }

  PointLightSource::PointLightSource(const FloatPoint3D &position) noexcept
      : LightSource(LightType::LS_POINT), _position(position)
  {
  }

  bool PointLightSource::operator==(const PointLightSource &other) const noexcept
  {
    return LightSource::operator==(other) && _position == other._position;
  }

  void PointLightSource::InitPaintingData(const Filter &filter, const FilterImage &result,
                                          PaintingData &) const noexcept
  {
    auto position = filter.ResolvedPoint3D(_position);
    auto absolutePosition = filter.ScaledByFilterScale(position.XY());
    _bufferPosition.SetXY(result.MappedAbsolutePoint(absolutePosition));

    // To scale Z, map a point offset from position in the x direction by z.
    auto absoluteMappedZ = filter.ScaledByFilterScale(FloatPoint {position.X() + position.Z(), position.Y()});
    _bufferPosition.SetZ(result.MappedAbsolutePoint(absoluteMappedZ).X() - _bufferPosition.X());
  }

  LightSource::ComputedLightingData
    PointLightSource::ComputePixelLightingData(const PaintingData &paintingData, int32 x, int32 y,
                                               float z) const noexcept
  {
    FloatPoint3D lightVector = {_bufferPosition.X() - static_cast<float>(x),
                                _bufferPosition.Y() - static_cast<float>(y), _bufferPosition.Z() - z};

    return {lightVector, paintingData.InitialLightingData.ColorVector, lightVector.Length()};
  }

  bool PointLightSource::SetX(float x) noexcept
  {
    if (_position.X() == x)
    {
      return false;
    }
    _position.SetX(x);
    return true;
  }

  bool PointLightSource::SetY(float y) noexcept
  {
    if (_position.Y() == y)
    {
      return false;
    }
    _position.SetY(y);
    return true;
  }

  bool PointLightSource::SetZ(float z) noexcept
  {
    if (_position.Z() == z)
    {
      return false;
    }
    _position.SetZ(z);
    return true;
  }

  TextStream &PointLightSource::ExternalRepresentation(TextStream &ts) const noexcept
  {
    ts << u8"[type=POINT-LIGHT] ";
    ts << u8"[position=\"" << Position() << u8"\"]";
    return ts;
  }
}
