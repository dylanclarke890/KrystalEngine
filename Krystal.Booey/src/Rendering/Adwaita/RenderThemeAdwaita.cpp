#include "Krystal.Booey/Rendering/Adwaita/RenderThemeAdwaita.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/Graphics/Adwaita/Adwaita.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Platform/ThemeAdwaita.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"

namespace krys::boo
{
  using namespace adwaita;
  using css::ValueId;

  RenderTheme &RenderTheme::Singleton() noexcept
  {
    static RenderThemeAdwaita theme;
    return theme;
  }

  RenderThemeAdwaita::~RenderThemeAdwaita() noexcept = default;

  bool RenderThemeAdwaita::CanCreateControlPartForRenderer(const RenderElement &renderer) const noexcept
  {
    switch (renderer.Style().UsedAppearance())
    {
      case StyleAppearance::Button:
      case StyleAppearance::Checkbox:
      case StyleAppearance::ColorWell:
      case StyleAppearance::DefaultButton:
      case StyleAppearance::InnerSpinButton:
      case StyleAppearance::Menulist:
      case StyleAppearance::ProgressBar:
      case StyleAppearance::PushButton:
      case StyleAppearance::Radio:
      case StyleAppearance::SearchField:
      case StyleAppearance::SliderThumbHorizontal:
      case StyleAppearance::SliderThumbVertical:
      case StyleAppearance::SliderHorizontal:
      case StyleAppearance::SliderVertical:
      case StyleAppearance::SquareButton:          return true;
      default:                                     break;
    }
    return false;
  }

  bool RenderThemeAdwaita::CanCreateControlPartForBorderOnly(const RenderElement &renderer) const noexcept
  {
    switch (renderer.Style().UsedAppearance())
    {
      case StyleAppearance::Listbox:
      case StyleAppearance::TextArea:
      case StyleAppearance::TextField: return true;
      default:                         break;
    }
    return false;
  }

  bool RenderThemeAdwaita::CanCreateControlPartForDecorations(const RenderElement &renderer) const noexcept
  {
    return renderer.Style().UsedAppearance() == StyleAppearance::MenulistButton;
  }

  bool RenderThemeAdwaita::SupportsFocusRing(const RenderElement &, const RenderStyle &style) const noexcept
  {
    switch (style.UsedAppearance())
    {
      case StyleAppearance::PushButton:
      case StyleAppearance::Button:
      case StyleAppearance::TextField:
      case StyleAppearance::TextArea:
      case StyleAppearance::SearchField:
      case StyleAppearance::Menulist:
      case StyleAppearance::Radio:
      case StyleAppearance::Checkbox:
      case StyleAppearance::SliderHorizontal:
      case StyleAppearance::SliderVertical:   return true;
      default:                                break;
    }
    return false;
  }

  bool RenderThemeAdwaita::ShouldHaveCapsLockIndicator(const html::HTMLInputElement &element) const noexcept
  {
    return element.IsPasswordField();
  }

  Color RenderThemeAdwaita::PlatformActiveSelectionBackgroundColor(style::ColorOptions) const noexcept
  {
    return SystemAccentColor().ColorWithAlphaMultipliedBy(0.3f);
  }

  Color
    RenderThemeAdwaita::PlatformInactiveSelectionBackgroundColor(style::ColorOptions options) const noexcept
  {
    if (HasFlag(options, style::ColorOptions::UseDarkAppearance))
    {
      return SRGBA<uint8> {255, 255, 255, 25};
    }
    return SRGBA<uint8> {0, 0, 0, 25};
  }

  Color RenderThemeAdwaita::PlatformActiveSelectionForegroundColor(style::ColorOptions) const noexcept
  {
    return {};
  }

  Color RenderThemeAdwaita::PlatformInactiveSelectionForegroundColor(style::ColorOptions) const noexcept
  {
    return {};
  }

  Color RenderThemeAdwaita::PlatformActiveListBoxSelectionBackgroundColor(
    style::ColorOptions options) const noexcept
  {
    return PlatformActiveListBoxSelectionForegroundColor(options).ColorWithAlpha(0.15f);
  }

