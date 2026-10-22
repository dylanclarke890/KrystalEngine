#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class Node;
}

namespace krys::boo
{
  /// @brief Inserts a node before a sibling.
  class InsertNodeBeforeCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::Node> _insertChild;
    const Ref<dom::Node> _refChild;
    ShouldAssumeContentIsAlwaysEditable _shouldAssumeContentIsAlwaysEditable;

  public:
    KRYS_NODISCARD static Ref<InsertNodeBeforeCommand>
      Create(dom::Node &childToInsert, dom::Node &childToInsertBefore,
             ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
             EditAction editingAction = EditAction::Insert) noexcept;

    InsertNodeBeforeCommand(dom::Node &childToInsert, dom::Node &childToInsertBefore,
                            ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
                            EditAction editingAction) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
  };
}
