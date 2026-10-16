#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  enum class PseudoElementType : uint8;

  /// @brief Updates the `::before` and `::after` pseudo-elements of the elements being updated.
  // FIXME: The renderers of the generated content itself (text, images, quotes and counters), ::backdrop and
  // writing suggestions come with the render tree.
  class RenderTreeUpdater::GeneratedContent
  {
    KRYS_NON_COPY_MOVABLE(GeneratedContent);

  private:
    RenderTreeUpdater &_updater;

  public:
    explicit GeneratedContent(RenderTreeUpdater &updater) noexcept;

    void UpdateBeforeOrAfterPseudoElement(dom::Element &current, const style::ElementUpdate &elementUpdate,
                                          PseudoElementType pseudoElementType) noexcept;

    static void RemoveBeforePseudoElement(dom::Element &element, RenderTreeBuilder &builder) noexcept;
    static void RemoveAfterPseudoElement(dom::Element &element, RenderTreeBuilder &builder) noexcept;

  private:
    KRYS_NODISCARD bool NeedsPseudoElement(const RenderStyle *style) noexcept;
  };
}
