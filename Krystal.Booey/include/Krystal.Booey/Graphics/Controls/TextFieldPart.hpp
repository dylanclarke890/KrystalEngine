#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A single-line text field.
  class TextFieldPart final : public ControlPart
  {
  public:
    KRYS_NODISCARD static Ref<TextFieldPart> Create() noexcept
    {
      return AdoptRef(*new TextFieldPart());
    }

  private:
    TextFieldPart() noexcept : ControlPart(StyleAppearance::TextField)
    {
    }

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept final
    {
      return GetControlFactory().CreatePlatformTextField(*this);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(TextField)
