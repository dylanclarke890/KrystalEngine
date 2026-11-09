/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2011, 2016 Apple Inc. All right reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2013 ChangSeok Oh <shivamidow@gmail.com>
 * Copyright (C) 2013 Adobe Systems Inc. All right reserved.
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

#include "Krystal.Booey/Rendering/Line/LegacyInlineIterator.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <functional>

namespace krys::boo
{
  class RenderBoxModelObject;
  class RenderText;

  /// @brief The trailing whitespace on a line and the boxes after it, whose transitions collapse it away.
  class TrailingObjects
  {
  private:
    CheckedPtr<RenderText> _whitespace;
    SmallList<std::reference_wrapper<RenderBoxModelObject>, 4> _boxes;

  public:
    enum class CollapseFirstSpace : bool
    {
      No,
      Yes
    };

    void SetTrailingWhitespace(RenderText &whitespace) noexcept
    {
      _whitespace = ShareCheckedPtr(&whitespace);
    }
    void Clear() noexcept
    {
      _whitespace = nullptr;
      _boxes.clear(); // Keeps our capacity.
    }

    void AppendBoxIfNeeded(RenderBoxModelObject &box) noexcept
    {
      if (_whitespace)
      {
        _boxes.push_back(box);
      }
    }

    void UpdateWhitespaceCollapsingTransitionsForTrailingBoxes(LineWhitespaceCollapsingState &,
                                                               const LegacyInlineIterator &lBreak,
                                                               CollapseFirstSpace) noexcept;
  };
}
