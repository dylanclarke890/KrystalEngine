#include "Krystal.Booey/HTML/ImageInputType.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <limits>
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

    /// @brief The width (or height, if @p height) of the dimensions of the image button @p element.
    /// @see https://html.spec.whatwg.org/#determine-dimensions
    KRYS_NODISCARD uint32 ImageButtonDimension(HTMLInputElement &element, bool height) noexcept
    {
      if (element.IsConnected())
      {
        (void)element.NodeDocument().UpdateLayoutIgnorePendingStylesheets(
          {dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
           dom::LayoutOptions::TreatContentVisibilityAutoAsVisible},
          &element);
      }

      // 1. If element is being rendered, then return the width and height of element's content box, in CSS
      //    pixels.
      if (auto *box = DynamicDowncast<RenderBox>(element.Renderer()))
      {
        auto contentRect = box->ContentBoxRect();
        return static_cast<uint32>(
          AdjustLayoutUnitForAbsoluteZoom(height ? contentRect.Height() : contentRect.Width(), box->Style())
            .Round());
      }

      // 2. Let width and height be 0.
      // 3. If naturalDimensions is not null, then set width to naturalDimensions's width and height to
      //    naturalDimensions's height.
      // FIXME: The image's natural dimensions, once image buttons load their image.
      // 4. If element has a width attribute:
      // 4a. Let attributeWidth be the result of parsing that attribute's value.
      // 4b. If attributeWidth is not an error, then set width to attributeWidth.
      // 5. If element has a height attribute:
      // 5a. Let attributeHeight be the result of parsing that attribute's value.
      // 5b. If attributeHeight is not an error, then set height to attributeHeight.
      auto attribute = element.GetAttribute(height ? u8"height" : u8"width");
      if (auto value = Numbers::ParseNonNegativeInteger(attribute.value_or(dom::DOMString()));
          value.Success())
      {
        return static_cast<uint32>(std::min<uint64>(value.Value, std::numeric_limits<int32>::max()));
      }

      // 6. Return a width of width and a height of height, in CSS pixels.
      return 0;
    }
  }

  uint32 ImageInputType::Height() const noexcept
  {
    krys_debug_assert(Element());
    return ImageButtonDimension(*Element(), true);
  }

  uint32 ImageInputType::Width() const noexcept
  {
    krys_debug_assert(Element());
    return ImageButtonDimension(*Element(), false);
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
    // FIXME: The image's renderer (RenderImage, with its image loader and alt text).
    return InputType::CreateInputRenderer(krys::move(style));
  }
}
