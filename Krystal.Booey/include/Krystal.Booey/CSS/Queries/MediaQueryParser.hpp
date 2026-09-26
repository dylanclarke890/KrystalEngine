#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"

namespace krys::boo::css::mq
{
  struct MediaQuery;
  using MediaQueryList = SmallList<MediaQuery>;

  struct MediaQueryParser : public GenericMediaQueryParser<MediaQueryParser>
  {
    KRYS_NODISCARD static MediaQueryList Parse(const CSSOMString &mediaQuery,
                                               const ParserContext &context) noexcept;

    KRYS_NODISCARD static MediaQueryList Parse(TokenRange tokens, const ParserContext &context) noexcept;

    KRYS_NODISCARD static Maybe<MediaQuery> ParseCondition(TokenRange tokens,
                                                           const ParserContext &context) noexcept;

    KRYS_NODISCARD static MediaQueryList ConsumeMediaQueryList(TokenRange &tokens,
                                                               const ParserContext &context) noexcept;

    KRYS_NODISCARD static Maybe<MediaQuery> ConsumeMediaQuery(TokenRange &tokens,
                                                              const ParserContext &context) noexcept;

    KRYS_NODISCARD static const FeatureSchema *
      SchemaForFeatureName(const CSSOMStringAtom &name, const ParserContext &context, State &state) noexcept;

    KRYS_NODISCARD static SmallList<const FeatureSchema *> FeatureSchemas() noexcept;
  };

  void Serialise(CSSOMString &builder, const MediaQueryList &queries) noexcept;

  void Serialise(CSSOMString &builder, const MediaQuery &query) noexcept;
}
