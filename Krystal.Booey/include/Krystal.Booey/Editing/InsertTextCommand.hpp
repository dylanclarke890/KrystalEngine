#pragma once

#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Adds document markers to the text an insertion made.
  class TextInsertionMarkerSupplier : public RefCounted<TextInsertionMarkerSupplier>
  {
  public:
    virtual ~TextInsertionMarkerSupplier() noexcept = default;
    virtual void AddMarkersToTextNode(dom::Text &text, uint32 offsetOfInsertion,
                                      utf8_stringview textInserted) noexcept = 0;

  protected:
    TextInsertionMarkerSupplier() noexcept = default;
  };

  /// @brief Replaces the selection with a line of text (or a tab, which goes in a tab span).
  // FIXME: Smart lists, with InsertListCommand (on Cocoa platforms only).
  class InsertTextCommand : public CompositeEditCommand
  {
    friend class TypingCommand;

  public:
    enum RebalanceType : bool
    {
      RebalanceLeadingAndTrailingWhitespaces,
      RebalanceAllWhitespaces
    };

  private:
    utf8_string _text;
    AllowPasswordEcho _allowPasswordEcho {AllowPasswordEcho::Yes};
    bool _selectInsertedText;
    RebalanceType _rebalanceType;
    RefPtr<TextInsertionMarkerSupplier> _markerSupplier;

  public:
    InsertTextCommand(dom::Document &document, utf8_stringview text,
                      Ref<TextInsertionMarkerSupplier> &&markerSupplier, EditAction editingAction) noexcept;
    InsertTextCommand(dom::Document &document, utf8_stringview text, AllowPasswordEcho allowPasswordEcho,
                      bool selectInsertedText, RebalanceType rebalanceType,
                      EditAction editingAction) noexcept;

    KRYS_NODISCARD static Ref<InsertTextCommand>
      Create(dom::Document &document, utf8_stringview text, AllowPasswordEcho allowPasswordEcho,
             bool selectInsertedText = false,
             RebalanceType rebalanceType = RebalanceLeadingAndTrailingWhitespaces,
             EditAction editingAction = EditAction::Insert) noexcept;
    KRYS_NODISCARD static Ref<InsertTextCommand>
      CreateWithMarkerSupplier(dom::Document &document, utf8_stringview text,
                               Ref<TextInsertionMarkerSupplier> &&markerSupplier,
                               EditAction editingAction = EditAction::Insert) noexcept;

  private:
    void DoApply() noexcept override;
    KRYS_NODISCARD bool IsInsertTextCommand() const noexcept override
    {
      return true;
    }

    dom::Position PositionInsideTextNode(const dom::Position &position) noexcept;
    dom::Position InsertTab(const dom::Position &position) noexcept;

    bool PerformTrivialReplace(utf8_stringview text, bool selectInsertedText) noexcept;
    bool PerformOverwrite(utf8_stringview text, bool selectInsertedText) noexcept;
    void SetEndingSelectionWithoutValidation(const dom::Position &startPosition,
                                             const dom::Position &endPosition) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::InsertTextCommand)
  KRYS_NODISCARD static bool IsType(const krys::boo::EditCommand &command) noexcept
  {
    return command.IsInsertTextCommand();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
