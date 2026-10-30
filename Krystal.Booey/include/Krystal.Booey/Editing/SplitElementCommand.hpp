#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class Element;
  class Node;
}

namespace krys::boo
{
  /// @brief Splits an element before one of its children: a clone takes the children before it.
  class SplitElementCommand final : public SimpleEditCommand
  {
  private:
    RefPtr<dom::Element> _element1;
    const Ref<dom::Element> _element2;
    const Ref<dom::Node> _atChild;

  public:
    KRYS_NODISCARD static Ref<SplitElementCommand> Create(dom::Element &element,
                                                          dom::Node &splitPointChild) noexcept;

    SplitElementCommand(dom::Element &element, dom::Node &splitPointChild) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
    void DoReapply() noexcept override;
    void ExecuteApply() noexcept;
  };
}
