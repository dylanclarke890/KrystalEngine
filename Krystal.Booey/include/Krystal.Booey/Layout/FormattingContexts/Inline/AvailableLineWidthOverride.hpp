#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  // This class overrides the total line width available for candidate content on a line.
  // Note that this only impacts how much content can fit in a line, and it does not change
  // the line box dimensions themselves (i.e. this won't change where text is aligned, etc).
  class AvailableLineWidthOverride
  {
  private:
    // Logical width constraint applied to all lines
    // Takes precedence over individual width overrides
    Maybe<LayoutUnit> _globalLineWidthOverride;

    // Logical width constraints applied separately for each line
    Maybe<List<LayoutUnit>> _individualLineWidthOverrides;

  public:
    AvailableLineWidthOverride() noexcept = default;
    AvailableLineWidthOverride(LayoutUnit globalLineWidthOverride) noexcept
        : _globalLineWidthOverride(globalLineWidthOverride)
    {
    }
    AvailableLineWidthOverride(List<LayoutUnit> individualLineWidthOverrides) noexcept
        : _individualLineWidthOverrides(krys::move(individualLineWidthOverrides))
    {
    }

    KRYS_NODISCARD Maybe<LayoutUnit> AvailableLineWidthOverrideForLine(size_t lineIndex) const noexcept
    {
      if (_globalLineWidthOverride)
      {
        return _globalLineWidthOverride;
      }
      if (_individualLineWidthOverrides && lineIndex < _individualLineWidthOverrides->size())
      {
        return (*_individualLineWidthOverrides)[lineIndex];
      }
      return {};
    }
  };
}
