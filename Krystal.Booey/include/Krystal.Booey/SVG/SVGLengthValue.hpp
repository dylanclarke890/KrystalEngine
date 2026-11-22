/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2019-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/SVG/SVGParsingError.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  template <typename T>
  class ExceptionOr;
  class SVGLengthContext;

  enum class SVGLengthType : uint8
  {
    Unknown = 0,
    Number,
    Percentage,
    Ems,
    Exs,
    Pixels,
    Centimeters,
    Millimeters,
    Inches,
    Points,
    Picas,
    Lh,
    Ch,
  };

  consteval SVGLengthType MaxEnumValue(SVGLengthType) noexcept
  {
    return SVGLengthType::Ch;
  }

  enum class SVGLengthMode : uint8
  {
    Width,
    Height,
    Other,
  };

  consteval SVGLengthMode MaxEnumValue(SVGLengthMode) noexcept
  {
    return SVGLengthMode::Other;
  }

  enum class SVGLengthNegativeValuesMode : uint8
  {
    Allow,
    Forbid,
  };

  consteval SVGLengthNegativeValuesMode MaxEnumValue(SVGLengthNegativeValuesMode) noexcept
  {
    return SVGLengthNegativeValuesMode::Forbid;
  }
}

namespace krys::boo::svg
{
  enum class ShouldConvertNumberToPxLength : bool
  {
    No,
    Yes,
  };
}

namespace krys::boo
{
  class SVGLengthValue
  {
  private:
    Variant<css::Number<>, css::LengthPercentage<>> _value;
    SVGLengthMode _lengthMode {SVGLengthMode::Other};

  public:
    explicit SVGLengthValue(SVGLengthMode = SVGLengthMode::Other, DOMStringView valueAsString = {}) noexcept;

    SVGLengthValue(float valueInSpecifiedUnits, SVGLengthType, SVGLengthMode = SVGLengthMode::Other) noexcept;

    SVGLengthValue(const SVGLengthContext &, float, SVGLengthType = SVGLengthType::Number,
                   SVGLengthMode = SVGLengthMode::Other) noexcept;

    KRYS_NODISCARD static SVGLengthValue
      Construct(SVGLengthMode, DOMStringView, SVGParsingError &,
                SVGLengthNegativeValuesMode = SVGLengthNegativeValuesMode::Allow,
                Maybe<DOMStringView> fallbackValue = {}) noexcept;

    KRYS_NODISCARD SVGLengthType LengthType() const noexcept;

    KRYS_NODISCARD SVGLengthMode LengthMode() const noexcept
    {
      return _lengthMode;
    }

    KRYS_NODISCARD bool IsZero() const noexcept;

    KRYS_NODISCARD bool IsRelative() const noexcept;

    KRYS_NODISCARD float Value(const SVGLengthContext &) const noexcept;

    KRYS_NODISCARD float ValueAsPercentage() const noexcept;

    KRYS_NODISCARD float ValueInSpecifiedUnits() const noexcept;

    KRYS_NODISCARD DOMString ValueAsString() const noexcept;

    KRYS_NODISCARD DOMStringAtom ValueAsAtomString() const noexcept;

    KRYS_NODISCARD ExceptionOr<float> ValueForBindings(const SVGLengthContext &) const noexcept;

    void SetValueInSpecifiedUnits(float) noexcept;

    ExceptionOr<void> SetValue(const SVGLengthContext &, float) noexcept;

    ExceptionOr<void> SetValue(const SVGLengthContext &, float, SVGLengthType, SVGLengthMode) noexcept;

    ExceptionOr<void> SetValueAsString(DOMStringView) noexcept;

    ExceptionOr<void> SetValueAsString(DOMStringView, SVGLengthMode) noexcept;

    ExceptionOr<void> ConvertToSpecifiedUnits(const SVGLengthContext &, SVGLengthType) noexcept;

    KRYS_NODISCARD friend bool operator==(const SVGLengthValue &, const SVGLengthValue &) noexcept = default;
  };
}
