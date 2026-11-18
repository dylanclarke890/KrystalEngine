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

namespace krys::boo
{
  /// @brief The `CSS` namespace's functions that make unit values: `CSS.px(10)` is
  /// `CSSNumericFactory::Px(10)`.
  /// @see https://drafts.css-houdini.org/css-typed-om/#numeric-factory
  struct CSSNumericFactory
  {
    KRYS_NODISCARD static Ref<CSSUnitValue> Number(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::Number);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Percent(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::Percentage);
    }

    // <length>
    KRYS_NODISCARD static Ref<CSSUnitValue> Em(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::em);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Rem(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::rem);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Ex(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::ex);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Rex(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::rex);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Cap(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::cap);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Rcap(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::rcap);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Ch(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::ch);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Rch(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::rch);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Ic(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::ic);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Ric(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::ric);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Lh(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::lh);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Rlh(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::rlh);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Vw(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::vw);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Vh(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::vh);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Vi(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::vi);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Vb(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::vb);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Vmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::vmin);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Vmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::vmax);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Cm(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::cm);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Mm(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::mm);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Q(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::Q);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> In(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::in);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Pt(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::pt);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Pc(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::pc);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Px(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::px);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Cqw(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::cqw);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Cqh(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::cqh);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Cqi(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::cqi);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Cqb(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::cqb);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Cqmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::cqmin);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Cqmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::cqmax);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Svw(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::svw);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Svh(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::svh);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Svi(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::svi);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Svb(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::svb);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Svmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::svmin);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Svmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::svmax);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Lvw(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::lvw);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Lvh(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::lvh);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Lvi(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::lvi);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Lvb(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::lvb);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Lvmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::lvmin);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Lvmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::lvmax);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Dvw(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dvw);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Dvh(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dvh);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Dvi(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dvi);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Dvb(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dvb);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Dvmin(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dvmin);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Dvmax(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dvmax);
    }

    // <angle>
    KRYS_NODISCARD static Ref<CSSUnitValue> Deg(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::deg);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Grad(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::grad);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Rad(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::rad);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Turn(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::turn);
    }

    // <time>
    KRYS_NODISCARD static Ref<CSSUnitValue> S(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::s);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Ms(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::ms);
    }

    // <frequency>
    KRYS_NODISCARD static Ref<CSSUnitValue> Hz(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::hz);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> KHz(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::khz);
    }

    // <resolution>
    KRYS_NODISCARD static Ref<CSSUnitValue> Dpi(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dpi);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Dpcm(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dpcm);
    }

    KRYS_NODISCARD static Ref<CSSUnitValue> Dppx(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::dppx);
    }

    // <flex>
    KRYS_NODISCARD static Ref<CSSUnitValue> Fr(double value) noexcept
    {
      return CSSUnitValue::Create(value, css::UnitType::fr);
    }
  };
}
