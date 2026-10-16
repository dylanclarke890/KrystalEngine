#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::mq
{
  /// @brief Parses media query lists (`@media`, `@import`, the `media` attribute, `matchMedia()`).
  struct MediaQueryParser : public GenericMediaQueryParser<MediaQueryParser>
  {
    KRYS_NODISCARD static MediaQueryList Parse(CSSOMStringView string, const ParserContext &context) noexcept;
    KRYS_NODISCARD static MediaQueryList Parse(TokenRange tokens, const ParserContext &context) noexcept;

    /// @brief Parses a bare `<media-condition>` (as in `@import ... supports() <condition>`); an empty range
    /// is `all`.
    KRYS_NODISCARD static Maybe<MediaQuery> ParseCondition(TokenRange tokens,
                                                           const ParserContext &context) noexcept;

    /// @brief Consumes a comma-separated list. A query that doesn't parse becomes `not all`.
    KRYS_NODISCARD static MediaQueryList ConsumeMediaQueryList(TokenRange &tokens,
                                                               const ParserContext &context) noexcept;
    KRYS_NODISCARD static Maybe<MediaQuery> ConsumeMediaQuery(TokenRange &tokens,
                                                              const ParserContext &context) noexcept;

    KRYS_NODISCARD static List<const FeatureSchema *> FeatureSchemas() noexcept;
  };

  void Serialise(StringBuilder &builder, const MediaQueryList &list) noexcept;
  void Serialise(StringBuilder &builder, const MediaQuery &query) noexcept;
}
