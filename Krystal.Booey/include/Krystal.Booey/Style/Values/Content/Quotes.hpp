/*
 * Copyright (C) 2011 Nokia Inc. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 *
 */

#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'quote'> = auto | none | [ <string> <string> ]+
  // https://www.w3.org/TR/css-content-3/#propdef-quotes
  struct Quotes
  {
    using Data = css::SpaceSeparatedRefCountedFixedVector<css::CSSOMString>;

    Quotes(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }

    Quotes(css::keywords::None keyword) noexcept : _value {keyword}
    {
    }

    Quotes(Data &&data) noexcept : _value {krys::move(data)}
    {
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Auto>(_value);
    }
    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::None>(_value);
    }
    KRYS_NODISCARD bool IsQuotes() const noexcept
    {
      return krys::HoldsAlternative<Data>(_value);
    }

    KRYS_NODISCARD const css::CSSOMString &OpenQuote(unsigned index) const noexcept;
    KRYS_NODISCARD const css::CSSOMString &CloseQuote(unsigned index) const noexcept;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const Quotes &) const noexcept = default;

  private:
    Variant<css::keywords::Auto, css::keywords::None, Data> _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Quotes>
  {
    KRYS_NODISCARD Quotes operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Quotes)
