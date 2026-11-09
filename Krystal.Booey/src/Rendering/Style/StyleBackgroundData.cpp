/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004-2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleBackgroundData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleBackgroundData::StyleBackgroundData() noexcept
      : Background(css::keywords::None {}), BackgroundColor(style::ComputedStyle::InitialBackgroundColor())
  {
  }

  inline StyleBackgroundData::StyleBackgroundData(const StyleBackgroundData &other) noexcept
      : RefCounted<StyleBackgroundData>(), Background(other.Background),
        BackgroundColor(other.BackgroundColor), Outline(other.Outline)
  {
  }

  Ref<StyleBackgroundData> StyleBackgroundData::Copy() const noexcept
  {
    return AdoptRef(*new StyleBackgroundData(*this));
  }

  bool StyleBackgroundData::operator==(const StyleBackgroundData &other) const noexcept
  {
    return Background == other.Background && BackgroundColor == other.BackgroundColor
           && Outline == other.Outline;
  }

  bool StyleBackgroundData::ContainsCurrentColor() const noexcept
  {
    return BackgroundColor.ContainsCurrentColor() || Outline.OutlineColor.ContainsCurrentColor();
  }
}
