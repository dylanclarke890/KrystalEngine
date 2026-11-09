/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003-2021 Apple Inc. All right reserved.
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

#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief The width of the space after a word, measured once and only when kerning makes it matter (a word
  /// is then measured with its trailing space, and the space's width taken off).
  struct WordTrailingSpace
  {
  private:
    enum class WordTrailingSpaceState : bool
    {
      Uninitialized,
      Initialized
    };

    const RenderStyle *_style;
    WordTrailingSpaceState _state {WordTrailingSpaceState::Uninitialized};
    Maybe<float> _width;

  public:
    explicit WordTrailingSpace(const RenderStyle &style,
                               bool measuringWithTrailingWhitespaceEnabled = true) noexcept
        : _style(&style)
    {
      if (!measuringWithTrailingWhitespaceEnabled || !_style->GetFontCascade().EnableKerning())
      {
        _state = WordTrailingSpaceState::Initialized;
      }
    }

    KRYS_NODISCARD Maybe<float> Width(HashSet<RefPtr<const Font>> &fallbackFonts) noexcept
    {
      if (_state == WordTrailingSpaceState::Initialized)
      {
        return _width;
      }

      auto &font = _style->GetFontCascade();
      _width = font.Width(RenderBlock::ConstructTextRun(u8" ", *_style), &fallbackFonts) + font.WordSpacing();
      _state = WordTrailingSpaceState::Initialized;
      return _width;
    }
  };
}
