#pragma once

#include "Krystal.Booey/Css/Queries/MediaQuery.hpp"
#include "Krystal.Booey/Css/Rules/Rule.hpp"
#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedStyleSheetClient.hpp"

namespace krys::boo::css
{
  /// @brief An `@import` rule: the URL of a style sheet to load and include, and the media, cascade layer and
  /// `supports()` condition it applies under.
  /// @see https://drafts.csswg.org/css-cascade-5/#at-import
  // FIXME: User style sheets' imports (requestUserCSSStyleSheet), once there are user style sheets.
  class ImportRule final : public Rule
  {
  public:
    /// @brief The `supports()` condition, as written, and whether it held when the rule was parsed.
    struct SupportsCondition
    {
      CSSOMString Text;
      bool ConditionMatches {true};
    };

  private:
    /// @brief Hears the cached sheet arrive. It's a member rather than a base of `ImportRule` so that rules,
    /// which have no virtual functions, don't gain a vtable pointer.
    class ImportedStyleSheetClient final : public CachedStyleSheetClient
    {
    private:
      ImportRule &_ownerRule;

    public:
      explicit ImportedStyleSheetClient(ImportRule &ownerRule) noexcept : _ownerRule(ownerRule)
      {
      }

      void SetCSSStyleSheet(const utf8_string &href, const net::Url &baseUrl, utf8_stringview charset,
                            const CachedCSSStyleSheet *sheet) noexcept final
      {
        _ownerRule.SetCSSStyleSheet(href, baseUrl, charset, sheet);
      }
    };

    friend class ImportedStyleSheetClient;

    StyleSheetContents *_parentStyleSheet {nullptr};
    ImportedStyleSheetClient _styleSheetClient;
    CSSOMString _href;
    mq::MediaQueryList _mediaQueries;
    /// @brief The imported sheet, once it has loaded and been parsed.
    RefPtr<StyleSheetContents> _styleSheet;
    Maybe<CascadeLayerName> _cascadeLayerName;
    CachedResourceHandle<CachedCSSStyleSheet> _cachedSheet;
    /// @brief Whether the sheet has been requested and hasn't arrived.
    bool _loading {false};
    SupportsCondition _supportsCondition;

    ImportRule(CSSOMString href, mq::MediaQueryList mediaQueries, Maybe<CascadeLayerName> cascadeLayerName,
               SupportsCondition supportsCondition) noexcept;

    /// @brief Parses the sheet that has arrived and tells the parent sheet whether it loaded.
    void SetCSSStyleSheet(const utf8_string &href, const net::Url &baseUrl, utf8_stringview charset,
                          const CachedCSSStyleSheet *cachedStyleSheet) noexcept;

  public:
    ~ImportRule() noexcept;

    KRYS_NODISCARD static Ref<ImportRule> Create(CSSOMString href, mq::MediaQueryList mediaQueries,
                                                 Maybe<CascadeLayerName> cascadeLayerName,
                                                 SupportsCondition supportsCondition) noexcept;

    /// @brief Never called: a sheet with imports isn't cached, so it's never copied. It exists for
    /// `Rule::Copy`, which needs every rule type to have one.
    KRYS_NODISCARD static Ref<ImportRule> Copy() noexcept;

    /// @brief The sheet the rule is in, or null if it isn't in one.
    KRYS_NODISCARD StyleSheetContents *ParentStyleSheet() const noexcept
    {
      return _parentStyleSheet;
    }

    void SetParentStyleSheet(StyleSheetContents *sheet) noexcept
    {
      krys_debug_assert(sheet);
      _parentStyleSheet = sheet;
    }

    void ClearParentStyleSheet() noexcept
    {
      _parentStyleSheet = nullptr;
    }

    /// @brief Stops waiting for the sheet, so the parent sheet can finish loading without it.
    void CancelLoad() noexcept;

    /// @brief The URL as written.
    KRYS_NODISCARD CSSOMString Href() const noexcept
    {
      return _href;
    }

    /// @brief The imported sheet, or null until it has loaded.
    KRYS_NODISCARD StyleSheetContents *StyleSheet() const noexcept
    {
      return _styleSheet.get();
    }

    /// @brief Whether the sheet, or one it imports, is still loading.
    KRYS_NODISCARD bool IsLoading() const noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept
    {
      return _mediaQueries;
    }

    void SetMediaQueries(mq::MediaQueryList queries) noexcept
    {
      _mediaQueries = krys::move(queries);
    }

    /// @brief Fetches the sheet, unless the rule isn't in a document's sheet or would import itself.
    /// @see https://drafts.csswg.org/css-cascade-5/#fetch-an-import
    void RequestStyleSheet() noexcept;

    /// @brief The cached resource being loaded or loaded, or null if none was requested.
    KRYS_NODISCARD const CachedCSSStyleSheet *CachedStyleSheet() const noexcept
    {
      return _cachedSheet.get();
    }

    KRYS_NODISCARD const Maybe<CascadeLayerName> &GetCascadeLayerName() const noexcept
    {
      return _cascadeLayerName;
    }

    KRYS_NODISCARD const CSSOMString &SupportsText() const noexcept
    {
      return _supportsCondition.Text;
    }

    KRYS_NODISCARD bool SupportsMatches() const noexcept
    {
      return _supportsCondition.ConditionMatches;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ImportRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsImportRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
