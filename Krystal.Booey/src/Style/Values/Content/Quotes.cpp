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

#include "Krystal.Booey/Style/Values/Content/Quotes.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD const css::CSSOMString &EmptyString() noexcept
    {
      static const css::CSSOMString empty;
      return empty;
    }
  }

  const css::CSSOMString &Quotes::OpenQuote(unsigned index) const noexcept
  {
    return krys::SwitchOn(
      _value,
      [&](const Data &data) -> const css::CSSOMString &
      {
        auto i = index * 2;

        if (i < data.size())
        {
          return data[i];
        }
        return data[data.size() - 2];
      },
      [&](const auto &) -> const css::CSSOMString & { return EmptyString(); });
  }

  const css::CSSOMString &Quotes::CloseQuote(unsigned index) const noexcept
  {
    return krys::SwitchOn(
      _value,
      [&](const Data &data) -> const css::CSSOMString &
      {
        auto i = (index * 2) + 1;

        if (i < data.size())
        {
          return data[i];
        }
        return data[data.size() - 1];
      },
      [&](const auto &) -> const css::CSSOMString & { return EmptyString(); });
  }

  // MARK: - Conversion

  Quotes CSSValueConversion<Quotes>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto: return css::keywords::Auto {};
        case css::ValueId::None: return css::keywords::None {};
        default:                 break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue, 2>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    if (list->size() % 2 != 0)
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    return Quotes::Data::map(*list, [](const css::PrimitiveValue &item) { return item.StringValue(); });
  }
}
