#pragma once

#include "Krystal.Booey/Platform/Scrolling/ScrollbarThemeComposite.hpp"

namespace krys::boo
{
  /// @brief GNOME's Adwaita scrollbars, drawn through `GraphicsContext`: a track with a border along its
  /// inner edge and a rounded thumb, without buttons.
  /// @note The scrollbars are classic by default, as Chrome's on Windows: always shown and taking layout
  /// space. A host can choose overlay scrollbars instead, which take no space, draw over the content, and
  /// fade in while the mouse moves over their area or it scrolls.
  class ScrollbarThemeAdwaita : public ScrollbarThemeComposite
  {
  private:
    bool _usesOverlayScrollbars {false};

  public:
    ScrollbarThemeAdwaita() noexcept = default;
    ~ScrollbarThemeAdwaita() noexcept override = default;

    /// @brief Chooses overlay scrollbars (@p overlay) or classic ones, for every page: the pages showing
    /// change over, laying out again for the space their scrollbars take.
    static void SetUsesOverlayScrollbars(bool overlay) noexcept;

  protected:
    KRYS_NODISCARD bool UsesOverlayScrollbars() const noexcept override;
    KRYS_NODISCARD bool InvalidateOnMouseEnterExit() const noexcept override
    {
      return UsesOverlayScrollbars();
    }
    bool Paint(Scrollbar &, GraphicsContext &, const IntRect &) noexcept override;
    void PaintScrollCorner(ScrollableArea &, GraphicsContext &, const IntRect &) noexcept override;
    KRYS_NODISCARD ScrollbarButtonPressAction HandleMousePressEvent(Scrollbar &, const PlatformMouseEvent &,
                                                                    ScrollbarPart) noexcept override;
    KRYS_NODISCARD int32 ScrollbarThickness(
      ScrollbarWidth, OverlayScrollbarSizeRelevancy =
                        OverlayScrollbarSizeRelevancy::IncludeOverlayScrollbarSize) const noexcept override;
    KRYS_NODISCARD int32 MinimumThumbLength(Scrollbar &) noexcept override;
    KRYS_NODISCARD bool HasButtons(Scrollbar &) noexcept override;
    KRYS_NODISCARD bool HasThumb(Scrollbar &) noexcept override;
    KRYS_NODISCARD IntRect BackButtonRect(Scrollbar &, ScrollbarPart,
                                          bool painting = false) noexcept override;
    KRYS_NODISCARD IntRect ForwardButtonRect(Scrollbar &, ScrollbarPart,
                                             bool painting = false) noexcept override;
    KRYS_NODISCARD IntRect TrackRect(Scrollbar &, bool painting = false) noexcept override;
  };
}
