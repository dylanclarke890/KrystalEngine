#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  /// @brief The at-rules the parser knows, by name.
  enum class AtRuleId : uint8
  {
    Invalid = 0,

    Charset,
    FontFace,
    Import,
    Keyframes,
    Media,
    Namespace,
    Page,
    PositionTry,
    Supports,
    ViewTransition,

    CounterStyle,
    Layer,
    Container,
    Property,

    FontFeatureValues,
    Stylistic,
    Styleset,
    CharacterVariant,
    Swash,
    Ornaments,
    Annotation,

    FontPaletteValues,
    Scope,
    StartingStyle,
    Function,
  };

  /// @brief Looks up an at-rule by name (without the `@`), ignoring ASCII case.
  /// @return The at-rule, or `AtRuleId::Invalid` if the name isn't a known at-rule.
  KRYS_NODISCARD AtRuleId AtRuleIdFromName(CSSOMStringView name) noexcept;
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::AtRuleId, 26uz);
