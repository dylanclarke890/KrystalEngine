#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedStyleSheetClient.hpp"

namespace krys::boo::css
{
  /// @brief `@import`.
  // FIXME: User style sheets' imports (requestUserCSSStyleSheet), once there are user style sheets.
  class ImportRule final : public Rule
  {
  public:
    struct SupportsCondition
    {
      CSSOMString Text;
      bool ConditionMatches {true};
    };

  private:
    // NOTE: We put the CachedStyleSheetClient in a member instead of inheriting from it
    // to avoid adding a vptr to ImportRule.
    class ImportedStyleSheetClient final : public CachedStyleSheetClient
    {
    private:
      ImportRule &_ownerRule;

    public:
      explicit ImportedStyleSheetClient(ImportRule &ownerRule) noexcept : _ownerRule(ownerRule)
      {
      }

      void SetCSSStyleSheet(const utf8_string &href, const url::URL &baseURL, utf8_stringview charset,
                            const CachedCSSStyleSheet *sheet) noexcept final
      {
        _ownerRule.SetCSSStyleSheet(href, baseURL, charset, sheet);
      }
    };
    friend class ImportedStyleSheetClient;

    StyleSheetContents *_parentStyleSheet {nullptr};
    ImportedStyleSheetClient _styleSheetClient;

    CSSOMString _href;
    mq::MediaQueryList _mediaQueries;
    RefPtr<StyleSheetContents> _styleSheet;
    Maybe<CascadeLayerName> _cascadeLayerName;
    CachedResourceHandle<CachedCSSStyleSheet> _cachedSheet;
    bool _loading {false};
    ImportRule::SupportsCondition _supportsCondition;

    ImportRule(const CSSOMString &href, mq::MediaQueryList &&mediaQueries,
               Maybe<CascadeLayerName> &&cascadeLayerName,
               ImportRule::SupportsCondition &&supportsCondition) noexcept;

    void SetCSSStyleSheet(const utf8_string &href, const url::URL &baseURL, utf8_stringview charset,
                          const CachedCSSStyleSheet *cachedStyleSheet) noexcept;

  public:
    ~ImportRule() noexcept;

    KRYS_NODISCARD static Ref<ImportRule> Create(const CSSOMString &href, mq::MediaQueryList &&mediaQueries,
                                                 Maybe<CascadeLayerName> &&cascadeLayerName,
                                                 ImportRule::SupportsCondition &&supportsCondition) noexcept;

    /// @attention Import rules are never copied.
    KRYS_NODISCARD Ref<ImportRule> Copy() const noexcept;

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

    void CancelLoad() noexcept;

    KRYS_NODISCARD CSSOMString Href() const noexcept
    {
      return _href;
    }

    KRYS_NODISCARD StyleSheetContents *StyleSheet() const noexcept
    {
      return _styleSheet.get();
    }

    KRYS_NODISCARD bool IsLoading() const noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept
    {
      return _mediaQueries;
    }

    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept
    {
      _mediaQueries = krys::move(queries);
    }

    void RequestStyleSheet() noexcept;
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
