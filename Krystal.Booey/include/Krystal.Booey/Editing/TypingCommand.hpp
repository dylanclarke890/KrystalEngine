#pragma once

#include "Krystal.Booey/Editing/TextGranularity.hpp"
#include "Krystal.Booey/Editing/TextInsertionBaseCommand.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::dom
{
  class Event;
}

namespace krys::boo
{
  enum class TypingCommandOption : uint8
  {
    SelectInsertedText,
    AddsToKillRing,
    RetainAutocorrectionIndicator,
    PreventSpellChecking,
    SmartDelete,
    IsAutocompletion,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::TypingCommandOption, 6uz);

namespace krys::boo
{
  /// @brief Typing: text, line breaks, paragraph breaks and deletions, which go into the same command (and so
  /// are undone together) while the selection stays where the last one left it.
  // FIXME: Accessibility's notifications of typed and deleted text (postTextStateChangeNotification and
  // AccessibilityReplacedText), with accessibility; spelling after typing (markMisspellingsAfterTyping), with
  // text checking; and the kill ring (addRangeToKillRing), with the kill ring.
  class TypingCommand final : public TextInsertionBaseCommand
  {
    friend class TypingCommandLineOperation;

  public:
    enum class Type : uint8
    {
      DeleteSelection,
      DeleteKey,
      ForwardDeleteKey,
      InsertText,
      InsertLineBreak,
      InsertParagraphSeparator,
      InsertParagraphSeparatorInQuotedContent
    };

    enum class TextCompositionType : uint8
    {
      None,
      Pending,
      Final,
    };

    using Option = TypingCommandOption;

  private:
    Type _commandType;
    EditAction _currentTypingEditAction;
    utf8_string _textToInsert;
    utf8_string _currentTextToInsert;
    bool _openForMoreTyping {true};
    bool _selectInsertedText;
    bool _smartDelete;
    bool _isHandlingInitialTypingCommand {true};
    TextGranularity _granularity;
    TextCompositionType _compositionType;
    bool _shouldAddToKillRing;
    bool _preservesTypingStyle {false};
    bool _isAutocompletion;

    // Undoing a series of backward deletes will restore a selection around all of the
    // characters that were deleted, but only if the typing command being undone
    // was opened with a backward delete.
    bool _openedByBackwardDelete {false};
    bool _shouldRetainAutocorrectionIndicator;
    bool _shouldPreventSpellChecking;
    bool _triggeringEventIsUntrusted {false};

  public:
    TypingCommand(dom::Document &document, Type commandType, utf8_stringview textToInsert,
                  EnumSet<Option> options, TextGranularity granularity,
                  TextCompositionType compositionType) noexcept;

    static void DeleteSelection(dom::Document &document, EnumSet<Option> options = {},
                                TextCompositionType compositionType = TextCompositionType::None) noexcept;
    static void
      DeleteKeyPressed(dom::Document &document, EnumSet<Option> options = {},
                       TextGranularity granularity = TextGranularity::CharacterGranularity) noexcept;
    static void
      ForwardDeleteKeyPressed(dom::Document &document, EnumSet<Option> options = {},
                              TextGranularity granularity = TextGranularity::CharacterGranularity) noexcept;
    static void InsertText(dom::Document &document, utf8_stringview text, dom::Event *triggeringEvent,
                           EnumSet<Option> options,
                           TextCompositionType compositionType = TextCompositionType::None) noexcept;
    static void InsertText(dom::Document &document, utf8_stringview text, dom::Event *triggeringEvent,
                           const VisibleSelection &selectionForInsertion, EnumSet<Option> options,
                           TextCompositionType compositionType = TextCompositionType::None) noexcept;
    static void InsertLineBreak(dom::Document &document, EnumSet<Option> options) noexcept;
    static void InsertParagraphSeparator(dom::Document &document, EnumSet<Option> options) noexcept;
    static void InsertParagraphSeparatorInQuotedContent(dom::Document &document) noexcept;
    static void CloseTyping(dom::Document &document) noexcept;

