#pragma once

#include "Krystal.Booey/Style/Values/Masking/MaskBorder.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleMaskBorderData : public RefCounted<StyleMaskBorderData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleMaskBorderData> Create() noexcept
    {
      return AdoptRef(*new StyleMaskBorderData);
    }
    KRYS_NODISCARD Ref<StyleMaskBorderData> Copy() const noexcept;

    bool operator==(const StyleMaskBorderData &) const noexcept;

    style::MaskBorder MaskBorder;

  private:
    StyleMaskBorderData() noexcept;
    StyleMaskBorderData(const StyleMaskBorderData &) noexcept;
  };
}
