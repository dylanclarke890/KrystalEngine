/*
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedDecoratedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPrimitiveProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyList.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedValueProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGDecoratedEnumeration.hpp"
#include "Krystal.Booey/SVG/SVGAngle.hpp"
#include "Krystal.Booey/SVG/SVGLength.hpp"
#include "Krystal.Booey/SVG/SVGLengthList.hpp"
#include "Krystal.Booey/SVG/SVGMarkerTypes.hpp"
#include "Krystal.Booey/SVG/SVGNumberList.hpp"
#include "Krystal.Booey/SVG/SVGPathSegList.hpp"
#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatio.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"

namespace krys::boo::svg
{
  using SVGAnimatedBoolean = SVGAnimatedPrimitiveProperty<bool>;
  using SVGAnimatedInteger = SVGAnimatedPrimitiveProperty<int32>;
  using SVGAnimatedNumber = SVGAnimatedPrimitiveProperty<float>;
  using SVGAnimatedEnumeration = SVGAnimatedDecoratedProperty<SVGDecoratedEnumeration, uint32>;

  using SVGAnimatedAngle = SVGAnimatedValueProperty<SVGAngle>;
  using SVGAnimatedLength = SVGAnimatedValueProperty<SVGLength>;
  using SVGAnimatedRect = SVGAnimatedValueProperty<SVGRect>;
  using SVGAnimatedPreserveAspectRatio = SVGAnimatedValueProperty<SVGPreserveAspectRatio>;

  using SVGAnimatedLengthList = SVGAnimatedPropertyList<SVGLengthList>;
  using SVGAnimatedNumberList = SVGAnimatedPropertyList<SVGNumberList>;
  using SVGAnimatedPointList = SVGAnimatedPropertyList<SVGPointList>;
  using SVGAnimatedTransformList = SVGAnimatedPropertyList<SVGTransformList>;

  class SVGAnimatedPathSegList : public SVGAnimatedPropertyList<SVGPathSegList>
  {
    using Base = SVGAnimatedPropertyList<SVGPathSegList>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGAnimatedPathSegList> Create(SVGElement *contextElement) noexcept
    {
      return AdoptRef(*new SVGAnimatedPathSegList(contextElement));
    }

    KRYS_NODISCARD const SVGPathByteStream &CurrentPathByteStream() const noexcept
    {
      return IsAnimating() ? AnimVal()->PathByteStream() : BaseVal()->PathByteStream();
    }

    KRYS_NODISCARD Path CurrentPath() const noexcept
    {
      return IsAnimating() ? AnimVal()->GetPath() : BaseVal()->GetPath();
    }
  };

  class SVGAnimatedOrientType : public SVGAnimatedEnumeration
  {
  public:
    using SVGAnimatedEnumeration::SVGAnimatedEnumeration;

    KRYS_NODISCARD static Ref<SVGAnimatedOrientType> Create(SVGElement *contextElement,
                                                            SVGMarkerOrientType baseValue) noexcept
    {
      return SVGAnimatedEnumeration::Create<SVGMarkerOrientType, SVGAnimatedOrientType>(contextElement,
                                                                                        baseValue);
    }
  };
}
