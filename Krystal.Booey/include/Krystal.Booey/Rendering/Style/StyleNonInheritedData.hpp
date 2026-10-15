#pragma once

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/DataRef.hpp"

namespace krys::boo
{
  class StyleBoxData;
  class StyleBackgroundData;
  class StyleSurroundData;
  class StyleMiscNonInheritedData;
  class StyleRareNonInheritedData;
  class StyleNonInheritedData : public RefCounted<StyleNonInheritedData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleNonInheritedData> Create() noexcept;
    KRYS_NODISCARD Ref<StyleNonInheritedData> Copy() const noexcept;

    bool operator==(const StyleNonInheritedData &) const noexcept;

    DataRef<StyleBoxData> BoxData;
    DataRef<StyleBackgroundData> BackgroundData;
    DataRef<StyleSurroundData> SurroundData;
    DataRef<StyleMiscNonInheritedData> MiscData;
    DataRef<StyleRareNonInheritedData> RareData;

  private:
    StyleNonInheritedData() noexcept;
    StyleNonInheritedData(const StyleNonInheritedData &) noexcept;
  };
}
