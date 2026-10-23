#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A search field's cancel button.
  class SearchFieldCancelButtonPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<SearchFieldCancelButtonPart> Create() noexcept
    {
      return AdoptRef(*new SearchFieldCancelButtonPart());
    }

  private:
    SearchFieldCancelButtonPart() noexcept : ControlPart(StyleAppearance::SearchFieldCancelButton)
    {
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformSearchFieldCancelButton(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(SearchFieldCancelButton)
