#include "Krystal.Booey/HTML/FileInputType.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"

namespace krys::boo::html
{
  FormControlState FileInputType::SaveFormControlState() const noexcept
  {
    if (_fileList->IsEmpty())
    {
      return {};
    }

    FormControlState stateVector;
    stateVector.reserve(_fileList->Files().size() * 2 + 1);
    for (auto &file : _fileList->Files())
    {
      stateVector.emplace_back(file->Path());
      stateVector.emplace_back(file->Name());
    }
    stateVector.push_back(_displayString);
    return stateVector;
  }

  void FileInputType::RestoreFormControlState(const FormControlState &state) noexcept
  {
    if (state.empty())
    {
      return;
    }

    // The files' paths and names, then the display string.
    auto size = state.size() - 1;
    if (!state[size].empty())
    {
      _displayString = state[size];
    }
    List<Ref<File>> files;
    files.reserve(size / 2);
    for (size_t i = 0; i + 1 < size; i += 2)
    {
      files.push_back(File::Create(state[i], state[i + 1]));
    }
    SetFiles(FileList::Create(krys::move(files)), WasSetByJavaScript::No);
  }

  bool FileInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto name = Element()->Name();

    // If no filename at all is entered, return successful but empty. Null would be more logical, but Netscape
    // posts an empty file.
    if (_fileList->IsEmpty())
    {
      auto file = File::Create(*Blob::Create(List<uint8> {}, MIMETypeRegistry::DefaultMIMEType()), u8"");
      formData.Append(name, *file);
      return true;
    }

    for (auto &file : _fileList->Files())
    {
      formData.Append(name, *file);
    }
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
    if (_fileList->IsEmpty())
    {
      return {};
    }

    // HTML5 tells us that we're supposed to use this goofy value for file input controls. Historically,
    // browsers revealed the real file path, but that's a privacy problem. Code on the web decided to try to
    // parse the value by looking for backslashes (because that's what Windows file paths use). To be
    // compatible with that code, we make up a fake path for the file.
    return dom::DOMString(u8"C:\\fakepath\\") + dom::DOMString(_fileList->GetFile(0).Name());
  }

  void FileInputType::SetValue(dom::DOMStringView, bool valueChanged, TextFieldEventBehavior,
                               TextControlSetValueSelection) noexcept
  {
    // FIXME: Should we clear the file list, or replace it with a new empty one here? This is observable from
    // JavaScript through custom properties.
    if (!valueChanged)
    {
      return;
    }

    _fileList->Clear();
    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    element->InvalidateStyleForSubtree();
    element->UpdateValidity();
  }

  bool FileInputType::DirAutoUsesValue() const noexcept
  {
    return false;
  }

  void FileInputType::SetFiles(RefPtr<FileList> &&files, WasSetByJavaScript wasSetByJavaScript) noexcept
  {
    if (!files)
    {
      return;
    }

    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());

    auto length = files->Length();

    bool pathsChanged = false;
    if (length != _fileList->Length())
    {
      pathsChanged = true;
    }
    else
    {
      for (uint32 i = 0; i < length; ++i)
      {
        if (files->GetFile(i).Path() != _fileList->GetFile(i).Path())
        {
          pathsChanged = true;
          break;
        }
      }
    }

    _fileList = krys::move(files);

    element->SetFormControlValueMatchesRenderer(true);
    element->UpdateValidity();

    if (auto *renderer = element->Renderer())
    {
      renderer->Repaint();
    }

    if (wasSetByJavaScript == WasSetByJavaScript::Yes)
    {
      return;
    }

    if (pathsChanged)
    {
      // This may destroy this input type; the element is kept alive.
      auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Input), true, false, true);
      (void)dom::EventDispatcher::DispatchToTarget(*event, element.get());
      element->DispatchChangeEvent();
    }
    else
    {
      element->DispatchCancelEvent();
    }

    element->SetChangedSinceLastFormControlChangeEvent(false);
    element->SetInteractedWithSinceLastFormSubmitEvent(true);
  }
}
