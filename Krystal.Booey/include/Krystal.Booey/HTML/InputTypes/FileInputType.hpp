#pragma once

#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/HTML/InputTypes/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/InputTypeNames.hpp"
#include "Krystal.Booey/Platform/Choosers/FileChooser.hpp"

namespace krys::boo::html
{
  /// @brief The file input type. A click (with the user's activation) or `showPicker()` asks the host to show
  /// its file picker (`ChromeClient::RunOpenPanel`).
  class FileInputType final : public BaseClickableWithKeyInputType, private FileChooserClient
  {
  private:
    Ref<FileList> _fileList;
    dom::DOMString _displayString;
    RefPtr<FileChooser> _fileChooser;

  public:
    explicit FileInputType(HTMLInputElement &element) noexcept
        : BaseClickableWithKeyInputType(Type::File, element), _fileList(FileList::Create())
    {
    }

    ~FileInputType() noexcept final;

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

    KRYS_NODISCARD FileList &Files() noexcept
    {
      return *_fileList;
    }
    /// @brief Replaces the files with @p files (if not null); unless set by script, fires `input` and
    /// `change` if their paths changed, or else `cancel`.
    void SetFiles(RefPtr<FileList> &&files, WasSetByJavaScript wasSetByJavaScript) noexcept;

    /// @brief Takes the files at @p paths as the user's choice: all of them if the input is `multiple`, else
    /// the first.
    void FilesChosen(const List<utf8_string> &paths) noexcept final;

    /// @brief Asks the host to show its file picker, with the input's settings.
    /// @see https://html.spec.whatwg.org/#show-the-picker,-if-applicable
    void ShowPicker() noexcept final;

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
    void HandleActivationBehavior(dom::Event &event) noexcept final;

    void FileChoosingCancelled() noexcept final;
    /// @brief The chooser's settings, from the input's attributes and files.
    KRYS_NODISCARD FileChooserSettings ChooserSettings() const noexcept;
  };
}
