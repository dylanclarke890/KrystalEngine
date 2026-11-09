/*
 * Copyright (C) 2008-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief How a popup menu, or one of its items, looks.
  class PopupMenuStyle
  {
  public:
    enum class PopupMenuType : uint8
    {
      SelectPopup,
      AutofillPopup
    };

    enum class BackgroundColorType : uint8
    {
      DefaultBackgroundColor,
      CustomBackgroundColor
    };

    enum class Size : uint8
    {
      Normal,
      Small,
      Mini,
      Large,
    };

  private:
    Color _foregroundColor;
    Color _backgroundColor;
    FontCascade _font;
    utf8_string _language;
    bool _visible;
    bool _isDisplayNone;
    bool _hasDefaultAppearance;
    TextDirection _textDirection;
    bool _hasTextDirectionOverride;
    BackgroundColorType _backgroundColorType;
    PopupMenuType _menuType;
    Size _menuSize;

  public:
    PopupMenuStyle(const Color &foreground, const Color &background, const FontCascade &font,
                   utf8_string language, bool visible, bool isDisplayNone, bool hasDefaultAppearance,
                   TextDirection textDirection, bool hasTextDirectionOverride,
                   BackgroundColorType backgroundColorType = BackgroundColorType::DefaultBackgroundColor,
                   PopupMenuType menuType = PopupMenuType::SelectPopup, Size menuSize = Size::Normal) noexcept
        : _foregroundColor(foreground), _backgroundColor(background), _font(font),
          _language(krys::move(language)), _visible(visible), _isDisplayNone(isDisplayNone),
          _hasDefaultAppearance(hasDefaultAppearance), _textDirection(textDirection),
          _hasTextDirectionOverride(hasTextDirectionOverride), _backgroundColorType(backgroundColorType),
          _menuType(menuType), _menuSize(menuSize)
    {
    }

    KRYS_NODISCARD const Color &ForegroundColor() const noexcept
    {
      return _foregroundColor;
    }

    KRYS_NODISCARD const Color &BackgroundColor() const noexcept
    {
      return _backgroundColor;
    }

    KRYS_NODISCARD const FontCascade &Font() const noexcept
    {
      return _font;
    }

    KRYS_NODISCARD const utf8_string &Language() const noexcept
    {
      return _language;
    }

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return _visible;
    }

    KRYS_NODISCARD bool IsDisplayNone() const noexcept
    {
      return _isDisplayNone;
    }

    KRYS_NODISCARD bool HasDefaultAppearance() const noexcept
    {
      return _hasDefaultAppearance;
    }

    KRYS_NODISCARD TextDirection GetTextDirection() const noexcept
    {
      return _textDirection;
    }

    KRYS_NODISCARD bool HasTextDirectionOverride() const noexcept
    {
      return _hasTextDirectionOverride;
    }

    KRYS_NODISCARD BackgroundColorType GetBackgroundColorType() const noexcept
    {
      return _backgroundColorType;
    }

    KRYS_NODISCARD PopupMenuType MenuType() const noexcept
    {
      return _menuType;
    }

    KRYS_NODISCARD Size MenuSize() const noexcept
    {
      return _menuSize;
    }
  };
}
