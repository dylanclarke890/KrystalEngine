#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class Element;
  class Node;
}

namespace krys::boo
{
  /// @brief Merges an element into the identical element after it.
  class MergeIdenticalElementsCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::Element> _element1;
    const Ref<dom::Element> _element2;
    RefPtr<dom::Node> _atChild;

  public:
    KRYS_NODISCARD static Ref<MergeIdenticalElementsCommand> Create(dom::Element &element1,
                                                                    dom::Element &element2) noexcept;

    MergeIdenticalElementsCommand(dom::Element &element1, dom::Element &element2) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
  };
}
