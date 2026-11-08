#include "Krystal.Booey/HTML/InputTypes/FileInputType.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/Locale/LocalizedStrings.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Booey/Platform/Pasteboard/DragData.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderFileUploadControl.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  namespace
  {
    // A MIME type or a wildcard one (`image/*`): a type and a subtype of token characters.
    KRYS_NODISCARD bool IsAcceptedMIMEType(utf8_stringview type) noexcept
    {
      auto slash = type.find(u8'/');
      if (slash == utf8_stringview::npos || slash == 0 || slash == type.size() - 1)
      {
        return false;
      }
      for (size_t i = 0; i < type.size(); ++i)
      {
        auto c = type[i];
        if (i != slash && !text::IsASCIIAlphanumeric(c)
            && utf8_stringview(u8"!#$%&'*+-.^_`|~").find(c) == utf8_stringview::npos)
        {
          return false;
        }
      }
      return true;
    }

    // https://html.spec.whatwg.org/#attr-input-accept
    // The accept attribute's comma-separated tokens that @p predicate keeps, trimmed and lowercased.
    template <typename TPredicate>
    KRYS_NODISCARD List<utf8_string> ParseAcceptAttribute(utf8_stringview accept,
                                                          TPredicate predicate) noexcept
    {
      List<utf8_string> tokens;
      size_t start = 0;
      while (start <= accept.size())
      {
        auto end = accept.find(u8',', start);
        if (end == utf8_stringview::npos)
        {
          end = accept.size();
        }
        auto token = accept.substr(start, end - start);
        while (!token.empty() && text::IsASCIIWhitespace(token.front()))
        {
          token.remove_prefix(1);
        }
        while (!token.empty() && text::IsASCIIWhitespace(token.back()))
        {
          token.remove_suffix(1);
        }
        if (!token.empty() && predicate(token))
        {
          utf8_string lowered(token);
          for (auto &c : lowered)
          {
            c = text::ToASCIILower(c);
          }
          tokens.push_back(krys::move(lowered));
        }
        start = end + 1;
      }
      return tokens;
    }
  }

  FileInputType::~FileInputType() noexcept
  {
    if (_fileChooser)
    {
      _fileChooser->Invalidate();
    }
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

  RenderPtr<RenderElement> FileInputType::CreateInputRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(Element());
    return CreateRenderer<RenderFileUploadControl>(*Element(), krys::move(style));
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

  void FileInputType::CreateShadowSubtree() noexcept
  {
    krys_debug_assert(NeedsShadowSubtree());
    krys_debug_assert(Element());
    krys_debug_assert(Element()->UserAgentShadowRoot());

    Ref<HTMLInputElement> element = ShareRef(*Element());
    auto button =
      dom::ElementFactory::CreateHTMLElement<HTMLInputElement>(element->NodeDocument(), u8"input");
    (void)button->SetAttribute(dom::DOMStringAtom(u8"type"), dom::DOMString(InputTypeNames::Button));
    button->SetUserAgentPart(css::UserAgentParts::FileSelectorButton());
    (void)button->SetValue(element->Multiple() ? FileButtonChooseMultipleFilesLabel()
                                               : FileButtonChooseFileLabel());
    (void)element->UserAgentShadowRoot()->AppendChild(*button);
    DisabledStateChanged();
  }

  namespace
  {
    KRYS_NODISCARD RefPtr<HTMLInputElement> FileSelectorButton(const dom::Element &element) noexcept
    {
      auto *root = element.UserAgentShadowRoot();
      return root ? ShareRefPtr(DynamicDowncast<HTMLInputElement>(root->FirstChild())) : nullptr;
    }
  }

  void FileInputType::DisabledStateChanged() noexcept
  {
    krys_debug_assert(Element());

    if (auto button = FileSelectorButton(*Element()))
    {
      (void)button->ToggleAttribute(dom::DOMStringAtom(u8"disabled"), Element()->IsDisabledFormControl());
    }
  }

  void FileInputType::AttributeChanged(dom::DOMStringAtom name) noexcept
  {
    if (name == u8"multiple")
    {
      if (auto *element = Element())
      {
        if (auto button = FileSelectorButton(*element))
        {
          (void)button->SetValue(element->Multiple() ? FileButtonChooseMultipleFilesLabel()
                                                     : FileButtonChooseFileLabel());
        }
      }
    }
    BaseClickableWithKeyInputType::AttributeChanged(name);
  }

  dom::DOMString FileInputType::DisplayString() const noexcept
  {
    return _displayString;
  }

  bool FileInputType::DirAutoUsesValue() const noexcept
  {
    return false;
  }

  void FileInputType::FilesChosen(const List<utf8_string> &paths) noexcept
  {
    krys_debug_assert(Element());
    krys_debug_assert(!paths.empty());

    size_t size = Element()->HasAttribute(u8"multiple") ? paths.size() : 1;
    List<Ref<File>> files;
    files.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
      files.push_back(File::Create(paths[i]));
    }
    SetFiles(FileList::Create(krys::move(files)), WasSetByJavaScript::No);
  }

  void FileInputType::FileChoosingCancelled() noexcept
  {
    // https://html.spec.whatwg.org/#show-the-picker,-if-applicable
    // If the user dismissed the prompt without changing their selection, then queue an element task on the
    // user interaction task source given element to fire an event named cancel at element, with the bubbles
    // attribute initialized to true.
    // (Fired as the host answers.)
    if (auto *element = Element())
    {
      Ref<HTMLInputElement> protectedElement = ShareRef(*element);
      protectedElement->DispatchCancelEvent();
    }
  }

  FileChooserSettings FileInputType::ChooserSettings() const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto accept = element.GetAttribute(u8"accept").value_or(dom::DOMString());
    FileChooserSettings settings {
      .AllowsMultipleFiles = element.Multiple(),
      .AcceptMIMETypes = ParseAcceptAttribute(accept, IsAcceptedMIMEType),
      // An extension: a dot and at least one character.
      .AcceptFileExtensions = ParseAcceptAttribute(accept, [](utf8_stringview token)
                                                   { return token.size() >= 2 && token[0] == u8'.'; }),
    };
    for (auto &file : _fileList->Files())
    {
      settings.SelectedFiles.push_back(utf8_string(file->Path()));
    }
    return settings;
  }

  void FileInputType::ShowPicker() noexcept
  {
    krys_debug_assert(Element());
    auto *frame = Element()->NodeDocument().Frame();
    auto *chromeClient = GetChromeClient();
    if (!frame || !chromeClient)
    {
      return;
    }

    // A new request replaces one the host hasn't answered.
    if (_fileChooser)
    {
      _fileChooser->Invalidate();
    }
    Ref<FileChooser> chooser = FileChooser::Create(*this, ChooserSettings());
    _fileChooser = chooser;
    chromeClient->RunOpenPanel(*frame, *chooser);
  }

  void FileInputType::HandleActivationBehavior(dom::Event &event) noexcept
  {
    krys_debug_assert(Element());
    // Only for a user's click or key.
    if (Element()->IsDisabledFormControl() || !HasTransientActivation())
    {
      return;
    }

    ShowPicker();
    event.SetDefaultHandled();
  }

  // Transcoding dropped images to a type the input accepts (receiveDroppedFilesWithImageTranscoding) is the
  // Mac's.
  bool FileInputType::ReceiveDroppedFiles(const DragData &dragData) noexcept
  {
    auto paths = dragData.AsFilenames();
    if (paths.empty())
    {
      return false;
    }

    FilesChosen(paths);
    return true;
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
