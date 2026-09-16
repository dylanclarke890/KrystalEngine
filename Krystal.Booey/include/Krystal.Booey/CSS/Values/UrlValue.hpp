#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class UrlValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<UrlValue> Create(const CSSURL &url) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::UrlValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsUrlValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()