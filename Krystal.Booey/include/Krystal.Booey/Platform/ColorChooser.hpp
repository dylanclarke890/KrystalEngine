#pragma once

#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo
{
  class Color;

  /// @brief A picker the host shows for a colour input.
  class ColorChooser : public RefCounted<ColorChooser>
  {
  public:
    virtual ~ColorChooser() noexcept = default;

    /// @brief Shows the chooser again, with @p color chosen.
    virtual void ReattachColorChooser(const Color &color) noexcept
    {
      (void)color;
    }

    virtual void SetSelectedColor(const Color &color) noexcept
    {
      (void)color;
    }

    /// @brief Closes the chooser; it tells its client it ended.
    virtual void EndChooser() noexcept
    {
    }
  };
}
