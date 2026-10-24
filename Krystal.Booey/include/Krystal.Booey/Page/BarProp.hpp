#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief One of a window's bars (its location bar, menu bar, personal bar, scroll bars, status bar and
  /// toolbar), as `window.locationbar` and the others report it.
  /// @see https://html.spec.whatwg.org/#barprop
  class BarProp final : public RefCounted<BarProp>
  {
    KRYS_NON_COPY_MOVABLE(BarProp);

  public:
    BarProp() noexcept = default;

    KRYS_NODISCARD static Ref<BarProp> Create() noexcept
    {
      return CreateRef<BarProp>();
    }

    /// @brief Whether the bar is shown: always, as there are no popups.
    /// @see https://html.spec.whatwg.org/#dom-barprop-visible
    KRYS_NODISCARD bool Visible() const noexcept
    {
      // Return true if this's relevant global object's browsing context is null, or if its top-level browsing
      // context's is popup is false (no browsing context is a popup); otherwise false.
      return true;
    }
  };
}
