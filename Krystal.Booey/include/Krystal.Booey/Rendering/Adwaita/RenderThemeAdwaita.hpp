/*
 * Copyright (C) 2020 Igalia S.L.
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

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
    KRYS_NODISCARD LayoutUnit SliderTickSnappingThreshold() const noexcept final;
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
