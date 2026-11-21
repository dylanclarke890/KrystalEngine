/**
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2000 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
 * Copyright (C) 2014-2021 Google Inc. All rights reserved.
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
 */

#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"

#define COMPUTED_STYLE_PROPERTIES_GETTERS_INLINES_INCLUDE_TRAP 1
#include "Krystal.Booey/Style/Computed/ComputedStyleProperties+GettersInlines.hpp"
#undef COMPUTED_STYLE_PROPERTIES_GETTERS_INLINES_INCLUDE_TRAP

namespace krys::boo::style
{
  inline bool ComputedStyle::ColumnSpanEqual(const ComputedStyle &other) const noexcept
  {
    if (_nonInheritedData.get() == other._nonInheritedData.get()
        || _nonInheritedData->MiscData.get() == other._nonInheritedData->MiscData.get()
        || _nonInheritedData->MiscData->MultiCol.get() == other._nonInheritedData->MiscData->MultiCol.get())
    {
      return true;
    }

    return _nonInheritedData->MiscData->MultiCol->ColumnSpan
           == other._nonInheritedData->MiscData->MultiCol->ColumnSpan;
  }

  inline bool ComputedStyle::ContainerTypeAndNamesEqual(const ComputedStyle &other) const noexcept
  {
    if (_nonInheritedData.get() == other._nonInheritedData.get()
        || _nonInheritedData->RareData.get() == other._nonInheritedData->RareData.get())
    {
      return true;
    }

    return ContainerType() == other.ContainerType() && ContainerNames() == other.ContainerNames();
  }

  inline bool ComputedStyle::ScrollPaddingEqual(const ComputedStyle &other) const noexcept
  {
    if (_nonInheritedData.get() == other._nonInheritedData.get()
        || _nonInheritedData->RareData.get() == other._nonInheritedData->RareData.get())
    {
      return true;
    }

    return _nonInheritedData->RareData->ScrollPadding == other._nonInheritedData->RareData->ScrollPadding;
  }

  inline bool ComputedStyle::FontCascadeEqual(const ComputedStyle &other) const noexcept
  {
    return _inheritedData.get() == other._inheritedData.get()
           || _inheritedData->FontData.get() == other._inheritedData->FontData.get()
           || _inheritedData->FontData->FontCascade == other._inheritedData->FontData->FontCascade;
  }
}
