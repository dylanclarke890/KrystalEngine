#include "Krystal.Booey/HTML/ImageInputType.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include <string>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD dom::DOMString ToDecimalString(int32 value) noexcept
    {
      auto string = std::to_string(value);
      return dom::DOMString(reinterpret_cast<const char8 *>(string.data()), string.size());
    }
  }

  bool ImageInputType::IsFormDataAppendable() const noexcept
  {
    return true;
  }

  Maybe<dom::DOMString> ImageInputType::ResultForDialogSubmit() const noexcept
  {
    return ToDecimalString(_clickLocation.X()) + u8"," + ToDecimalString(_clickLocation.Y());
  }

  bool ImageInputType::DirAutoUsesValue() const noexcept
  {
    return false;
  }

  bool ImageInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (!element.IsActivatedSubmit())
    {
      return false;
    }

    auto name = element.Name();
    if (name.empty())
    {
      formData.Append(u8"x", ToDecimalString(_clickLocation.X()));
      formData.Append(u8"y", ToDecimalString(_clickLocation.Y()));
      return true;
    }

    formData.Append(name + u8".x", ToDecimalString(_clickLocation.X()));
    formData.Append(name + u8".y", ToDecimalString(_clickLocation.Y()));

    return true;
  }

  void ImageInputType::HandleActivationBehavior(dom::Event &event) noexcept
  {
    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    if (element->IsDisabledFormControl() || !element->Form())
    {
      return;
    }

    Ref<HTMLFormElement> protectedForm = ShareRef(*element->Form());

    _clickLocation = IntPoint();
    if (auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event))
    {
      if (!mouseEvent->IsSimulated())
      {
        _clickLocation =
          IntPoint(static_cast<int32>(mouseEvent->OffsetX()), static_cast<int32>(mouseEvent->OffsetY()));
      }
    }

    // Update layout before processing form actions in case the style changes the form or button
    // relationships.
    (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    if (RefPtr<HTMLFormElement> currentForm = ShareRefPtr(element->Form()))
    {
      currentForm->SubmitIfPossible(&event, element.get()); // Event handlers can run.
    }

    event.SetDefaultHandled();
  }

  bool ImageInputType::ShouldRespectHeightAndWidthAttributes() noexcept
  {
    return true;
  }

  bool ImageInputType::CanBeSuccessfulSubmitButton() noexcept
  {
    return true;
  }

  RenderPtr<RenderElement> ImageInputType::CreateInputRenderer(RenderStyle &&style) noexcept
  {
    // FIXME: The image's renderer (RenderImage, with its image loader and alt text), with image buttons.
    return InputType::CreateInputRenderer(krys::move(style));
  }
}
