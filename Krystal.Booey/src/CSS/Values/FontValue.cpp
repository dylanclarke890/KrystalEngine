/**
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/FontValue.hpp"

namespace krys::boo::css
{
  CSSOMString FontValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    // font variant weight size / line-height family
    CSSOMString result;
    auto append = [&](CSSOMStringView separator, const Value &value)
    {
      result.append(separator);
      result.append(value.CssText(context));
    };

    if (Style)
    {
      result.append(Style->CssText(context));
    }
    if (Variant)
    {
      append(result.empty() ? u8"" : u8" ", *Variant);
    }
    if (Weight)
    {
      append(result.empty() ? u8"" : u8" ", *Weight);
    }
    if (Width)
    {
      append(result.empty() ? u8"" : u8" ", *Width);
    }
    if (Size)
    {
      append(result.empty() ? u8"" : u8" ", *Size);
    }
    if (LineHeight)
    {
      append(Size ? u8" / " : result.empty() ? u8"" : u8" ", *LineHeight);
    }
    if (Family)
    {
      append(result.empty() ? u8"" : u8" ", *Family);
    }
    return result;
  }

  bool FontValue::Equals(const FontValue &other) const noexcept
  {
    return CompareValuePtr(Style, other.Style) && CompareValuePtr(Variant, other.Variant)
           && CompareValuePtr(Weight, other.Weight) && CompareValuePtr(Width, other.Width)
           && CompareValuePtr(Size, other.Size) && CompareValuePtr(LineHeight, other.LineHeight)
           && CompareValuePtr(Family, other.Family);
  }

  IterationStatus
    FontValue::CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    if (Style)
    {
      if (func(*Style) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Variant)
    {
      if (func(*Variant) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Weight)
    {
      if (func(*Weight) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Width)
    {
      if (func(*Width) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Size)
    {
      if (func(*Size) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (LineHeight)
    {
      if (func(*LineHeight) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Family)
    {
      if (func(*Family) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    return IterationStatus::Continue;
  }
}
