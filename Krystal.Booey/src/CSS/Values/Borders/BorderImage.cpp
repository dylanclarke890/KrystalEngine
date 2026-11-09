/*
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
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

#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css
{
  Ref<ValueList> CreateBorderImageValue(BorderImageComponents &&components) noexcept
  {
    ValueListBuilder list;
    if (components.Source)
    {
      list.push_back(krys::move(components.Source));
    }

    if (components.Width || components.Outset)
    {
      ValueListBuilder listSlash;
      if (components.Slice)
      {
        listSlash.push_back(krys::move(components.Slice));
      }

      if (components.Width)
      {
        listSlash.push_back(krys::move(components.Width));
      }

      if (components.Outset)
      {
        listSlash.push_back(krys::move(components.Outset));
      }

      list.push_back(ValueList::CreateSlashSeparated(krys::move(listSlash)));
    }
    else if (components.Slice)
    {
      list.push_back(krys::move(components.Slice));
    }

    if (components.Repeat)
    {
      list.push_back(krys::move(components.Repeat));
    }

    return ValueList::CreateSpaceSeparated(krys::move(list));
  }
}
