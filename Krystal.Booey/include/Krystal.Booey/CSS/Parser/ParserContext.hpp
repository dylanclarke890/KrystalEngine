#pragma once

#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/Loader/LoadedFromOpaqueSource.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Booey/URL/URLHash.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include <memory>

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  /// @brief The document state and settings that affect how CSS is parsed.
  struct ParserContext
  {
    url::URL BaseURL;
    utf8_stringview Charset;
    ParserMode Mode {ParserMode::HTMLStandard};
    Maybe<RuleType> EnclosingRuleType;
    bool IsHTMLDocument : 1 {false};

    // This is only needed to support getMatchedCSSRules.
    bool HasDocumentSecurityOrigin : 1 {false};

    boo::LoadedFromOpaqueSource LoadedFromOpaqueSource : 1 {boo::LoadedFromOpaqueSource::No};
    bool useSystemAppearance : 1 {false};
    bool shouldIgnoreImportRules : 1 {false};

    // Settings, excluding those affecting properties.
    bool counterStyleAtRuleImageSymbolsEnabled : 1 {false};
    bool springTimingFunctionEnabled : 1 {false};
    bool cssTransformStyleSeparatedEnabled : 1 {false};
    bool gridLanesEnabled : 1 {false};
    bool cssAppearanceBaseEnabled : 1 {false};
    bool cssPaintingAPIEnabled : 1 {false};
    bool cssShapeFunctionEnabled : 1 {false};
    bool cssTextDecorationLineErrorValues : 1 {false};
    bool cssBackgroundClipBorderAreaEnabled : 1 {false};
    bool cssWordBreakAutoPhraseEnabled : 1 {false};
    bool popoverAttributeEnabled : 1 {false};
    bool sidewaysWritingModesEnabled : 1 {false};
    bool cssTextWrapPrettyEnabled : 1 {true};
    bool thumbAndTrackPseudoElementsEnabled : 1 {false};
    bool imageControlsEnabled : 1 {false};
    bool colorLayersEnabled : 1 {false};
    bool contrastColorEnabled : 1 {false};
    bool targetTextPseudoElementEnabled : 1 {false};
    bool cssProgressFunctionEnabled : 1 {false};
    bool cssRandomFunctionEnabled : 1 {false};
    bool cssTreeCountingFunctionsEnabled : 1 {false};
    bool cssURLModifiersEnabled : 1 {false};
    bool cssURLIntegrityModifierEnabled : 1 {false};
    bool cssAxisRelativePositionKeywordsEnabled : 1 {false};
    bool cssDynamicRangeLimitMixEnabled : 1 {false};
    bool cssConstrainedDynamicRangeLimitEnabled : 1 {false};
    bool cssTextTransformMathAutoEnabled : 1 {false};
    bool cssInternalAutoBaseParsingEnabled : 1 {false};
    bool webkitMediaTextTrackDisplayQuirkEnabled : 1 {false};
    bool cssMathDepthEnabled : 1 {false};

    // Settings, those affecting properties.
    css::PropertySettings PropertySettings;

    /// @brief Creates a context for @p mode, with every setting at its default.
    /// @note Initialises the static value pool.
    explicit ParserContext(ParserMode mode, const url::URL &baseURL = {}) noexcept;

    /// @brief The document's context (`Document::CssParserContext()`).
    explicit ParserContext(const dom::Document &document) noexcept;

    /// @brief A context for a sheet in @p document: @p sheetBaseURL (or the document base URL if it's null),
    /// the document's mode and settings.
    /// FIXME: the settings are the defaults until there's a `Settings`; add the `Settings` constructor then.
    ParserContext(const dom::Document &document, const url::URL &sheetBaseURL,
                  utf8_stringview charset) noexcept;

    /// @brief Switches to the UA sheet mode and enables the features UA sheets rely on.
    void SetUASheetMode() noexcept;

    KRYS_NODISCARD bool operator==(const ParserContext &) const noexcept = default;

    KRYS_NODISCARD size_t Hash() const noexcept;
  };

  /// @brief A standards-mode context with every setting at its default.
  KRYS_NODISCARD const ParserContext &StrictParserContext() noexcept;
}

namespace krys
{
  template <>
  struct HashTraits<boo::css::ParserContext> : public GenericHashTraits<boo::css::ParserContext>
  {
    static void ConstructDeletedValue(boo::css::ParserContext &slot) noexcept
    {
      new (NotNullTag {}, std::addressof(slot.BaseURL)) boo::url::URL(HashTableDeletedValueTag {});
    }

    KRYS_NODISCARD static bool IsDeletedValue(const boo::css::ParserContext &value) noexcept
    {
      return value.BaseURL.IsHashTableDeletedValue();
    }

    KRYS_NODISCARD static boo::css::ParserContext EmptyValue() noexcept
    {
      return boo::css::ParserContext(boo::css::ParserMode::HTMLStandard);
    }
  };
}
