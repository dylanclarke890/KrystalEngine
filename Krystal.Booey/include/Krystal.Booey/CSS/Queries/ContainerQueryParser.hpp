#pragma once

#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryParser.hpp"

namespace krys::boo::css::cq
{
  /// @brief Parses the prelude of an `@container` rule.
  struct ContainerQueryParser : mq::GenericMediaQueryParser<ContainerQueryParser>
  {
    KRYS_NODISCARD static Maybe<ContainerQuery>
      ConsumeContainerQuery(TokenRange &tokens, const MediaQueryParserContext &context) noexcept;

    KRYS_NODISCARD static bool IsValidFunctionId(ValueId functionId) noexcept;
    KRYS_NODISCARD static const mq::FeatureSchema *
      SchemaForFeatureName(const CSSOMStringAtom &name, const MediaQueryParserContext &context,
                           State &state) noexcept;
    KRYS_NODISCARD static List<const mq::FeatureSchema *> FeatureSchemas() noexcept;
  };
}
