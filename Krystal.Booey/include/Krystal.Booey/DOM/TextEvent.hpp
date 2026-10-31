#pragma once

#include "Krystal.Booey/DOM/UIEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  enum class MailBlockquoteHandling : bool;
}

namespace krys::boo::dom
{
  class DocumentFragment;

  enum class TextEventInputType : uint8
  {
    Keyboard, // any newline characters in the text are line breaks only, not paragraph separators.
    Autocompletion,
    LineBreak, // any tab characters in the text are backtabs.
    Composition,
    BackTab,
    Paste,
    Drop,
    Dictation,
    RemoveBackground,
    Other,
  };

  /// @brief The text an edit inserts (`textInput`): typed, pasted or dropped, which listeners can cancel
  /// before the editor inserts it.
  /// @note Legacy.
  /// @see https://w3c.github.io/uievents/#legacy-textevent-events
  // FIXME: Dictation's alternatives (dictationAlternatives), with dictation.
  class TextEvent final : public UIEvent
  {
  private:
    TextEventInputType _inputType {TextEventInputType::Keyboard};
    DOMString _data;
    RefPtr<DocumentFragment> _pastingFragment;
    bool _shouldSmartReplace {false};
    bool _shouldMatchStyle {false};
    MailBlockquoteHandling _mailBlockquoteHandling;

  public:
    TextEvent() noexcept;
    TextEvent(DOMStringView data, TextEventInputType inputType) noexcept;
    TextEvent(DOMStringView data, RefPtr<DocumentFragment> &&pastingFragment, TextEventInputType inputType,
              bool shouldSmartReplace, bool shouldMatchStyle,
              MailBlockquoteHandling mailBlockquoteHandling) noexcept;
    ~TextEvent() noexcept override;

    /// @brief A trusted `textInput` event, which bubbles, can be cancelled and crosses shadow boundaries.
    KRYS_NODISCARD static Ref<TextEvent>
      Create(DOMStringView data, TextEventInputType inputType = TextEventInputType::Keyboard) noexcept;
    KRYS_NODISCARD static Ref<TextEvent> CreateForPlainTextPaste(DOMStringView data,
                                                                 bool shouldSmartReplace) noexcept;
    KRYS_NODISCARD static Ref<TextEvent>
      CreateForFragmentPaste(RefPtr<DocumentFragment> &&data, TextEventInputType inputType,
                             bool shouldSmartReplace, bool shouldMatchStyle,
                             MailBlockquoteHandling mailBlockquoteHandling) noexcept;
    KRYS_NODISCARD static Ref<TextEvent> CreateForDrop(DOMStringView data) noexcept;

    /// @see https://w3c.github.io/uievents/#dom-textevent-inittextevent
    void InitTextEvent(DOMStringAtom type, bool bubbles, bool cancelable, DOMStringView data) noexcept;

    KRYS_NODISCARD bool IsTextEvent() const noexcept final
    {
      return true;
    }

    /// @see https://w3c.github.io/uievents/#dom-textevent-data
    KRYS_NODISCARD const DOMString &Data() const noexcept
    {
      return _data;
    }
    KRYS_NODISCARD bool IsLineBreak() const noexcept
    {
      return _inputType == TextEventInputType::LineBreak;
    }
    KRYS_NODISCARD bool IsComposition() const noexcept
    {
      return _inputType == TextEventInputType::Composition;
    }
    KRYS_NODISCARD bool IsBackTab() const noexcept
    {
      return _inputType == TextEventInputType::BackTab;
    }
    KRYS_NODISCARD bool IsPaste() const noexcept
    {
      return _inputType == TextEventInputType::Paste;
    }
    KRYS_NODISCARD bool IsDrop() const noexcept
    {
      return _inputType == TextEventInputType::Drop;
    }
    KRYS_NODISCARD bool IsDictation() const noexcept
    {
      return _inputType == TextEventInputType::Dictation;
    }
    KRYS_NODISCARD bool IsAutocompletion() const noexcept
    {
      return _inputType == TextEventInputType::Autocompletion;
    }
    KRYS_NODISCARD bool IsKeyboard() const noexcept
    {
      return _inputType == TextEventInputType::Keyboard;
    }
    KRYS_NODISCARD bool IsRemoveBackground() const noexcept
    {
      return _inputType == TextEventInputType::RemoveBackground;
    }

    KRYS_NODISCARD bool ShouldSmartReplace() const noexcept
    {
      return _shouldSmartReplace;
    }
    KRYS_NODISCARD bool ShouldMatchStyle() const noexcept
    {
      return _shouldMatchStyle;
    }
    KRYS_NODISCARD MailBlockquoteHandling GetMailBlockquoteHandling() const noexcept
    {
      return _mailBlockquoteHandling;
    }
    KRYS_NODISCARD DocumentFragment *PastingFragment() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::TextEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsTextEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
