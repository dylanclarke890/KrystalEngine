#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Text/FreeTypeFace.hpp"

namespace krys::boo
{
  namespace css
  {
    enum class FontTechnology : uint8;
  }

  class FontCreationContext;
  class FontDescription;
  class FontPlatformData;
  class SharedBuffer;

  /// @brief A web font loaded for the font backend: a FreeType face made from the font file's bytes.
  // FIXME: The faces of a font collection (itemInCollection), and font-face features and variations, which
  // the font backend doesn't apply yet.
  struct FontCustomPlatformData : public RefCounted<FontCustomPlatformData>
  {
    KRYS_NON_COPY_MOVABLE(FontCustomPlatformData);

    Ref<text::FreeTypeFace> Face;

    explicit FontCustomPlatformData(Ref<text::FreeTypeFace> &&face) noexcept : Face(krys::move(face))
    {
    }

    /// @brief The font in @p buffer (a TrueType or OpenType font; WOFF fonts are converted before), or null
    /// if it isn't one.
    KRYS_NODISCARD static RefPtr<FontCustomPlatformData>
      Create(SharedBuffer &buffer, const css::CSSOMString &itemInCollection) noexcept;

    /// @brief The platform font for @p description, made bold or italic synthetically if asked.
    KRYS_NODISCARD boo::FontPlatformData
      GetFontPlatformData(const FontDescription &description, bool bold, bool italic,
                          const FontCreationContext &fontCreationContext) const noexcept;

    /// @brief Whether fonts in @p format (a `format()` keyword or string, e.g. `woff2`) can be loaded.
    KRYS_NODISCARD static bool SupportsFormat(css::CSSOMStringView format) noexcept;

    /// @brief Whether the font technology @p technology is supported.
    KRYS_NODISCARD static bool SupportsTechnology(css::FontTechnology technology) noexcept;
  };
}
