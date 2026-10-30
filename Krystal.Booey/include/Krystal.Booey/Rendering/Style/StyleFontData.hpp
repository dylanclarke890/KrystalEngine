#pragma once

#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Style/Values/Text/LetterSpacing.hpp"
#include "Krystal.Booey/Style/Values/Text/WordSpacing.hpp"
#include "Krystal.Core/Types/DataRef.hpp"

namespace krys::boo
{
  class StyleFontData : public RefCounted<StyleFontData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleFontData> Create() noexcept
    {
      return AdoptRef(*new StyleFontData);
    }
    KRYS_NODISCARD Ref<StyleFontData> Copy() const noexcept;

    bool operator==(const StyleFontData &) const noexcept;

    style::LetterSpacing LetterSpacing;
    style::WordSpacing WordSpacing;
    FontCascade FontCascade;

  private:
    StyleFontData() noexcept;
    StyleFontData(const StyleFontData &) noexcept;
    void operator=(const StyleFontData &) noexcept = delete;
  };
}
