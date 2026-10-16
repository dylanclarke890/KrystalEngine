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

    // Settings (`boo::Settings`), excluding those affecting properties. Standard features are always on.
    bool springTimingFunctionEnabled : 1 {false};
    bool cssTransformStyleSeparatedEnabled : 1 {false};
    bool thumbAndTrackPseudoElementsEnabled : 1 {false};
    bool colorLayersEnabled : 1 {false};
    // User agent sheets only.
    bool cssInternalAutoBaseParsingEnabled : 1 {false};
    // A document quirk.
    bool webkitMediaTextTrackDisplayQuirkEnabled : 1 {false};

    // Settings, those affecting properties.
    css::PropertySettings PropertySettings;

    /// @brief Creates a context for @p mode, with every setting at its default.
    /// @note Initialises the static value pool.
    explicit ParserContext(ParserMode mode, const url::URL &baseURL = {}) noexcept;

    /// @brief The document's context (`Document::CssParserContext()`).
    explicit ParserContext(const dom::Document &document) noexcept;

    /// @brief A context for a sheet in @p document: @p sheetBaseURL (or the document base URL if it's null),
    /// the document's mode and settings.
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
