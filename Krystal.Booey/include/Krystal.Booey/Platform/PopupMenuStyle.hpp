#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
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
