#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <'size'> (for @page) = <length [0,∞]>{1,2} | auto | [ <page-size> || [ portrait | landscape ] ]
  // https://drafts.csswg.org/css-page-3/#descdef-page-size
  struct PageSize
  {
    using Lengths = css::MinimallySerialisingSpaceSeparatedSize<Length<css::NonNegative>>;

    PageSize(Lengths &&lengths) noexcept : _value {krys::move(lengths)}
    {
    }
    PageSize(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }
    PageSize(css::keywords::Portrait keyword) noexcept : _value {keyword}
    {
    }
    PageSize(css::keywords::Landscape keyword) noexcept : _value {keyword}
    {
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const PageSize &) const noexcept = default;

  private:
    Variant<Lengths, css::keywords::Auto, css::keywords::Portrait, css::keywords::Landscape> _value {
      css::keywords::Auto {}};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<PageSize>
  {
    KRYS_NODISCARD PageSize operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PageSize)
