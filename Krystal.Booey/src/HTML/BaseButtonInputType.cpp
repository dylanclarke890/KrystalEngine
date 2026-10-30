#include "Krystal.Booey/HTML/BaseButtonInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/RenderButton.hpp"

namespace krys::boo::html
{
  bool BaseButtonInputType::AppendFormData(DOMFormData &) const noexcept
  {
    // Buttons except overridden types are never successful.
    return false;
  }

  bool BaseButtonInputType::StoresValueSeparateFromAttribute() noexcept
  {
    return false;
  }

  void BaseButtonInputType::SetValue(dom::DOMStringView sanitizedValue, bool, TextFieldEventBehavior,
                                     TextControlSetValueSelection) noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetAttribute(u8"value", dom::DOMString(sanitizedValue));
  }

  bool BaseButtonInputType::DirAutoUsesValue() const noexcept
  {
    return true;
  }

  RenderPtr<RenderElement> BaseButtonInputType::CreateInputRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(Element());
    return CreateRenderer<RenderButton>(*Element(), krys::move(style));
  }
}
