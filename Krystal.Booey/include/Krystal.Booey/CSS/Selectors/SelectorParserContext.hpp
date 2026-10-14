#pragma once

#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"

namespace krys::boo::css
{
  struct ParserContext;

  /// @brief The parser mode and settings that affect which selectors parse.
  struct SelectorParserContext
  {
    ParserMode Mode {ParserMode::HTMLStandard};
    bool imageControlsEnabled : 1 {false};
    bool popoverAttributeEnabled : 1 {false};
    bool targetTextPseudoElementEnabled : 1 {false};
    bool thumbAndTrackPseudoElementsEnabled : 1 {false};
    bool viewTransitionsEnabled : 1 {false};
    bool cssAppearanceBaseEnabled : 1 {false};
    bool webkitMediaTextTrackDisplayQuirkEnabled : 1 {false};

    SelectorParserContext() noexcept = default;

    explicit SelectorParserContext(const ParserContext &context) noexcept;

    // FIXME: SelectorParserContext(const dom::Document &), once Document has settings and quirks.

    KRYS_NODISCARD friend bool operator==(const SelectorParserContext &,
                                          const SelectorParserContext &) noexcept = default;
  };
}
