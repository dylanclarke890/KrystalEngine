/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/LineWidth.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"

namespace krys::boo
{
  /// @brief One edge of an element's border: its colour, width and style.
  struct BorderValue
  {
    style::Color Color {style::Color::GetCurrentColor()};
    style::LineWidth Width {style::LineWidth::Length {3.0f}};
    uint8 Style : 4 {static_cast<uint8>(BorderStyle::None)};

    KRYS_NODISCARD bool IsVisible() const noexcept;
    KRYS_NODISCARD bool NonZero() const noexcept;
    KRYS_NODISCARD bool IsTransparent() const noexcept;

    KRYS_NODISCARD bool HasHiddenStyle() const noexcept
    {
      return static_cast<BorderStyle>(Style) == BorderStyle::Hidden;
    }
    KRYS_NODISCARD bool HasVisibleStyle() const noexcept
    {
      return static_cast<BorderStyle>(Style) > BorderStyle::Hidden;
    }

    KRYS_NODISCARD bool operator==(const BorderValue &) const noexcept = default;
  };

  inline bool BorderValue::NonZero() const noexcept
  {
    return Width && static_cast<BorderStyle>(Style) != BorderStyle::None;
  }
}
