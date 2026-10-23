#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A search field's results button or decoration.
  class SearchFieldResultsPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<SearchFieldResultsPart> Create(StyleAppearance type) noexcept
    {
      return AdoptRef(*new SearchFieldResultsPart(type));
    }

  private:
    explicit SearchFieldResultsPart(StyleAppearance type) noexcept : ControlPart(type)
    {
      krys_debug_assert(type == StyleAppearance::SearchFieldResultsDecoration
                        || type == StyleAppearance::SearchFieldResultsButton);
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformSearchFieldResults(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SearchFieldResultsPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::SearchFieldResultsDecoration
           || part.GetType() == krys::boo::StyleAppearance::SearchFieldResultsButton;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
