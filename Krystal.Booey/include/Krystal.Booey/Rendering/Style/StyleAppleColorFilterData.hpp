#pragma once

#include "Krystal.Booey/Style/Values/FilterEffects/AppleColorFilter.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleAppleColorFilterData : public RefCounted<StyleAppleColorFilterData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleAppleColorFilterData> Create() noexcept
    {
      return AdoptRef(*new StyleAppleColorFilterData);
    }
    KRYS_NODISCARD Ref<StyleAppleColorFilterData> Copy() const noexcept;

    bool operator==(const StyleAppleColorFilterData &) const noexcept;

    style::AppleColorFilter AppleColorFilter;

  private:
    StyleAppleColorFilterData() noexcept;
    StyleAppleColorFilterData(const StyleAppleColorFilterData &) noexcept;
  };
}
