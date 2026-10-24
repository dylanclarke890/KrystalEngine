#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LocalDOMWindow;
  class LocalFrame;

  /// @brief The screen the window's page is shown on, as the host describes it in the frame's media
  /// environment.
  /// @see https://drafts.csswg.org/cssom-view/#the-screen-interface
  class Screen final : public RefCounted<Screen>
  {
    KRYS_NON_COPY_MOVABLE(Screen);

  private:
    WeakPtr<LocalDOMWindow> _window;

  public:
    explicit Screen(LocalDOMWindow &window) noexcept;

    KRYS_NODISCARD static Ref<Screen> Create(LocalDOMWindow &window) noexcept
    {
      return CreateRef<Screen>(window);
    }

    /// @see https://drafts.csswg.org/cssom-view/#dom-screen-availwidth
    KRYS_NODISCARD int32 AvailWidth() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-screen-availheight
    KRYS_NODISCARD int32 AvailHeight() const noexcept;

    /// @brief The left of the screen's area that windows can use (outside the task bar).
    /// @note Not in the spec; browsers have it.
    KRYS_NODISCARD int32 AvailLeft() const noexcept;

    /// @brief The top of the screen's area that windows can use.
    /// @note Not in the spec; browsers have it.
    KRYS_NODISCARD int32 AvailTop() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-screen-width
    KRYS_NODISCARD int32 Width() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-screen-height
    KRYS_NODISCARD int32 Height() const noexcept;

    /// @brief The bits of a pixel's colour: 24 for 8 bits a component.
    /// @see https://drafts.csswg.org/cssom-view/#dom-screen-colordepth
    KRYS_NODISCARD uint32 ColorDepth() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-screen-pixeldepth
    KRYS_NODISCARD uint32 PixelDepth() const noexcept
    {
      return ColorDepth();
    }

  private:
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;
  };
}
