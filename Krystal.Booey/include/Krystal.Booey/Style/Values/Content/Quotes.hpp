#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'quote'> = auto | none | [ <string> <string> ]+
  // https://www.w3.org/TR/css-content-3/#propdef-quotes
  struct Quotes
  {
    using Data = css::SpaceSeparatedRefCountedFixedVector<css::CSSOMString>;

    Quotes(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }

    Quotes(css::keywords::None keyword) noexcept : _value {keyword}
    {
    }

    Quotes(Data &&data) noexcept : _value {krys::move(data)}
    {
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Auto>(_value);
    }
    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::None>(_value);
    }
    KRYS_NODISCARD bool IsQuotes() const noexcept
    {
      return krys::HoldsAlternative<Data>(_value);
    }

    KRYS_NODISCARD const css::CSSOMString &OpenQuote(unsigned index) const noexcept;
    KRYS_NODISCARD const css::CSSOMString &CloseQuote(unsigned index) const noexcept;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const Quotes &) const noexcept = default;

  private:
    Variant<css::keywords::Auto, css::keywords::None, Data> _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Quotes>
  {
    KRYS_NODISCARD Quotes operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Quotes)
