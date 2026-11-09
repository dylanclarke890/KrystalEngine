/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"

namespace krys::boo::css
{
  /// @brief The `CSS` namespace's functions that make unit values: `CSS.px(10)` is
  /// `CSSNumericFactory::Px(10)`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#numeric-factory
  struct CSSNumericFactory
  {
    KRYS_NODISCARD static Ref<CSSUnitValue> Number(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::Number);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Percent(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::Percentage);
    }

    // <length>
    KRYS_NODISCARD static Ref<CSSUnitValue> Em(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::em);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Rem(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::rem);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Ex(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::ex);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Rex(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::rex);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Cap(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::cap);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Rcap(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::rcap);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Ch(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::ch);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Rch(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::rch);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Ic(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::ic);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Ric(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::ric);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Lh(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::lh);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Rlh(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::rlh);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Vw(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::vw);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Vh(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::vh);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Vi(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::vi);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Vb(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::vb);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Vmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::vmin);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Vmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::vmax);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Cm(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::cm);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Mm(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::mm);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Q(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::Q);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> In(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::in);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Pt(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::pt);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Pc(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::pc);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Px(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::px);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Cqw(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::cqw);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Cqh(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::cqh);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Cqi(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::cqi);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Cqb(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::cqb);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Cqmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::cqmin);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Cqmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::cqmax);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Svw(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::svw);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Svh(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::svh);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Svi(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::svi);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Svb(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::svb);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Svmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::svmin);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Svmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::svmax);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Lvw(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::lvw);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Lvh(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::lvh);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Lvi(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::lvi);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Lvb(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::lvb);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Lvmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::lvmin);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Lvmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::lvmax);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Dvw(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dvw);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Dvh(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dvh);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Dvi(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dvi);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Dvb(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dvb);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Dvmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dvmin);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Dvmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dvmax);
    }

    // <angle>
    KRYS_NODISCARD static Ref<CSSUnitValue> Deg(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::deg);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Grad(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::grad);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Rad(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::rad);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Turn(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::turn);
    }

    // <time>
    KRYS_NODISCARD static Ref<CSSUnitValue> S(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::s);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Ms(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::ms);
    }

    // <frequency>
    KRYS_NODISCARD static Ref<CSSUnitValue> Hz(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::hz);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> KHz(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::khz);
    }

    // <resolution>
    KRYS_NODISCARD static Ref<CSSUnitValue> Dpi(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dpi);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Dpcm(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dpcm);
    }
    KRYS_NODISCARD static Ref<CSSUnitValue> Dppx(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::dppx);
    }

    // <flex>
    KRYS_NODISCARD static Ref<CSSUnitValue> Fr(double value) noexcept
    {
      return CSSUnitValue::Create(value, UnitType::fr);
    }
  };
}
