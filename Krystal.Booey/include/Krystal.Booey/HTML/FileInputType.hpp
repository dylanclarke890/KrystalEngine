#pragma once

#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/HTML/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The file input type.
  // FIXME: The file chooser the host shows (showPicker, its settings, the files chosen in it, and its
  // icons) and the tooltip naming the files come with the host's file picker; directories
  // (allowsDirectories), with directory uploads.
  class FileInputType final : public BaseClickableWithKeyInputType
  {
  private:
    Ref<FileList> _fileList;
    dom::DOMString _displayString;

  public:
    explicit FileInputType(HTMLInputElement &element) noexcept
        : BaseClickableWithKeyInputType(Type::File, element), _fileList(FileList::Create())
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::File;
    }

    /// @brief The files' paths and names, and the display string.
    KRYS_NODISCARD FormControlState SaveFormControlState() const noexcept final;
    void RestoreFormControlState(const FormControlState &state) noexcept final;
    bool AppendFormData(DOMFormData &formData) const noexcept final;
    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD dom::DOMString ValueMissingText() const noexcept final;
    KRYS_NODISCARD bool CanSetStringValue() const noexcept final;

    /// @brief The value: C:\\fakepath\\ and the first file's name, or empty without files.
    KRYS_NODISCARD dom::DOMString FirstElementPathForInputValue() const noexcept;

    KRYS_NODISCARD FileList &Files() noexcept
    {
      return *_fileList;
    }
    /// @brief Replaces the files with @p files (if not null); unless set by script, fires `input` and
    /// `change` if their paths changed, or else `cancel`.
    void SetFiles(RefPtr<FileList> &&files, WasSetByJavaScript wasSetByJavaScript) noexcept;

    /// @brief Takes the files at @p paths as the user's choice: all of them if the input is `multiple`, else
    /// the first.
    void FilesChosen(const List<utf8_string> &paths) noexcept;

    bool ReceiveDroppedFiles(const DragData &dragData) noexcept final;
    KRYS_NODISCARD dom::DOMString DisplayString() const noexcept final;

    KRYS_NODISCARD bool DirAutoUsesValue() const noexcept final;

    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateInputRenderer(RenderStyle &&style) noexcept final;
    /// @brief The shadow tree: the button that opens the chooser (`::file-selector-button`).
    void CreateShadowSubtree() noexcept final;
    void DisabledStateChanged() noexcept final;
    void AttributeChanged(dom::DOMStringAtom name) noexcept final;
  };
}
