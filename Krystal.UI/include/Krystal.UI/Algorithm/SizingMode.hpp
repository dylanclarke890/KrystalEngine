#pragma once

#include "Krystal.UI/Enums/MeasureMode.hpp"
#include "Krystal.UI/Enums/SizingMode.hpp"
#include <stdexcept>

namespace Krys::UI
{
  inline MeasureMode ToMeasureMode(SizingMode mode)
  {
    switch (mode)
    {
      case SizingMode::StretchFit: return MeasureMode::Exactly;
      case SizingMode::MaxContent: return MeasureMode::Undefined;
      case SizingMode::FitContent: return MeasureMode::AtMost;
    }

    throw std::logic_error("Invalid SizingMode");
  }

  inline SizingMode ToSizingMode(MeasureMode mode)
  {
    switch (mode)
    {
      case MeasureMode::Exactly:   return SizingMode::StretchFit;
      case MeasureMode::Undefined: return SizingMode::MaxContent;
      case MeasureMode::AtMost:    return SizingMode::FitContent;
    }

    throw std::logic_error("Invalid MeasureMode");
  }
}