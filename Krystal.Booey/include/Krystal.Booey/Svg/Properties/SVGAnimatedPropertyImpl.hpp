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

#include "Krystal.Booey/Svg/Properties/SVGAnimatedDecoratedProperty.hpp"
#include "Krystal.Booey/Svg/Properties/SVGAnimatedPrimitiveProperty.hpp"
#include "Krystal.Booey/Svg/Properties/SVGAnimatedPropertyList.hpp"
#include "Krystal.Booey/Svg/Properties/SVGAnimatedString.hpp"
#include "Krystal.Booey/Svg/Properties/SVGAnimatedValueProperty.hpp"
#include "Krystal.Booey/Svg/Properties/SVGDecoratedEnumeration.hpp"
#include "Krystal.Booey/Svg/SVGAngle.hpp"
#include "Krystal.Booey/Svg/SVGLength.hpp"
#include "Krystal.Booey/Svg/SVGLengthList.hpp"
#include "Krystal.Booey/Svg/SVGMarkerTypes.hpp"
#include "Krystal.Booey/Svg/SVGNumberList.hpp"
#include "Krystal.Booey/Svg/SVGPathSegList.hpp"
#include "Krystal.Booey/Svg/SVGPointList.hpp"
#include "Krystal.Booey/Svg/SVGPreserveAspectRatio.hpp"
#include "Krystal.Booey/Svg/SVGRect.hpp"
#include "Krystal.Booey/Svg/SVGTransformList.hpp"

namespace krys::boo
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
}

namespace krys::boo::svg
{
  // Instantiated once in SVGAnimatedPropertyImpl.cpp: their virtual members are then compiled the same way by
  // every compiler (whether an implicit instantiation includes unused virtuals is unspecified), and once
  // rather than in every file that uses them.
  extern template class SVGSharedPrimitiveProperty<bool>;
  extern template class SVGSharedPrimitiveProperty<int32>;
  extern template class SVGSharedPrimitiveProperty<float>;
  extern template class SVGAnimatedPrimitiveProperty<bool>;
  extern template class SVGAnimatedPrimitiveProperty<int32>;
  extern template class SVGAnimatedPrimitiveProperty<float>;
  extern template class SVGAnimatedValueProperty<SVGAngle>;
  extern template class SVGAnimatedValueProperty<SVGLength>;
  extern template class SVGAnimatedValueProperty<SVGRect>;
  extern template class SVGAnimatedValueProperty<SVGPreserveAspectRatio>;
  extern template class SVGAnimatedPropertyList<SVGLengthList>;
  extern template class SVGAnimatedPropertyList<SVGNumberList>;
  extern template class SVGAnimatedPropertyList<SVGPointList>;
  extern template class SVGAnimatedPropertyList<SVGTransformList>;
  extern template class SVGAnimatedPropertyList<SVGPathSegList>;
}

namespace krys::boo
{
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
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved): the base moves it.
    using SVGAnimatedEnumeration::SVGAnimatedEnumeration;

    KRYS_NODISCARD static Ref<SVGAnimatedOrientType> Create(SVGElement *contextElement,
                                                            SVGMarkerOrientType baseValue) noexcept
    {
      return SVGAnimatedEnumeration::Create<SVGMarkerOrientType, SVGAnimatedOrientType>(contextElement,
                                                                                        baseValue);
    }
  };
}
