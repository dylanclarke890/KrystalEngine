#pragma once

#include "Krystal.Web/Accessibility/AXRenderObject.hpp"

namespace krys::web
{
  class HTMLMeterElement;
  class HTMLProgressElement;

  /// @brief The accessibility object of a `progress` or `meter` element, whose range comes from the
  /// element's attributes.
  /// @see https://w3c.github.io/html-aam/#el-progress
  /// @see https://w3c.github.io/html-aam/#el-meter
  class AXProgressIndicator final : public AXRenderObject
  {
  public:
    /// @brief The object for @p renderer's element.
    KRYS_NODISCARD static Ref<AXProgressIndicator> Create(AXId id, RenderObject &renderer,
                                                          AXObjectCache &cache) noexcept;

    /// @brief The object for @p element, which isn't rendered.
    KRYS_NODISCARD static Ref<AXProgressIndicator> Create(AXId id, Element &element,
                                                          AXObjectCache &cache) noexcept;

    /// @brief Whether it's a progress bar with no `value`.
    KRYS_NODISCARD bool IsIndeterminate() const noexcept override;

    /// @brief The element's value; 0 for an indeterminate progress bar.
    KRYS_NODISCARD float ValueForRange() const noexcept override;

    /// @brief The element's `max`.
    KRYS_NODISCARD float MaxValueForRange() const noexcept override;

    /// @brief A meter's `min`; 0 for a progress bar.
    KRYS_NODISCARD float MinValueForRange() const noexcept override;

    /// @brief `aria-valuetext`, or else a meter's text content.
    KRYS_NODISCARD DOMString ValueDescription() const noexcept override;

  private:
    AXProgressIndicator(AXId id, RenderObject *renderer, Node *node, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AXRole DetermineAccessibilityRole() noexcept override;

    /// @brief Only the common reasons (inertness, hiddenness, a presentational ancestor) hide it, not the
    /// rules for other render objects.
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override
    {
      return IsIgnoredByDefault();
    }

    KRYS_NODISCARD HTMLProgressElement *ProgressElement() const noexcept;

    KRYS_NODISCARD HTMLMeterElement *MeterElement() const noexcept;
  };
}
