#include "Krystal.Booey/HTML/FileInputType.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"

namespace krys::boo::html
{
  bool FileInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto name = Element()->Name();

    // If no filename at all is entered, return successful but empty. Null would be more logical, but Netscape
    // posts an empty file.
    // FIXME: An empty file entry (with an empty name and the default MIME type), once entries can be files.
    formData.Append(name, {});
    return true;
  }

  bool FileInputType::ValueMissing(dom::DOMStringView value) const noexcept
  {
    krys_debug_assert(Element());
    return Element()->IsRequired() && value.empty();
  }

  dom::DOMString FileInputType::ValueMissingText() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Multiple() ? ValidationMessageValueMissingForMultipleFileText()
                                 : ValidationMessageValueMissingForFileText();
  }

  bool FileInputType::CanSetStringValue() const noexcept
  {
    return false;
  }

  dom::DOMString FileInputType::FirstElementPathForInputValue() const noexcept
  {
    // FIXME: "C:\\fakepath\\" followed by the first file's name, once there are files.
    return {};
  }

  void FileInputType::SetValue(dom::DOMStringView, bool, TextFieldEventBehavior,
                               TextControlSetValueSelection) noexcept
  {
    // FIXME: Clearing the files (setting the value to the empty string empties the list), once there are
    // files.
  }
}
