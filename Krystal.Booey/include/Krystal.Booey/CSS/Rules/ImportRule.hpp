#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  /// @brief `@import`.
  // FIXME: Loading the imported sheet (RequestStyleSheet, the cached sheet and its client) comes with the
  // resource loader; until then StyleSheet() stays null.
  class ImportRule final : public Rule
  {
  public:
    struct SupportsCondition
    {
      CSSOMString Text;
      bool ConditionMatches {true};
    };

  private:
    StyleSheetContents *_parentStyleSheet {nullptr};

    CSSOMString _href;
    mq::MediaQueryList _mediaQueries;
    RefPtr<StyleSheetContents> _styleSheet;
    Maybe<CascadeLayerName> _cascadeLayerName;
    bool _loading {false};
    ImportRule::SupportsCondition _supportsCondition;

    ImportRule(const CSSOMString &href, mq::MediaQueryList &&mediaQueries,
               Maybe<CascadeLayerName> &&cascadeLayerName,
               ImportRule::SupportsCondition &&supportsCondition) noexcept;

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
