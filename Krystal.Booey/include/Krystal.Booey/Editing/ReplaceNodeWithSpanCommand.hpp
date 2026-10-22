#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo
{
  /// @brief Replaces an element with a span that has its attributes and children.
  class ReplaceNodeWithSpanCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::Element> _elementToReplace;
    RefPtr<dom::Element> _spanElement;

  public:
    KRYS_NODISCARD static Ref<ReplaceNodeWithSpanCommand> Create(dom::Element &element) noexcept;

    explicit ReplaceNodeWithSpanCommand(dom::Element &element) noexcept;

    KRYS_NODISCARD dom::Element *SpanElement() const noexcept
    {
      return _spanElement.get();
    }

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
  };
}
