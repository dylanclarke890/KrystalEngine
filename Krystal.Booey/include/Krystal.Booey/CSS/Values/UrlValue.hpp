#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<url>` value, e.g. `url("image.png")`.
  class UrlValue final : public Value
  {
  private:
    CSSURL _url;

    explicit UrlValue(CSSURL &&url) noexcept : Value(ValueType::Url), _url {krys::move(url)}
    {
    }

  public:
    KRYS_NODISCARD static Ref<UrlValue> Create(CSSURL url) noexcept
    {
      return AdoptRef(*new UrlValue(krys::move(url)));
    }

    KRYS_NODISCARD const CSSURL &URL() const noexcept
    {
      return _url;
    }

    KRYS_NODISCARD CSSOMString StringValue() const noexcept
    {
      return _url.Specified;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const UrlValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::UrlValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsUrlValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
