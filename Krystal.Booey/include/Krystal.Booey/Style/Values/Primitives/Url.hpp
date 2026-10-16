#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  /// @brief A computed `<url>`: the resolved URL and its modifiers.
  struct URL
  {
    url::URL Resolved;
    css::URLModifiers Modifiers;

    KRYS_NODISCARD bool operator==(const URL &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const URL &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Resolved;
    }
    if constexpr (I == 1)
    {
      return value.Modifiers;
    }
  }

  // MARK: Conversion

  /// @brief Special conversion function for use by filters and font-face code.
  KRYS_NODISCARD URL ToStyleWithScriptExecutionContext(const css::CSSURL &, const dom::Document &) noexcept;

  template <>
  struct ToCSS<URL>
  {
    KRYS_NODISCARD css::CSSURL operator()(const URL &, const RenderStyle &) noexcept;
  };

  template <>
  struct ToStyle<css::CSSURL>
  {
    KRYS_NODISCARD URL operator()(const css::CSSURL &, const BuilderState &) noexcept;
  };

  // `URL` is special-cased to return a `css::UrlValue`.
  template <>
  struct CSSValueCreation<URL>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const URL &) noexcept;
  };

  template <>
  struct CSSValueConversion<URL>
  {
    KRYS_NODISCARD URL operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: Serialisation

  template <>
  struct Serialiser<URL>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const URL &) noexcept;
  };
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::style::URL>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const boo::style::URL &value) noexcept
    {
      return value.Resolved.IsNull();
    }

    KRYS_NODISCARD static boo::style::URL EmptyValue() noexcept
    {
      return {.Resolved = {}, .Modifiers = {}};
    }
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::URL, 2)
