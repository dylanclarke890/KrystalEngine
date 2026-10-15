#include "Krystal.Booey/Style/Values/Page/PageSize.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD PageSize PageSizeFromName(BuilderState &state, const css::PrimitiveValue &pageSizeName,
                                             const css::PrimitiveValue *pageOrientation) noexcept
    {
      auto mmLength = [](double mm)
      {
        return Length<css::NonNegative>(static_cast<float>(css::pxPerMm * mm));
      };

      auto inchLength = [](double inch)
      {
        return Length<css::NonNegative>(static_cast<float>(css::pxPerInch * inch));
      };

      static constexpr Length<css::NonNegative> a5Width(mmLength(148));
      static constexpr Length<css::NonNegative> a5Height(mmLength(210));
      static constexpr Length<css::NonNegative> a4Width(mmLength(210));
      static constexpr Length<css::NonNegative> a4Height(mmLength(297));
      static constexpr Length<css::NonNegative> a3Width(mmLength(297));
      static constexpr Length<css::NonNegative> a3Height(mmLength(420));
      static constexpr Length<css::NonNegative> b5Width(mmLength(176));
      static constexpr Length<css::NonNegative> b5Height(mmLength(250));
      static constexpr Length<css::NonNegative> b4Width(mmLength(250));
      static constexpr Length<css::NonNegative> b4Height(mmLength(353));
      static constexpr Length<css::NonNegative> jisB5Width(mmLength(182));
      static constexpr Length<css::NonNegative> jisB5Height(mmLength(257));
      static constexpr Length<css::NonNegative> jisB4Width(mmLength(257));
      static constexpr Length<css::NonNegative> jisB4Height(mmLength(364));
      static constexpr Length<css::NonNegative> letterWidth(inchLength(8.5));
      static constexpr Length<css::NonNegative> letterHeight(inchLength(11));
      static constexpr Length<css::NonNegative> legalWidth(inchLength(8.5));
      static constexpr Length<css::NonNegative> legalHeight(inchLength(14));
      static constexpr Length<css::NonNegative> ledgerWidth(inchLength(11));
      static constexpr Length<css::NonNegative> ledgerHeight(inchLength(17));

      Length<css::NonNegative> width {0};
      Length<css::NonNegative> height {0};

      switch (pageSizeName.ValueId())
      {
        case css::ValueId::A5:
        {
          width = a5Width;
          height = a5Height;
          break;
        }
        case css::ValueId::A4:
        {
          width = a4Width;
          height = a4Height;
          break;
        }
        case css::ValueId::A3:
        {
          width = a3Width;
          height = a3Height;
          break;
        }
        case css::ValueId::B5:
        {
          width = b5Width;
          height = b5Height;
          break;
        }
        case css::ValueId::B4:
        {
          width = b4Width;
          height = b4Height;
          break;
        }
        case css::ValueId::JisB5:
        {
          width = jisB5Width;
          height = jisB5Height;
          break;
        }
        case css::ValueId::JisB4:
        {
          width = jisB4Width;
          height = jisB4Height;
          break;
        }
        case css::ValueId::Letter:
        {
          width = letterWidth;
          height = letterHeight;
          break;
        }
        case css::ValueId::Legal:
        {
          width = legalWidth;
          height = legalHeight;
          break;
        }
        case css::ValueId::Ledger:
        {
          width = ledgerWidth;
          height = ledgerHeight;
          break;
        }
        default:
        {
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return css::keywords::Auto {};
        }
      }

      if (pageOrientation)
      {
        switch (pageOrientation->ValueId())
        {
          case css::ValueId::Landscape: std::swap(width, height); break;
          case css::ValueId::Portrait:
            // Nothing to do.
            break;
          default:
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
        }
      }

      return PageSize::Lengths {width, height};
    }
  }

  PageSize CSSValueConversion<PageSize>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *pair = DynamicDowncast<css::ValuePair>(value))
    {
      // <length [0,∞]>{2} | [ <page-size> [ portrait | landscape ] ]
      auto *first = RequiredDowncast<css::PrimitiveValue>(state, pair->First());
      if (!first)
      {
        return css::keywords::Auto {};
      }
      auto *second = RequiredDowncast<css::PrimitiveValue>(state, pair->Second());
      if (!second)
      {
        return css::keywords::Auto {};
      }

      if (first->IsLength())
      {
        // <length [0,∞]>{2}
        if (!second->IsLength())
        {
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return css::keywords::Auto {};
        }

        auto conversionData = state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f);
        return PageSize::Lengths {
          ConvertCSSValueToStyle<Length<css::NonNegative>>(conversionData, *first),
          ConvertCSSValueToStyle<Length<css::NonNegative>>(conversionData, *second),
        };
      }

      // [ <page-size> [ portrait | landscape ] ]
      // The value order is guaranteed. See Parser::ParseSizeParameter.
      return PageSizeFromName(state, *first, second);
    }

    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      // <length [0,∞]> | auto | <page-size> | [ portrait | landscape]
      if (primitiveValue->IsLength())
      {
        // <length [0,∞]>
        auto conversionData = state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f);
        auto length = ConvertCSSValueToStyle<Length<css::NonNegative>>(conversionData, *primitiveValue);
        return PageSize::Lengths {length, length};
      }

      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto:      return css::keywords::Auto {};
        case css::ValueId::Portrait:  return css::keywords::Portrait {};
        case css::ValueId::Landscape: return css::keywords::Landscape {};
        default:                      return PageSizeFromName(state, *primitiveValue, nullptr);
      }
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Auto {};
  }
}
