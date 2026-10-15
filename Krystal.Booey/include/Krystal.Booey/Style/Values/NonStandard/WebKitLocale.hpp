#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'-webkit-locale'> = auto | <string>
  // NOTE: There is no standard associated with this property.
  struct WebkitLocale
  {
    WebkitLocale(css::keywords::Auto) noexcept : _platform {css::CSSOMStringAtom::Null()}
    {
    }
    WebkitLocale(const css::CSSOMStringAtom &value) noexcept : _platform {value}
    {
    }
    WebkitLocale(css::CSSOMStringAtom &&value) noexcept : _platform {krys::move(value)}
    {
    }

    KRYS_NODISCARD const css::CSSOMStringAtom &GetPlatform() const noexcept KRYS_LIFETIME_BOUND
    {
      return _platform;
    }
    KRYS_NODISCARD css::CSSOMStringAtom TakePlatform() noexcept
    {
      return krys::move(_platform);
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return _platform == css::CSSOMStringAtom::Null();
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsAuto())
      {
        return visitor(css::keywords::Auto {});
      }

      // FIXME: It seems wrong that we extract/serialize the value as a <custom-ident>, given it is parsed as
      // a <string>, but this maintains existing behavior. See https://bugs.webkit.org/show_bug.cgi?id=302724.
      return visitor(css::CustomIdentifier {_platform});
    }

    KRYS_NODISCARD bool operator==(const WebkitLocale &) const noexcept = default;

  private:
    css::CSSOMStringAtom _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<WebkitLocale>
  {
    KRYS_NODISCARD WebkitLocale operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<WebkitLocale>
  {
    const css::CSSOMStringAtom &operator()(const WebkitLocale &value KRYS_LIFETIME_BOUND) noexcept
    {
      return value.GetPlatform();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::WebkitLocale)
