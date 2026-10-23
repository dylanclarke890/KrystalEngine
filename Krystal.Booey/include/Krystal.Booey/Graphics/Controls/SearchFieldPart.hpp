#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A search field.
  class SearchFieldPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<SearchFieldPart> Create() noexcept
    {
      return AdoptRef(*new SearchFieldPart());
    }

  private:
    SearchFieldPart() noexcept : ControlPart(StyleAppearance::SearchField)
    {
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformSearchField(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(SearchField)
