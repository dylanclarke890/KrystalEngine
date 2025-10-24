#pragma once

#include "Krystal.UI.Styles/Enums/MeasureMode.hpp"
#include "Krystal.UI.Styles/Enums/SizingMode.hpp"
#include <stdexcept>

namespace Krys::UI::Layout
{
  inline Styles::MeasureMode ToMeasureMode(Styles::SizingMode mode)
  {
    switch (mode)
    {
      case Styles::SizingMode::StretchFit: return Styles::MeasureMode::Exactly;
      case Styles::SizingMode::MaxContent: return Styles::MeasureMode::Undefined;
      case Styles::SizingMode::FitContent: return Styles::MeasureMode::AtMost;
    }

    throw std::logic_error("Invalid SizingMode");
  }

  inline Styles::SizingMode ToSizingMode(Styles::MeasureMode mode)
  {
    switch (mode)
    {
      case Styles::MeasureMode::Exactly:   return Styles::SizingMode::StretchFit;
      case Styles::MeasureMode::Undefined: return Styles::SizingMode::MaxContent;
      case Styles::MeasureMode::AtMost:    return Styles::SizingMode::FitContent;
    }

    throw std::logic_error("Invalid MeasureMode");
  }
}