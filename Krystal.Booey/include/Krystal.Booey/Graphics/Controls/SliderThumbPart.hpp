#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A slider's thumb.
  class SliderThumbPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<SliderThumbPart> Create(StyleAppearance type) noexcept
    {
      return AdoptRef(*new SliderThumbPart(type));
    }

  private:
    explicit SliderThumbPart(StyleAppearance type) noexcept : ControlPart(type)
    {
      krys_debug_assert(type == StyleAppearance::SliderThumbHorizontal
                        || type == StyleAppearance::SliderThumbVertical);
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformSliderThumb(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SliderThumbPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::SliderThumbHorizontal
           || part.GetType() == krys::boo::StyleAppearance::SliderThumbVertical;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
