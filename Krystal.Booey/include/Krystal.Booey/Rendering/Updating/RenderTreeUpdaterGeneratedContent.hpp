#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  enum class PseudoElementType : uint8;
  class RenderElement;
  class RenderQuote;

  /// @brief Updates the `::before` and `::after` pseudo-elements of the elements being updated, their content
  /// renderers, and the quotes and counters.
  // FIXME: Image content (RenderImage) and writing suggestions come with images and editing.
  class RenderTreeUpdater::GeneratedContent
  {
    KRYS_NON_COPY_MOVABLE(GeneratedContent);

  private:
    RenderTreeUpdater &_updater;
    WeakPtr<RenderQuote> _previousUpdatedQuote;

  public:
    explicit GeneratedContent(RenderTreeUpdater &updater) noexcept;

    void UpdateBeforeOrAfterPseudoElement(dom::Element &current, const style::ElementUpdate &elementUpdate,
                                          PseudoElementType pseudoElementType) noexcept;
    void UpdateRemainingQuotes() noexcept;
    void UpdateBackdropRenderer(RenderElement &renderer,
                                style::DifferenceResult minimalStyleDifference) noexcept;
    void UpdateCounters() noexcept;

    static void RemoveBeforePseudoElement(dom::Element &element, RenderTreeBuilder &builder) noexcept;
    static void RemoveAfterPseudoElement(dom::Element &element, RenderTreeBuilder &builder) noexcept;

  private:
    void UpdateQuotesUpTo(RenderQuote *lastQuote) noexcept;

    KRYS_NODISCARD bool NeedsPseudoElement(const RenderStyle *style) noexcept;
  };
}
