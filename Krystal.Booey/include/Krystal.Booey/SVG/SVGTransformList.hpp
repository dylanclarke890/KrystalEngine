/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValuePropertyList.hpp"
#include "Krystal.Booey/SVG/SVGTransform.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/coords.html#InterfaceSVGTransformList
  class SVGTransformList final : public SVGValuePropertyList<SVGTransform>
  {
    friend class SVGViewSpec;
    using Base = SVGValuePropertyList<SVGTransform>;
    using Base::Base;

    enum class ListReplacement : bool
    {
      Append,
      Replace
    };

  public:
    KRYS_NODISCARD static Ref<SVGTransformList> Create() noexcept
    {
      return AdoptRef(*new SVGTransformList());
    }

    KRYS_NODISCARD static Ref<SVGTransformList> Create(SVGPropertyOwner *owner,
                                                       SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGTransformList(owner, access));
    }

    KRYS_NODISCARD static Ref<SVGTransformList> Create(const SVGTransformList &other,
                                                       SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGTransformList(other, access));
    }

    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGTransform>>
      CreateSVGTransformFromMatrix(css::DOMMatrix2DInit &&matrixInit) noexcept
    {
      Ref svgTransform = SVGTransform::Create();
      (void)svgTransform->SetMatrix(krys::move(matrixInit));
      return svgTransform;
    }

    dom::ExceptionOr<RefPtr<SVGTransform>> Consolidate() noexcept;
    KRYS_NODISCARD AffineTransform Concatenate() const noexcept;

    void Parse(dom::DOMStringView) noexcept;
    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override;

  private:
    bool ParseGeneric(dom::DOMStringView &buffer, ListReplacement = ListReplacement::Append) noexcept;
  };
}
