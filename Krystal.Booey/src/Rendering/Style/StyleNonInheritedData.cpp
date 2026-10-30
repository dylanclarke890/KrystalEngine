#include "Krystal.Booey/Rendering/Style/StyleNonInheritedData.hpp"

#include "Krystal.Booey/Rendering/Style/StyleBackgroundData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMiscNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSurroundData.hpp"

namespace krys::boo
{

  StyleNonInheritedData::StyleNonInheritedData() noexcept
      : BoxData(StyleBoxData::Create()), BackgroundData(StyleBackgroundData::Create()),
        SurroundData(StyleSurroundData::Create()), MiscData(StyleMiscNonInheritedData::Create()),
        RareData(StyleRareNonInheritedData::Create())
  {
  }

  StyleNonInheritedData::StyleNonInheritedData(const StyleNonInheritedData &other) noexcept
      : RefCounted<StyleNonInheritedData>(), BoxData(other.BoxData), BackgroundData(other.BackgroundData),
        SurroundData(other.SurroundData), MiscData(other.MiscData), RareData(other.RareData)
  {
    krys_debug_assert_msg(other == *this, "StyleNonInheritedData should be properly copied.");
  }

  Ref<StyleNonInheritedData> StyleNonInheritedData::Create() noexcept
  {
    return AdoptRef(*new StyleNonInheritedData);
  }

  Ref<StyleNonInheritedData> StyleNonInheritedData::Copy() const noexcept
  {
    return AdoptRef(*new StyleNonInheritedData(*this));
  }

  bool StyleNonInheritedData::operator==(const StyleNonInheritedData &other) const noexcept
  {
    return BoxData == other.BoxData && BackgroundData == other.BackgroundData
           && SurroundData == other.SurroundData && MiscData == other.MiscData && RareData == other.RareData;
  }
}
