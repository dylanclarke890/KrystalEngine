#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  /// @brief The platform's theme: the user's preferences that affect how controls are drawn.
  class Theme
  {
    KRYS_NON_COPY_MOVABLE(Theme);

  public:
    KRYS_NODISCARD static Theme &Singleton() noexcept;

    KRYS_NODISCARD virtual bool UserPrefersContrast() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool UserPrefersReducedMotion() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool UserPrefersOnOffLabels() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool UserPrefersDifferentiationWithoutColor() const noexcept
    {
      return false;
    }

  protected:
    Theme() noexcept = default;
    virtual ~Theme() noexcept = default;
  };
}
