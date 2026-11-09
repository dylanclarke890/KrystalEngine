/*
 * Copyright (C) 2016-2019 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/SVG/Properties/SVGValuePropertyList.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGPointList
  class SVGPointList final : public SVGValuePropertyList<SVGPoint>
  {
    using Base = SVGValuePropertyList<SVGPoint>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGPointList> Create() noexcept
    {
      return AdoptRef(*new SVGPointList());
    }

    KRYS_NODISCARD static Ref<SVGPointList> Create(SVGPropertyOwner *owner, SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGPointList(owner, access));
    }

    KRYS_NODISCARD static Ref<SVGPointList> Create(const SVGPointList &other,
                                                   SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGPointList(other, access));
    }

    bool Parse(dom::DOMStringView) noexcept;
    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override;
  };
}
