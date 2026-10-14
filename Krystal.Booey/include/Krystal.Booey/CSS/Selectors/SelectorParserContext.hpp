#pragma once

#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"

namespace krys::boo::dom
{
  class Document;
}

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

    /// @brief The document's mode and settings.
    /// FIXME: the settings are the defaults until there's a `Settings`.
    explicit SelectorParserContext(const dom::Document &document) noexcept;

    KRYS_NODISCARD friend bool operator==(const SelectorParserContext &,
                                          const SelectorParserContext &) noexcept = default;
  };
}
