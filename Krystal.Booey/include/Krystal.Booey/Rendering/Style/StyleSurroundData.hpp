#pragma once

#include "Krystal.Booey/Rendering/Style/BorderData.hpp"
#include "Krystal.Booey/Style/Values/Box/Margin.hpp"
#include "Krystal.Booey/Style/Values/Box/Padding.hpp"
#include "Krystal.Booey/Style/Values/Position/Inset.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleSurroundData : public RefCounted<StyleSurroundData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleSurroundData> Create() noexcept
    {
      return AdoptRef(*new StyleSurroundData);
    }
    KRYS_NODISCARD Ref<StyleSurroundData> Copy() const noexcept;

    bool operator==(const StyleSurroundData &) const noexcept;

    // Here instead of in BorderData to pack up against the refcount.
    bool HasExplicitlySetBorderBottomLeftRadius : 1;
    bool HasExplicitlySetBorderBottomRightRadius : 1;
    bool HasExplicitlySetBorderTopLeftRadius : 1;
    bool HasExplicitlySetBorderTopRightRadius : 1;

    bool HasExplicitlySetPaddingBottom : 1;
    bool HasExplicitlySetPaddingLeft : 1;
    bool HasExplicitlySetPaddingRight : 1;
    bool HasExplicitlySetPaddingTop : 1;

    style::InsetBox Inset;
    style::MarginBox Margin;
    style::PaddingBox Padding;
    BorderData Border;

  private:
    StyleSurroundData() noexcept;
    StyleSurroundData(const StyleSurroundData &) noexcept;
  };
}
