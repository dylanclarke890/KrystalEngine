#pragma once

#include "Krystal.Booey/HTML/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The file input type.
  // FIXME: Files (FileList, File and Blob), the file chooser the host shows, drag and drop, and the button
  // and file name it draws come with the host's file picker.
  class FileInputType final : public BaseClickableWithKeyInputType
  {
  public:
    explicit FileInputType(HTMLInputElement &element) noexcept
        : BaseClickableWithKeyInputType(Type::File, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::File;
    }

    bool AppendFormData(DOMFormData &formData) const noexcept final;
    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD dom::DOMString ValueMissingText() const noexcept final;
    KRYS_NODISCARD bool CanSetStringValue() const noexcept final;

    /// @brief The value: C:\\fakepath\\ and the first file's name, or empty without files.
    KRYS_NODISCARD dom::DOMString FirstElementPathForInputValue() const noexcept;

    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;
  };
}
