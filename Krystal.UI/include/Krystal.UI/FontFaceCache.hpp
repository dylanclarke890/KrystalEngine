#pragma once

#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/Render2D/FontFace2D.hpp"

namespace krys::boo
{
  class Font;
}

namespace krys::ui
{
  /// @brief The Render2D faces Booey's fonts are drawn with: one per FreeType face and synthetic bold and
  /// oblique, shared by every size.
  class FontFaceCache
  {
    KRYS_NON_COPYABLE(FontFaceCache);

  private:
    struct Entry
    {
      const text::FreeTypeFace *Face;
      bool SyntheticBold;
      bool SyntheticOblique;
      UniquePtr<gfx::FontFace2D> Face2D;
    };

    List<Entry> _entries;

  public:
    FontFaceCache() noexcept = default;

    /// @brief The face @p font is drawn with, made on first use; null if the font has no face or it can't be
    /// drawn.
    KRYS_NODISCARD const gfx::FontFace2D *FaceFor(const boo::Font &font) noexcept;

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _entries.size();
    }

    void Clear() noexcept
    {
      _entries.clear();
    }
  };
}
