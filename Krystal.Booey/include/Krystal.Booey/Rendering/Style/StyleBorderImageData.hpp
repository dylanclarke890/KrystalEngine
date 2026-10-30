#pragma once

#include "Krystal.Booey/Style/Values/Backgrounds/BorderImage.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleBorderImageData : public RefCounted<StyleBorderImageData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleBorderImageData> Create() noexcept
    {
      return AdoptRef(*new StyleBorderImageData);
    }
    KRYS_NODISCARD Ref<StyleBorderImageData> Copy() const noexcept;

    bool operator==(const StyleBorderImageData &) const noexcept;

    style::BorderImage BorderImage;

  private:
    StyleBorderImageData() noexcept;
    StyleBorderImageData(const StyleBorderImageData &) noexcept;
  };
}
