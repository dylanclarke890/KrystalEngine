#pragma once

#include "Krystal.Booey/Editing/Commands/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Replaces the selection with a paragraph break, splitting the block (and the inline elements) the
  /// caret is in.
  class InsertParagraphSeparatorCommand final : public CompositeEditCommand
  {
  private:
    RefPtr<EditingStyle> _style;
    bool _mustUseDefaultParagraphElement;

  public:
    InsertParagraphSeparatorCommand(dom::Document &document, bool mustUseDefaultParagraphElement,
                                    EditAction editingAction) noexcept;
    ~InsertParagraphSeparatorCommand() noexcept override;

    KRYS_NODISCARD static Ref<InsertParagraphSeparatorCommand>
      Create(dom::Document &document, bool useDefaultParagraphElement = false,
             EditAction editingAction = EditAction::Insert) noexcept;

  private:
    void DoApply() noexcept override;

    void CalculateStyleBeforeInsertion(const dom::Position &position) noexcept;
    void ApplyStyleAfterInsertion(dom::Node *originalEnclosingBlock) noexcept;
    void GetAncestorsInsideBlock(const dom::Node *insertionNode, dom::Element *outerBlock,
                                 List<RefPtr<dom::Element>> &ancestors) noexcept;
    Ref<dom::Element> CloneHierarchyUnderNewBlock(const List<RefPtr<dom::Element>> &ancestors,
                                                  Ref<dom::Element> &&blockToInsert) noexcept;
    KRYS_NODISCARD bool ShouldUseDefaultParagraphElement(dom::Node *enclosingBlock) const noexcept;
    KRYS_NODISCARD bool PreservesTypingStyle() const noexcept override;
  };
}
