/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class FontCascade;
  class RenderStyle;
  class RenderView;
}

namespace krys::boo::css
{
  class ToLengthConversionData;

  enum class LengthUnit : uint8;
  enum class PropertyId : uint16;
}

namespace krys::boo::style
{
  // FIXME: These functions have odd names and invariants and could use improvements.

  // NOTE: `ComputeUnzoomedNonCalcLengthDouble` has the following restrictions:
  //
  // It can never be called with the following LengthUnits:
  //    Lh, Rlh, Cqw, Cqh, Cqi, Cqb, Cqmin, Cqmax (line height, and container-percentage units)
  //
  // If `fontCascadeForUnit` is nullptr, it additionally cannot be called with the following LengthUnits:
  //    Em, QuirkyEm, Ex, Cap, Ch, Ic, Rcap, Rch, Rem, Rex, Ric (font and root font dependent units)
  //
  // If `RenderView` is nullptr, the following LengthUnits will all cause a return value of zero:
  //    Vw, Vh, Vb, Vi, Svw, Svh, Svb, Svi, Lvw, Lvh, Lvb, Lvi, Dvw, Dvh, Dvb, Dvi (viewport-percentage units)
  //    (the *min and *max viewport-percentage units return `value` unchanged instead)
  KRYS_NODISCARD double ComputeUnzoomedNonCalcLengthDouble(
    double value, css::LengthUnit, css::PropertyId, const FontCascade *fontCascadeForUnit = nullptr,
    css::RangeZoomOptions = css::RangeZoomOptions::Default, const RenderView * = nullptr) noexcept;

  KRYS_NODISCARD double ComputeNonCalcLengthDouble(double value, css::LengthUnit,
                                                   const css::ToLengthConversionData &) noexcept;

  KRYS_NODISCARD double ComputeCanonicalNonCalcLengthDouble(double value, css::LengthUnit,
                                                            const css::ToLengthConversionData &) noexcept;

  // True if `ComputeNonCalcLengthDouble` would produce identical results when resolved against both these
  // styles.
  KRYS_NODISCARD bool EqualForLengthResolution(const RenderStyle &, const RenderStyle &) noexcept;
}
