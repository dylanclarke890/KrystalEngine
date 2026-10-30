#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo
{
  /// @brief Wraps an element's children in a style span.
  class WrapContentsInDummySpanCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::Element> _element;
    RefPtr<dom::Element> _dummySpan;

  public:
    KRYS_NODISCARD static Ref<WrapContentsInDummySpanCommand> Create(dom::Element &element) noexcept;

    explicit WrapContentsInDummySpanCommand(dom::Element &element) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
    void DoReapply() noexcept override;
    void ExecuteApply() noexcept;
  };
}
