#include "Krystal.Booey/Style/Values/Color/SystemColor.hpp"

namespace krys::boo::style
{
  using css::ValueId;

  namespace
  {
    KRYS_NODISCARD boo::Color DefaultLinkColor(bool useDarkAppearance) noexcept
    {
      return useDarkAppearance ? SRGBA<uint8> {158, 158, 255} : SRGBA<uint8> {0, 0, 238};
    }

    KRYS_NODISCARD boo::Color DefaultVisitedLinkColor(bool useDarkAppearance) noexcept
    {
      return useDarkAppearance ? SRGBA<uint8> {208, 173, 240} : SRGBA<uint8> {85, 26, 139};
    }
  }

  boo::Color SystemColor(ValueId keyword, ColorOptions options) noexcept
  {
    auto useDarkAppearance = HasFlag(options, ColorOptions::UseDarkAppearance);
    auto forVisitedLink = HasFlag(options, ColorOptions::ForVisitedLink);

    switch (keyword)
    {
      // https://drafts.csswg.org/css-color-4/#valdef-system-color-canvas
      // Background of application content or documents.
      case ValueId::Canvas:      return boo::Color::white;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-canvastext
      // Text in application content or documents.
      case ValueId::Canvastext:  return boo::Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-linktext
      // Text in non-active, non-visited links. For light backgrounds, traditionally blue.
      case ValueId::Linktext:    return DefaultLinkColor(useDarkAppearance);

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-visitedtext
      // Text in visited links. For light backgrounds, traditionally purple.
      case ValueId::Visitedtext: return DefaultVisitedLinkColor(useDarkAppearance);

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-activetext
      // Text in active links. For light backgrounds, traditionally red.
      case ValueId::Activetext:
      case ValueId::WebkitActivelink: // Non-standard addition.
        return useDarkAppearance ? SRGBA<uint8> {255, 158, 158} : boo::Color::red;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-buttonface
      // The face background color for push buttons.
      case ValueId::Buttonface:       return boo::Color::lightGray;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-buttontext
      // Text on push buttons.
      case ValueId::Buttontext:       return boo::Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-buttonborder
      // The base border color for push buttons.
      case ValueId::Buttonborder:     return boo::Color::white;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-field
      // Background of input fields.
      case ValueId::Field:            return boo::Color::white;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-fieldtext
      // Text in input fields.
      case ValueId::Fieldtext:        return boo::Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-highlight
      // Background of selected text, for example from ::selection.
      case ValueId::Highlight:        return SRGBA<uint8> {181, 213, 255};

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-highlighttext
      // Text of selected text.
      case ValueId::Highlighttext:    return boo::Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-selecteditem
      // Background of selected items, for example a selected checkbox.
      case ValueId::Selecteditem:     return boo::Color::lightGray;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-selecteditemtext
      // Text of selected items.
      case ValueId::Selecteditemtext: return boo::Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-mark
      // Background of text that has been specially marked (such as by the HTML mark element).
      case ValueId::Mark:             return boo::Color::yellow;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-marktext
      // Text that has been specially marked (such as by the HTML mark element).
      case ValueId::Marktext:         return boo::Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-graytext
      // Disabled text. (Often, but not necessarily, gray.)
      case ValueId::Graytext:         return boo::Color::darkGray;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-accentcolor
      // Background of accented user interface controls.
      case ValueId::Accentcolor:      return SRGBA<uint8> {0, 122, 255};

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-accentcolortext
      // Text of accented user interface controls.
      case ValueId::Accentcolortext:  return boo::Color::black;

      // Non-standard addition.
      case ValueId::Activebuttontext: return boo::Color::black;

      // Non-standard addition.
      case ValueId::Text:             return boo::Color::black;

      // Non-standard addition.
      case ValueId::WebkitLink:
      {
        if (forVisitedLink)
        {
          return DefaultVisitedLinkColor(useDarkAppearance);
        }
        return DefaultLinkColor(useDarkAppearance);
      }

      // Deprecated system-colors:
      // https://drafts.csswg.org/css-color-4/#deprecated-system-colors

      // https://drafts.csswg.org/css-color-4/#activeborder
      // DEPRECATED: Active window border.
      case ValueId::Activeborder:        return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#activecaption
      // DEPRECATED: Active window caption.
      case ValueId::Activecaption:       return SystemColor(ValueId::Canvastext, options);

      // https://drafts.csswg.org/css-color-4/#appworkspace
      // DEPRECATED: Background color of multiple document interface.
      case ValueId::Appworkspace:        return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#background
      // DEPRECATED: Desktop background.
      case ValueId::Background:          return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#buttonhighlight
      // DEPRECATED: The color of the border facing the light source for 3-D elements that
      // appear 3-D due to one layer of surrounding border.
      case ValueId::Buttonhighlight:     return SystemColor(ValueId::Buttonface, options);

      // https://drafts.csswg.org/css-color-4/#buttonshadow
      // DEPRECATED: The color of the border away from the light source for 3-D elements that
      // appear 3-D due to one layer of surrounding border.
      case ValueId::Buttonshadow:        return SystemColor(ValueId::Buttonface, options);

      // https://drafts.csswg.org/css-color-4/#captiontext
      // DEPRECATED: Text in caption, size box, and scrollbar arrow box.
      case ValueId::Captiontext:         return SystemColor(ValueId::Canvastext, options);

      // https://drafts.csswg.org/css-color-4/#inactiveborder
      // DEPRECATED: Inactive window border.
      case ValueId::Inactiveborder:      return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#inactivecaption
      // DEPRECATED: Inactive window caption.
      case ValueId::Inactivecaption:     return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#inactivecaptiontext
      // DEPRECATED: Color of text in an inactive caption.
      case ValueId::Inactivecaptiontext: return SystemColor(ValueId::Graytext, options);

      // https://drafts.csswg.org/css-color-4/#infobackground
      // DEPRECATED: Background color for tooltip controls.
      case ValueId::Infobackground:      return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#infotext
      // DEPRECATED: Text color for tooltip controls.
      case ValueId::Infotext:            return SystemColor(ValueId::Canvastext, options);

      // https://drafts.csswg.org/css-color-4/#menu
      // DEPRECATED: Menu background.
      case ValueId::Menu:                return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#menutext
      // DEPRECATED: Text in menus.
      case ValueId::Menutext:            return SystemColor(ValueId::Canvastext, options);

      // https://drafts.csswg.org/css-color-4/#scrollbar
      // DEPRECATED: Scroll bar gray area.
      case ValueId::Scrollbar:           return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#threeddarkshadow
      // DEPRECATED: The color of the darker (generally outer) of the two borders away from
      // thelight source for 3-D elements that appear 3-D due to two concentric layers of
      // surrounding border.
      case ValueId::Threeddarkshadow:    return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#threedface
      // DEPRECATED: The face background color for 3-D elements that appear 3-D due to two
      // concentric layers of surrounding border
      case ValueId::Threedface:          return SystemColor(ValueId::Buttonface, options);

      // https://drafts.csswg.org/css-color-4/#threedhighlight
      // DEPRECATED: The color of the lighter (generally outer) of the two borders facing
      // the light source for 3-D elements that appear 3-D due to two concentric layers of
      // surrounding border.
      case ValueId::Threedhighlight:     return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#threedlightshadow
      // DEPRECATED: The color of the darker (generally inner) of the two borders facing
      // the light source for 3-D elements that appear 3-D due to two concentric layers of
      // surrounding border
      case ValueId::Threedlightshadow:   return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#threedshadow
      // DEPRECATED: The color of the lighter (generally inner) of the two borders away
      // from the light source for 3-D elements that appear 3-D due to two concentric layers
      // of surrounding border.
      case ValueId::Threedshadow:        return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#window
      // DEPRECATED: Window background.
      case ValueId::Window:              return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#windowframe
      // DEPRECATED: Window frame.
      case ValueId::Windowframe:         return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#windowtext
      // DEPRECATED: Text in windows.
      case ValueId::Windowtext:          return SystemColor(ValueId::Canvastext, options);

      default:                           return {};
    }
  }

  boo::Color FocusRingColor(ColorOptions) noexcept
  {
    return boo::Color::black;
  }
}
