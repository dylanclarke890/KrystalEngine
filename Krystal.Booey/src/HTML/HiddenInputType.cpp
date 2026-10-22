#include "Krystal.Booey/HTML/HiddenInputType.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  bool HiddenInputType::RendererIsNeeded() noexcept
  {
    return false;
  }

  bool HiddenInputType::StoresValueSeparateFromAttribute() noexcept
  {
    return false;
  }

  void HiddenInputType::SetValue(dom::DOMStringView sanitizedValue, bool, TextFieldEventBehavior,
                                 TextControlSetValueSelection) noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetAttribute(u8"value", dom::DOMString(sanitizedValue));
  }

  bool HiddenInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto name = element.Name();

    if (text::IsASCIICaselessEqual(dom::DOMStringView(name), dom::DOMStringView(u8"_charset_")))
    {
      // FIXME: The entry list's encoding, once there are encodings other than UTF-8.
      formData.Append(name, u8"UTF-8");
      return true;
    }
    InputType::AppendFormData(formData);
    if (auto dirname = element.GetAttribute(u8"dirname"))
    {
      formData.Append(*dirname, element.DirectionForFormData());
    }
    return true;
  }

  bool HiddenInputType::ShouldRespectHeightAndWidthAttributes() noexcept
  {
    return true;
  }

  bool HiddenInputType::DirAutoUsesValue() const noexcept
  {
    return true;
  }
}
