#include "Krystal.Booey/DOM/Events/TextEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD UIEventInit TextInputInit() noexcept
    {
      UIEventInit init;
      init.Bubbles = true;
      init.Cancelable = true;
      init.Composed = true;
      return init;
    }
  }

  TextEvent::TextEvent() noexcept : UIEvent(DOMStringAtom(u8""))
  {
  }

  TextEvent::TextEvent(DOMStringView data, TextEventInputType inputType) noexcept
      : UIEvent(DOMStringAtom(EventNames::TextInput), TextInputInit()), _inputType(inputType), _data(data)
  {
  }

  TextEvent::TextEvent(DOMStringView data, RefPtr<DocumentFragment> &&pastingFragment,
                       TextEventInputType inputType, bool shouldSmartReplace, bool shouldMatchStyle) noexcept
      : UIEvent(DOMStringAtom(EventNames::TextInput), TextInputInit()), _inputType(inputType), _data(data),
        _pastingFragment(krys::move(pastingFragment)), _shouldSmartReplace(shouldSmartReplace),
        _shouldMatchStyle(shouldMatchStyle)
  {
  }

  TextEvent::~TextEvent() noexcept = default;

  Ref<TextEvent> TextEvent::Create(DOMStringView data, TextEventInputType inputType) noexcept
  {
    auto event = CreateRef<TextEvent>(data, inputType);
    event->_isTrusted = true;
    return event;
  }

  Ref<TextEvent> TextEvent::CreateForPlainTextPaste(DOMStringView data, bool shouldSmartReplace) noexcept
  {
    auto event = CreateRef<TextEvent>(data, nullptr, TextEventInputType::Paste, shouldSmartReplace, false);
    event->_isTrusted = true;
    return event;
  }

  Ref<TextEvent> TextEvent::CreateForFragmentPaste(RefPtr<DocumentFragment> &&data,
                                                   TextEventInputType inputType, bool shouldSmartReplace,
                                                   bool shouldMatchStyle) noexcept
  {
    auto event = CreateRef<TextEvent>(DOMStringView(), krys::move(data), inputType, shouldSmartReplace,
                                      shouldMatchStyle);
    event->_isTrusted = true;
    return event;
  }

  Ref<TextEvent> TextEvent::CreateForDrop(DOMStringView data) noexcept
  {
    return Create(data, TextEventInputType::Drop);
  }

  void TextEvent::InitTextEvent(DOMStringAtom type, bool bubbles, bool cancelable,
                                DOMStringView data) noexcept
  {
    if (_dispatched)
    {
      return;
    }
    InitUIEvent(type, bubbles, cancelable, 0);
    _inputType = TextEventInputType::Keyboard;
    _data = DOMString(data);
    _pastingFragment = nullptr;
    _shouldSmartReplace = false;
    _shouldMatchStyle = false;
  }

  DocumentFragment *TextEvent::PastingFragment() const noexcept
  {
    return _pastingFragment.get();
  }
}