    void InsertText(utf8_stringview text, bool selectInsertedText) noexcept;
    void InsertTextRunWithoutNewlines(utf8_stringview text, bool selectInsertedText) noexcept;
    void InsertLineBreak() noexcept;
    void InsertParagraphSeparatorInQuotedContent() noexcept;
    void InsertParagraphSeparator() noexcept;
    void DeleteKeyPressed(TextGranularity granularity, bool shouldAddToKillRing) noexcept;
    void ForwardDeleteKeyPressed(TextGranularity granularity, bool shouldAddToKillRing) noexcept;
    void DeleteSelection(bool smartDelete) noexcept;

    void SetCompositionType(TextCompositionType type) noexcept
    {
      _compositionType = type;
    }
    void SetIsAutocompletion(bool isAutocompletion) noexcept
    {
      _isAutocompletion = isAutocompletion;
    }
    KRYS_NODISCARD bool TriggeringEventIsUntrusted() const noexcept
    {
      return _triggeringEventIsUntrusted;
    }
    void SetTriggeringEventIsUntrusted(bool value) noexcept
    {
      _triggeringEventIsUntrusted = value;
    }

    KRYS_NODISCARD bool IsTypingCommand() const noexcept final;

  private:
    KRYS_NODISCARD static Ref<TypingCommand>
      Create(dom::Document &document, Type command, utf8_stringview text = {}, EnumSet<Option> options = {},
             TextGranularity granularity = TextGranularity::CharacterGranularity,
             TextCompositionType compositionType = TextCompositionType::None) noexcept;
    KRYS_NODISCARD static Ref<TypingCommand> Create(dom::Document &document, Type command,
                                                    utf8_stringview text, EnumSet<Option> options,
                                                    TextCompositionType compositionType) noexcept;

    void SetSmartDelete(bool smartDelete) noexcept
    {
      _smartDelete = smartDelete;
    }
    KRYS_NODISCARD bool IsOpenForMoreTyping() const noexcept
    {
      return _openForMoreTyping;
    }
    void CloseTyping() noexcept
    {
      _openForMoreTyping = false;
    }

    KRYS_NODISCARD static RefPtr<TypingCommand>
      LastTypingCommandIfStillOpenForTyping(dom::Document &document) noexcept;

    void DoApply() noexcept final;
    KRYS_NODISCARD bool PreservesTypingStyle() const noexcept final
    {
      return _preservesTypingStyle;
    }
    KRYS_NODISCARD bool ShouldRetainAutocorrectionIndicator() const noexcept final
    {
      krys_debug_assert(IsTopLevelCommand());
      return _shouldRetainAutocorrectionIndicator;
    }
    void SetShouldRetainAutocorrectionIndicator(bool retain) noexcept final
    {
      _shouldRetainAutocorrectionIndicator = retain;
    }
    KRYS_NODISCARD bool ShouldStopCaretBlinking() const noexcept final
    {
      return true;
    }
    void SetShouldPreventSpellChecking(bool prevent) noexcept
    {
      _shouldPreventSpellChecking = prevent;
    }

    KRYS_NODISCARD dom::DOMString InputEventTypeName() const noexcept final;
    KRYS_NODISCARD bool IsInputMethodComposing() const noexcept final;
    KRYS_NODISCARD dom::DOMString InputEventData() const noexcept final;
    KRYS_NODISCARD bool IsBeforeInputEventCancelable() const noexcept final;

    static void UpdateSelectionIfDifferentFromCurrentSelection(TypingCommand *typingCommand,
                                                               dom::Document &document) noexcept;

    void UpdatePreservesTypingStyle(Type commandType) noexcept;
    bool WillAddTypingToOpenCommand(Type commandType, TextGranularity granularity, utf8_stringview text = {},
                                    const Maybe<dom::SimpleRange> &range = null) noexcept;
    void TypingAddedToOpenCommand(Type commandTypeForAddedTyping) noexcept;
    bool MakeEditableRootEmpty() noexcept;

    bool WillApplyCommand() noexcept final;
    void DidApplyCommand() noexcept final;
    KRYS_NODISCARD bool ShouldDeferWillApplyCommandUntilAddingTypingCommand() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::TypingCommand)
  KRYS_NODISCARD static bool IsType(const krys::boo::CompositeEditCommand &command) noexcept
  {
    return command.IsTypingCommand();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
