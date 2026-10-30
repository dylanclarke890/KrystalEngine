#include "Krystal.Booey/CSS/Values/FontValue.hpp"

namespace krys::boo::css
{
  CSSOMString FontValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    // font variant weight size / line-height family
    CSSOMString result;
    auto append = [&](CSSOMStringView separator, const Value &value)
    {
      result.append(separator);
      result.append(value.CssText(context));
    };

    if (Style)
    {
      result.append(Style->CssText(context));
    }
    if (Variant)
    {
      append(result.empty() ? u8"" : u8" ", *Variant);
    }
    if (Weight)
    {
      append(result.empty() ? u8"" : u8" ", *Weight);
    }
    if (Width)
    {
      append(result.empty() ? u8"" : u8" ", *Width);
    }
    if (Size)
    {
      append(result.empty() ? u8"" : u8" ", *Size);
    }
    if (LineHeight)
    {
      append(Size ? u8" / " : result.empty() ? u8"" : u8" ", *LineHeight);
    }
    if (Family)
    {
      append(result.empty() ? u8"" : u8" ", *Family);
    }
    return result;
  }

  bool FontValue::Equals(const FontValue &other) const noexcept
  {
    return CompareValuePtr(Style, other.Style) && CompareValuePtr(Variant, other.Variant)
           && CompareValuePtr(Weight, other.Weight) && CompareValuePtr(Width, other.Width)
           && CompareValuePtr(Size, other.Size) && CompareValuePtr(LineHeight, other.LineHeight)
           && CompareValuePtr(Family, other.Family);
  }

  IterationStatus
    FontValue::CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    if (Style)
    {
      if (func(*Style) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Variant)
    {
      if (func(*Variant) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Weight)
    {
      if (func(*Weight) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Width)
    {
      if (func(*Width) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Size)
    {
      if (func(*Size) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (LineHeight)
    {
      if (func(*LineHeight) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    if (Family)
    {
      if (func(*Family) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
    }
    return IterationStatus::Continue;
  }
}
