#pragma once

#include "Krystal.Booey/Rendering/RenderTheme.hpp"

namespace krys::boo
{
  /// @brief GNOME's Adwaita theme: controls drawn with the graphics context in Adwaita's style.
  class RenderThemeAdwaita : public RenderTheme
  {
  public:
    ~RenderThemeAdwaita() noexcept override;

    KRYS_NODISCARD bool CanCreateControlPartForRenderer(const RenderElement &) const noexcept final;
    KRYS_NODISCARD bool CanCreateControlPartForBorderOnly(const RenderElement &) const noexcept final;
    KRYS_NODISCARD bool CanCreateControlPartForDecorations(const RenderElement &) const noexcept final;

    KRYS_NODISCARD bool IsControlStyled(const RenderStyle &) const noexcept final;

    void SetAccentColor(const Color &) noexcept;

  private:
    KRYS_NODISCARD utf8_stringview ExtraDefaultStyleSheet() noexcept final;

    KRYS_NODISCARD bool SupportsHover() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool SupportsFocusRing(const RenderElement &, const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool SupportsSelectionForegroundColors(style::ColorOptions) const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD bool SupportsListBoxSelectionForegroundColors(style::ColorOptions) const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool ShouldHaveCapsLockIndicator(const html::HTMLInputElement &) const noexcept final;

    KRYS_NODISCARD Color PlatformActiveSelectionBackgroundColor(style::ColorOptions) const noexcept final;
    KRYS_NODISCARD Color PlatformInactiveSelectionBackgroundColor(style::ColorOptions) const noexcept final;
    KRYS_NODISCARD Color PlatformActiveSelectionForegroundColor(style::ColorOptions) const noexcept final;
    KRYS_NODISCARD Color PlatformInactiveSelectionForegroundColor(style::ColorOptions) const noexcept final;

    KRYS_NODISCARD
    Color PlatformActiveListBoxSelectionBackgroundColor(style::ColorOptions) const noexcept final;
    KRYS_NODISCARD
    Color PlatformActiveListBoxSelectionForegroundColor(style::ColorOptions) const noexcept final;
    KRYS_NODISCARD
    Color PlatformInactiveListBoxSelectionBackgroundColor(style::ColorOptions) const noexcept final;
    KRYS_NODISCARD
    Color PlatformInactiveListBoxSelectionForegroundColor(style::ColorOptions) const noexcept final;

    KRYS_NODISCARD Color PlatformFocusRingColor(style::ColorOptions) const noexcept final;
    void PlatformColorsDidChange() noexcept final;

    KRYS_NODISCARD Color SystemColor(css::ValueId, style::ColorOptions) const noexcept final;

    void AdjustTextFieldStyle(RenderStyle &, const dom::Element *) const noexcept final;
    void AdjustTextAreaStyle(RenderStyle &, const dom::Element *) const noexcept final;
    void AdjustSearchFieldStyle(RenderStyle &, const dom::Element *) const noexcept final;

    KRYS_NODISCARD bool PopsMenuBySpaceOrReturn() const noexcept final
    {
      return true;
    }

    void AdjustMenuListStyle(RenderStyle &, const dom::Element *) const noexcept override;
    void AdjustMenuListButtonStyle(RenderStyle &, const dom::Element *) const noexcept override;

    KRYS_NODISCARD style::PaddingBox PopupInternalPaddingBox(const RenderStyle &) const noexcept final;

    KRYS_NODISCARD MonotonicTime::duration
      AnimationRepeatIntervalForProgressBar(const RenderProgress &) const noexcept final;
    KRYS_NODISCARD IntRect ProgressBarRectForBounds(const RenderProgress &,
                                                    const IntRect &) const noexcept final;

    void AdjustSliderThumbSize(RenderStyle &, const dom::Element *) const noexcept final;

    KRYS_NODISCARD IntSize SliderTickSize() const noexcept final;
    KRYS_NODISCARD int32 SliderTickOffsetFromTrackCenter() const noexcept final;

    void AdjustListButtonStyle(RenderStyle &, const dom::Element *) const noexcept final;

    KRYS_NODISCARD style::PreferredSizePair ControlSize(StyleAppearance, const FontCascade &,
                                                        const style::PreferredSizePair &,
                                                        float) const noexcept final;
    KRYS_NODISCARD style::MinimumSizePair MinimumControlSize(StyleAppearance, const FontCascade &,
                                                             const style::MinimumSizePair &,
                                                             float) const noexcept final;
    KRYS_NODISCARD style::LineWidthBox ControlBorder(StyleAppearance, const FontCascade &,
                                                     const style::LineWidthBox &, float,
                                                     const dom::Element *) const noexcept final;
  };
}