  Color RenderThemeAdwaita::PlatformInactiveListBoxSelectionBackgroundColor(
    style::ColorOptions options) const noexcept
  {
    return PlatformInactiveListBoxSelectionForegroundColor(options).ColorWithAlpha(0.15f);
  }

  Color RenderThemeAdwaita::PlatformActiveListBoxSelectionForegroundColor(
    style::ColorOptions options) const noexcept
  {
    return HasFlag(options, style::ColorOptions::UseDarkAppearance) ? Color::white : Color::black;
  }

  Color RenderThemeAdwaita::PlatformInactiveListBoxSelectionForegroundColor(
    style::ColorOptions options) const noexcept
  {
    return PlatformActiveListBoxSelectionForegroundColor(options);
  }

  Color RenderThemeAdwaita::PlatformFocusRingColor(style::ColorOptions) const noexcept
  {
    return SystemFocusRingColor();
  }

  void RenderThemeAdwaita::PlatformColorsDidChange() noexcept
  {
    static_cast<ThemeAdwaita &>(Theme::Singleton()).PlatformColorsDidChange();
    RenderTheme::PlatformColorsDidChange();
  }

  utf8_stringview RenderThemeAdwaita::ExtraDefaultStyleSheet() noexcept
  {
    return css::ThemeAdwaitaUserAgentStyleSheet;
  }

  Color RenderThemeAdwaita::SystemColor(ValueId cssValueID, style::ColorOptions options) const noexcept
  {
    const bool useDarkAppearance = HasFlag(options, style::ColorOptions::UseDarkAppearance);

    switch (cssValueID)
    {
      case ValueId::Activebuttontext:
      case ValueId::Buttontext:
        if (useDarkAppearance)
        {
          return {ButtonTextColorDark, ColorFlags::Semantic};
        }
        return {ButtonTextColorLight, ColorFlags::Semantic};

      case ValueId::Graytext:
        if (useDarkAppearance)
        {
          return {ButtonTextDisabledColorDark, ColorFlags::Semantic};
        }
        return {ButtonTextDisabledColorLight, ColorFlags::Semantic};

      case ValueId::Canvas:
        if (useDarkAppearance)
        {
          return {SRGBA<uint8> {30, 30, 30}, ColorFlags::Semantic};
        }
        return {Color::white, ColorFlags::Semantic};

      case ValueId::Field:
        if (useDarkAppearance)
        {
          return {TextFieldBackgroundColorDark, ColorFlags::Semantic};
        }
        return {TextFieldBackgroundColorLight, ColorFlags::Semantic};

      case ValueId::Canvastext:
      case ValueId::Fieldtext:
      case ValueId::Text:
        if (useDarkAppearance)
        {
          return {Color::white, ColorFlags::Semantic};
        }
        return {Color::black, ColorFlags::Semantic};

      case ValueId::Highlight:
        // Hardcoded to avoid exposing a user appearance preference to the web for fingerprinting.
        return {SRGBA<uint8> {52, 132, 228}, ColorFlags::Semantic};

      case ValueId::Highlighttext: return {Color::white, ColorFlags::Semantic};

      default:                     return RenderTheme::SystemColor(cssValueID, options);
    }
  }

  bool RenderThemeAdwaita::IsControlStyled(const RenderStyle &style) const noexcept
  {
    auto appearance = style.UsedAppearance();
    if (appearance == StyleAppearance::TextField || appearance == StyleAppearance::TextArea
        || appearance == StyleAppearance::SearchField || appearance == StyleAppearance::Listbox)
    {
      return style.NativeAppearanceDisabled();
    }
    return RenderTheme::IsControlStyled(style);
  }

  void RenderThemeAdwaita::AdjustTextFieldStyle(RenderStyle &style, const dom::Element *) const noexcept
  {
    if (!style.HasExplicitlySetBorderRadius())
    {
      style.SetBorderRadius(style::BorderRadiusValue {5_css_px, 5_css_px});
    }
  }

