#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LocalDOMWindow;

  /// @brief The window's user activation state, as `navigator.userActivation` reports it.
  /// @see https://html.spec.whatwg.org/#the-useractivation-interface
  class UserActivation final : public RefCounted<UserActivation>
  {
    KRYS_NON_COPY_MOVABLE(UserActivation);

  private:
    WeakPtr<LocalDOMWindow> _window;

  public:
    explicit UserActivation(LocalDOMWindow &window) noexcept;

    KRYS_NODISCARD static Ref<UserActivation> Create(LocalDOMWindow &window) noexcept
    {
      return CreateRef<UserActivation>(window);
    }

    /// @brief Whether the window has sticky activation.
    /// @see https://html.spec.whatwg.org/#dom-useractivation-hasbeenactive
    KRYS_NODISCARD bool HasBeenActive() const noexcept;

    /// @brief Whether the window has transient activation.
    /// @see https://html.spec.whatwg.org/#dom-useractivation-isactive
    KRYS_NODISCARD bool IsActive() const noexcept;
  };
}
