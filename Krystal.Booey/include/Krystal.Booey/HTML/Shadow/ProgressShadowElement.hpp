#pragma once

#include "Krystal.Booey/HTML/HTMLDivElement.hpp"

namespace krys::boo::html
{
  class HTMLProgressElement;

  /// @brief An element of a `<progress>`'s shadow tree, rendered only when the progress element has no native
  /// appearance.
  class ProgressShadowElement : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProgressShadowElement);

  public:
    KRYS_NODISCARD HTMLProgressElement *ProgressElement() const noexcept;

  protected:
    explicit ProgressShadowElement(dom::Document &) noexcept;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override;
  };

  /// @brief The `::-webkit-progress-inner-element` part, rendered as a `RenderProgress`.
  class ProgressInnerElement final : public ProgressShadowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProgressInnerElement);

  public:
    explicit ProgressInnerElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<ProgressInnerElement> Create(dom::Document &) noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept override;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override;
  };

  /// @brief The `::-webkit-progress-bar` part.
  class ProgressBarElement final : public ProgressShadowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProgressBarElement);

  public:
    explicit ProgressBarElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<ProgressBarElement> Create(dom::Document &) noexcept;
  };

  /// @brief The `::-webkit-progress-value` part, as wide as the progress element's position.
  class ProgressValueElement final : public ProgressShadowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProgressValueElement);

  public:
    explicit ProgressValueElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<ProgressValueElement> Create(dom::Document &) noexcept;
    void SetInlineSizePercentage(double) noexcept;
  };
}
