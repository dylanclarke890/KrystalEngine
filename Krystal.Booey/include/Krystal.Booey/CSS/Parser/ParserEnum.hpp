#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  /// @brief The kind of rule a nested rule or selector is parsed inside.
  enum class NestedParserContextType : uint8
  {
    Style,
    Scope,
    Function,
  };

  /// @brief The innermost rule a selector or rule is nested inside, if any.
  using NestedContext = Maybe<NestedParserContextType>;
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::NestedParserContextType, 3uz);
