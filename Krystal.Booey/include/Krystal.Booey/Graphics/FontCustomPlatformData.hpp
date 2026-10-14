#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  namespace css
  {
    enum class FontTechnology : uint8;
  }

  /// @brief A web font loaded for the platform's font backend.
  // FIXME: Only the support queries exist until there's a font backend.
  struct FontCustomPlatformData
  {
    /// @brief Whether fonts in @p format (a `format()` keyword or string, e.g. `woff2`) can be loaded.
    KRYS_NODISCARD static bool SupportsFormat(css::CSSOMStringView format) noexcept;

    /// @brief Whether the font technology @p technology is supported.
    KRYS_NODISCARD static bool SupportsTechnology(css::FontTechnology technology) noexcept;
  };
}