  void RenderThemeAdwaita::AdjustTextAreaStyle(RenderStyle &style, const dom::Element *element) const noexcept
  {
    AdjustTextFieldStyle(style, element);
  }

  void RenderThemeAdwaita::AdjustSearchFieldStyle(RenderStyle &style,
                                                  const dom::Element *element) const noexcept
  {
    AdjustTextFieldStyle(style, element);
  }

  void RenderThemeAdwaita::AdjustMenuListStyle(RenderStyle &style, const dom::Element *element) const noexcept
  {
    RenderTheme::AdjustMenuListStyle(style, element);
    style.SetLineHeight(style::ComputedStyle::InitialLineHeight());
  }

  void RenderThemeAdwaita::AdjustMenuListButtonStyle(RenderStyle &style,
                                                     const dom::Element *element) const noexcept
  {
    AdjustMenuListStyle(style, element);
  }

  style::PaddingBox RenderThemeAdwaita::PopupInternalPaddingBox(const RenderStyle &style) const noexcept
  {
    if (style.UsedAppearance() == StyleAppearance::None)
    {
      return style::PaddingBox {style::PaddingEdge::Fixed {0}};
    }

    auto zoomedArrowSize = MenuListButtonArrowSize * style.UsedZoom();
    int32 leftPadding =
      static_cast<int32>(MenuListButtonPadding + (style.GetWritingMode().IsBidiRTL() ? zoomedArrowSize : 0));
    int32 rightPadding =
      static_cast<int32>(MenuListButtonPadding + (style.GetWritingMode().IsBidiLTR() ? zoomedArrowSize : 0));

    return {
      style::PaddingEdge::Fixed {static_cast<float>(MenuListButtonPadding)},
      style::PaddingEdge::Fixed {static_cast<float>(rightPadding)},
      style::PaddingEdge::Fixed {static_cast<float>(MenuListButtonPadding)},
      style::PaddingEdge::Fixed {static_cast<float>(leftPadding)},
    };
  }

  MonotonicTime::duration
    RenderThemeAdwaita::AnimationRepeatIntervalForProgressBar(const RenderProgress &) const noexcept
  {
    // FIXME: The page's preferred rendering update interval (Page::preferredRenderingUpdateInterval), with
    // RenderProgress (G14d2).
    return std::chrono::milliseconds(16);
  }

  IntRect RenderThemeAdwaita::ProgressBarRectForBounds(const RenderProgress &,
                                                       const IntRect &bounds) const noexcept
  {
    // FIXME: Vertical progress bars (the renderer's isHorizontalWritingMode), with RenderProgress (G14d2).
    bool isHorizontal = true;
    return {bounds.X(), bounds.Y(), isHorizontal ? bounds.Width() : ProgressBarSize,
            isHorizontal ? ProgressBarSize : bounds.Height()};
  }

  void RenderThemeAdwaita::AdjustSliderThumbSize(RenderStyle &style, const dom::Element *) const noexcept
  {
    auto appearance = style.UsedAppearance();
    if (appearance != StyleAppearance::SliderThumbHorizontal
        && appearance != StyleAppearance::SliderThumbVertical)
    {
      return;
    }

    style.SetWidth(style::PreferredSize::Fixed {static_cast<float>(SliderThumbSize)});
    style.SetHeight(style::PreferredSize::Fixed {static_cast<float>(SliderThumbSize)});
  }

  IntSize RenderThemeAdwaita::SliderTickSize() const noexcept
  {
    return {1, 7};
  }

  int32 RenderThemeAdwaita::SliderTickOffsetFromTrackCenter() const noexcept
  {
    return -16;
  }

  void RenderThemeAdwaita::AdjustListButtonStyle(RenderStyle &style, const dom::Element *) const noexcept
  {
    // Add a margin to place the button at end of the input field.
    if (style.IsLeftToRightDirection())
    {
      style.SetMarginRight(style::MarginEdge::Fixed {-2});
    }
    else
    {
      style.SetMarginLeft(style::MarginEdge::Fixed {-2});
    }
  }

