/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2010 Zoltan Herczeg <zherczeg@webkit.org>
 * Copyright (C) 2011 University of Szeged
 * Copyright (C) 2011 Renata Hodovan <reni@webkit.org>
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

#include "Krystal.Booey/Graphics/Filters/DistantLightSource.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <cmath>

namespace krys::boo
{
  Ref<DistantLightSource> DistantLightSource::Create(float azimuth, float elevation) noexcept
  {
    return AdoptRef(*new DistantLightSource(azimuth, elevation));
  }

  DistantLightSource::DistantLightSource(float azimuth, float elevation) noexcept
      : LightSource(LightType::LS_DISTANT), _azimuth(azimuth), _elevation(elevation)
  {
  }

  bool DistantLightSource::operator==(const DistantLightSource &other) const noexcept
  {
    return LightSource::operator==(other) && _azimuth == other._azimuth && _elevation == other._elevation;
  }

  void DistantLightSource::InitPaintingData(const Filter &, const FilterImage &,
                                            PaintingData &paintingData) const noexcept
  {
    float azimuth = deg2rad(_azimuth);
    float elevation = deg2rad(_elevation);
    paintingData.InitialLightingData.LightVector = {
      std::cos(azimuth) * std::cos(elevation), std::sin(azimuth) * std::cos(elevation), std::sin(elevation)};
    paintingData.InitialLightingData.LightVectorLength = 1;
  }

  LightSource::ComputedLightingData
    DistantLightSource::ComputePixelLightingData(const PaintingData &paintingData, int32, int32,
                                                 float) const noexcept
  {
    return paintingData.InitialLightingData;
  }

  bool DistantLightSource::SetAzimuth(float azimuth) noexcept
  {
    if (_azimuth == azimuth)
    {
      return false;
    }
    _azimuth = azimuth;
    return true;
  }

  bool DistantLightSource::SetElevation(float elevation) noexcept
  {
    if (_elevation == elevation)
    {
      return false;
    }
    _elevation = elevation;
    return true;
  }

  TextStream &DistantLightSource::ExternalRepresentation(TextStream &ts) const noexcept
  {
    ts << u8"[type=DISTANT-LIGHT] ";
    ts << u8"[azimuth=\"" << Azimuth() << u8"\"]";
    ts << u8"[elevation=\"" << Elevation() << u8"\"]";
    return ts;
  }
}
