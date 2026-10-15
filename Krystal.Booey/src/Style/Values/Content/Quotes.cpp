#include "Krystal.Booey/Style/Values/Content/Quotes.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD const css::CSSOMString &EmptyString() noexcept
    {
      static const css::CSSOMString empty;
      return empty;
    }
  }

  const css::CSSOMString &Quotes::OpenQuote(unsigned index) const noexcept
  {
    return krys::SwitchOn(
      _value,
      [&](const Data &data) -> const css::CSSOMString &
      {
        auto i = index * 2;

        if (i < data.size())
        {
          return data[i];
        }
        return data[data.size() - 2];
      },
      [&](const auto &) -> const css::CSSOMString & { return EmptyString(); });
  }

  const css::CSSOMString &Quotes::CloseQuote(unsigned index) const noexcept
  {
    return krys::SwitchOn(
      _value,
      [&](const Data &data) -> const css::CSSOMString &
      {
        auto i = (index * 2) + 1;

        if (i < data.size())
        {
          return data[i];
        }
        return data[data.size() - 1];
      },
      [&](const auto &) -> const css::CSSOMString & { return EmptyString(); });
  }

  // MARK: - Conversion

  Quotes CSSValueConversion<Quotes>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto: return css::keywords::Auto {};
        case css::ValueId::None: return css::keywords::None {};
        default:                 break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue, 2>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    if (list->size() % 2 != 0)
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    return Quotes::Data::map(*list, [](const css::PrimitiveValue &item) { return item.StringValue(); });
  }
}