  style::PreferredSizePair RenderThemeAdwaita::ControlSize(StyleAppearance appearance,
                                                           const FontCascade &fontCascade,
                                                           const style::PreferredSizePair &zoomedSize,
                                                           float zoomFactor) const noexcept
  {
    if (!zoomedSize.Width().IsIntrinsicOrLegacyIntrinsicOrAuto()
        && !zoomedSize.Height().IsIntrinsicOrLegacyIntrinsicOrAuto())
    {
      return RenderTheme::ControlSize(appearance, fontCascade, zoomedSize, zoomFactor);
    }

    switch (appearance)
    {
      case StyleAppearance::Checkbox:
      case StyleAppearance::Radio:
      {
        auto buttonSizeWidth = zoomedSize.Width();
        auto buttonSizeHeight = zoomedSize.Height();
        if (buttonSizeWidth.IsIntrinsicOrLegacyIntrinsicOrAuto())
        {
          buttonSizeWidth = style::PreferredSize::Fixed {12 * zoomFactor};
        }
        if (buttonSizeHeight.IsIntrinsicOrLegacyIntrinsicOrAuto())
        {
          buttonSizeHeight = style::PreferredSize::Fixed {12 * zoomFactor};
        }
        return {krys::move(buttonSizeWidth), krys::move(buttonSizeHeight)};
      }
      case StyleAppearance::InnerSpinButton:
      {
        auto spinButtonSizeWidth = zoomedSize.Width();
        auto spinButtonSizeHeight = zoomedSize.Height();
        if (spinButtonSizeWidth.IsIntrinsicOrLegacyIntrinsicOrAuto())
        {
          spinButtonSizeWidth =
            style::PreferredSize::Fixed {static_cast<float>(static_cast<int32>(ArrowSize * zoomFactor))};
        }
        if (spinButtonSizeHeight.IsIntrinsicOrLegacyIntrinsicOrAuto() || fontCascade.Size() > ArrowSize)
        {
          spinButtonSizeHeight = style::PreferredSize::Fixed {fontCascade.Size()};
        }
        return {krys::move(spinButtonSizeWidth), krys::move(spinButtonSizeHeight)};
      }
      default: break;
    }

    return RenderTheme::ControlSize(appearance, fontCascade, zoomedSize, zoomFactor);
  }

  style::MinimumSizePair RenderThemeAdwaita::MinimumControlSize(StyleAppearance, const FontCascade &,
                                                                const style::MinimumSizePair &zoomedSize,
                                                                float) const noexcept
  {
    if (!zoomedSize.Width().IsIntrinsicOrLegacyIntrinsicOrAuto()
        && !zoomedSize.Height().IsIntrinsicOrLegacyIntrinsicOrAuto())
    {
      return zoomedSize;
    }

    auto resultWidth = zoomedSize.Width();
    auto resultHeight = zoomedSize.Height();
    if (resultWidth.IsIntrinsicOrLegacyIntrinsicOrAuto())
    {
      resultWidth = style::MinimumSize::Fixed {0};
    }
    if (resultHeight.IsIntrinsicOrLegacyIntrinsicOrAuto())
    {
      resultHeight = style::MinimumSize::Fixed {0};
    }
    return {krys::move(resultWidth), krys::move(resultHeight)};
  }

  style::LineWidthBox RenderThemeAdwaita::ControlBorder(StyleAppearance appearance, const FontCascade &font,
                                                        const style::LineWidthBox &zoomedBox,
                                                        float zoomFactor,
                                                        const dom::Element *element) const noexcept
  {
    switch (appearance)
    {
      case StyleAppearance::PushButton:
      case StyleAppearance::DefaultButton:
      case StyleAppearance::Button:
      case StyleAppearance::SquareButton:  return zoomedBox;
      default:                             break;
    }

    return RenderTheme::ControlBorder(appearance, font, zoomedBox, zoomFactor, element);
  }

  void RenderThemeAdwaita::SetAccentColor(const Color &color) noexcept
  {
    static_cast<ThemeAdwaita &>(Theme::Singleton()).SetAccentColor(color);
    PlatformColorsDidChange();
  }
}
