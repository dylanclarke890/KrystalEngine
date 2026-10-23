#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A text area or list box.
  class TextAreaPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<TextAreaPart> Create(StyleAppearance type) noexcept
    {
      return AdoptRef(*new TextAreaPart(type));
    }

  private:
    explicit TextAreaPart(StyleAppearance type) noexcept : ControlPart(type)
    {
      krys_debug_assert(type == StyleAppearance::Listbox || type == StyleAppearance::TextArea);
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformTextArea(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::TextAreaPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept
  {
    return part.GetType() == krys::boo::StyleAppearance::Listbox
           || part.GetType() == krys::boo::StyleAppearance::TextArea;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
