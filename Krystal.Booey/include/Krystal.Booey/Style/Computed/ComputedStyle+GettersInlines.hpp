#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"

#define COMPUTED_STYLE_PROPERTIES_GETTERS_INLINES_INCLUDE_TRAP 1
#include "Krystal.Booey/Style/Computed/ComputedStyleProperties+GettersInlines.hpp"
#undef COMPUTED_STYLE_PROPERTIES_GETTERS_INLINES_INCLUDE_TRAP

namespace krys::boo::style
{
  inline bool ComputedStyle::ColumnSpanEqual(const ComputedStyle &other) const noexcept
  {
    if (_nonInheritedData.Ptr() == other._nonInheritedData.Ptr()
        || _nonInheritedData->MiscData.Ptr() == other._nonInheritedData->MiscData.Ptr()
        || _nonInheritedData->MiscData->MultiCol.Ptr() == other._nonInheritedData->MiscData->MultiCol.Ptr())
    {
      return true;
    }

    return _nonInheritedData->MiscData->MultiCol->ColumnSpan
           == other._nonInheritedData->MiscData->MultiCol->ColumnSpan;
  }

  inline bool ComputedStyle::ContainerTypeAndNamesEqual(const ComputedStyle &other) const noexcept
  {
    if (_nonInheritedData.Ptr() == other._nonInheritedData.Ptr()
        || _nonInheritedData->RareData.Ptr() == other._nonInheritedData->RareData.Ptr())
    {
      return true;
    }

    return GetContainerType() == other.GetContainerType() && GetContainerNames() == other.GetContainerNames();
  }

  inline bool ComputedStyle::ScrollPaddingEqual(const ComputedStyle &other) const noexcept
  {
    if (_nonInheritedData.Ptr() == other._nonInheritedData.Ptr()
        || _nonInheritedData->RareData.Ptr() == other._nonInheritedData->RareData.Ptr())
    {
      return true;
    }

    return _nonInheritedData->RareData->ScrollPadding == other._nonInheritedData->RareData->ScrollPadding;
  }

  inline bool ComputedStyle::FontCascadeEqual(const ComputedStyle &other) const noexcept
  {
    return _inheritedData.Ptr() == other._inheritedData.Ptr()
           || _inheritedData->FontData.Ptr() == other._inheritedData->FontData.Ptr()
           || _inheritedData->FontData->FontCascade == other._inheritedData->FontData->FontCascade;
  }
}
