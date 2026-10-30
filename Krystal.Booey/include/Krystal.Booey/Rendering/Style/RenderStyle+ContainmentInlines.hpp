#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

// Whether containment applies to an element's box, which depends on the element (replaced elements, relevance
// to the user) as well as its style. Kept apart from the getters so that the style headers don't depend on
// the DOM.
namespace krys::boo
{
  KRYS_NODISCARD inline bool ShouldApplyLayoutContainment(const RenderStyle &style,
                                                          const dom::Element &element) noexcept
  {
    // content-visibility hidden and auto turns on layout containment.
    auto hasContainment = style.UsedContain().Contains(style::ContainValue::Layout)
                          || style.GetContentVisibility() == ContentVisibility::Hidden
                          || style.GetContentVisibility() == ContentVisibility::Auto;
    if (!hasContainment)
    {
      return false;
    }
    // Giving an element layout containment has no effect if any of the following are true:
    //   if the element does not generate a principal box (as is the case with display: contents or display:
    //   none) if its principal box is an internal table box other than table-cell if its principal box is an
    //   internal ruby box or a non-atomic inline-level box
    if (style.Display() == DisplayType::None || style.Display() == DisplayType::Contents)
    {
      return false;
    }
    if (style.IsInternalTableBox() && style.Display() != DisplayType::TableCell)
    {
      return false;
    }
    if (style.IsRubyContainerOrInternalRubyBox()
        || (style.Display() == DisplayType::Inline && !element.IsReplaced(&style)))
    {
      return false;
    }
    return true;
  }

  KRYS_NODISCARD inline bool ShouldApplySizeContainment(const RenderStyle &style,
                                                        const dom::Element &element) noexcept
  {
    auto hasContainment =
      style.UsedContain().Contains(style::ContainValue::Size)
      || style.GetContentVisibility() == ContentVisibility::Hidden
      || (style.GetContentVisibility() == ContentVisibility::Auto && !element.IsRelevantToUser());
    if (!hasContainment)
    {
      return false;
    }
    // Giving an element size containment has no effect if any of the following are true:
    //   if the element does not generate a principal box (as is the case with display: contents or display:
    //   none) if its inner display type is table if its principal box is an internal table box if its
    //   principal box is an internal ruby box or a non-atomic inline-level box
    if (style.Display() == DisplayType::None || style.Display() == DisplayType::Contents)
    {
      return false;
    }
    if (style.Display() == DisplayType::Table || style.Display() == DisplayType::InlineTable)
    {
      return false;
    }
    if (style.IsInternalTableBox())
    {
      return false;
    }
    if (style.IsRubyContainerOrInternalRubyBox()
        || (style.Display() == DisplayType::Inline && !element.IsReplaced(&style)))
    {
      return false;
    }
    return true;
  }

  KRYS_NODISCARD inline bool ShouldApplyInlineSizeContainment(const RenderStyle &style,
                                                              const dom::Element &element) noexcept
  {
    if (!style.UsedContain().Contains(style::ContainValue::InlineSize))
    {
      return false;
    }
    // Giving an element inline-size containment has no effect if any of the following are true:
    //   if the element does not generate a principal box (as is the case with display: contents or display:
    //   none) if its inner display type is table if its principal box is an internal table box if its
    //   principal box is an internal ruby box or a non-atomic inline-level box
    if (style.Display() == DisplayType::None || style.Display() == DisplayType::Contents)
    {
      return false;
    }
    if (style.Display() == DisplayType::Table || style.Display() == DisplayType::InlineTable)
    {
      return false;
    }
    if (style.IsInternalTableBox())
    {
      return false;
    }
    if (style.IsRubyContainerOrInternalRubyBox()
        || (style.Display() == DisplayType::Inline && !element.IsReplaced(&style)))
    {
      return false;
    }
    return true;
  }

  KRYS_NODISCARD inline bool ShouldApplyStyleContainment(const RenderStyle &style,
                                                         const dom::Element &) noexcept
  {
    // content-visibility hidden and auto turns on style containment.
    return style.UsedContain().Contains(style::ContainValue::Style)
           || style.GetContentVisibility() == ContentVisibility::Hidden
           || style.GetContentVisibility() == ContentVisibility::Auto;
  }

  KRYS_NODISCARD inline bool ShouldApplyPaintContainment(const RenderStyle &style,
                                                         const dom::Element &element) noexcept
  {
    // content-visibility hidden and auto turns on paint containment.
    auto hasContainment = style.UsedContain().Contains(style::ContainValue::Paint)
                          || style.GetContentVisibility() == ContentVisibility::Hidden
                          || style.GetContentVisibility() == ContentVisibility::Auto;
    if (!hasContainment)
    {
      return false;
    }
    // Giving an element paint containment has no effect if any of the following are true:
    //   if the element does not generate a principal box (as is the case with display: contents or display:
    //   none) if its principal box is an internal table box other than table-cell if its principal box is an
    //   internal ruby box or a non-atomic inline-level box
    if (style.Display() == DisplayType::None || style.Display() == DisplayType::Contents)
    {
      return false;
    }
    if (style.IsInternalTableBox() && style.Display() != DisplayType::TableCell)
    {
      return false;
    }
    if (style.IsRubyContainerOrInternalRubyBox()
        || (style.Display() == DisplayType::Inline && !element.IsReplaced(&style)))
    {
      return false;
    }
    return true;
  }

  KRYS_NODISCARD inline bool IsSkippedContentRoot(const RenderStyle &style,
                                                  const dom::Element &element) noexcept
  {
    if (!ShouldApplySizeContainment(style, element))
    {
      return false;
    }

    switch (style.GetContentVisibility())
    {
      case ContentVisibility::Visible: return false;
      case ContentVisibility::Hidden:  return true;
      case ContentVisibility::Auto:    return !element.IsRelevantToUser();
    }
    krys_debug_assert(false);
    return false;
  }
}
