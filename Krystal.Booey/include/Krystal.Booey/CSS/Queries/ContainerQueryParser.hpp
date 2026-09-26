#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryParser.hpp"

namespace krys::boo::css::cq
{
  struct ContainerQueryParser : mq::GenericMediaQueryParser<ContainerQueryParser>
  {
    KRYS_NODISCARD static Maybe<cq::ContainerQuery>
      ConsumeContainerQuery(TokenRange &tokens, const MediaQueryParserContext &context);

    KRYS_NODISCARD static bool IsValidFunctionId(ValueId functionId) noexcept;

    KRYS_NODISCARD static const mq::FeatureSchema *
      SchemaForFeatureName(const CSSOMStringAtom &name, const MediaQueryParserContext &context,
                           State &state) noexcept;

    KRYS_NODISCARD static SmallList<const mq::FeatureSchema *> FeatureSchemas() noexcept;
  };
}
