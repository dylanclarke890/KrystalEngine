#pragma once

#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/StyleSheet.hpp"
#include "Krystal.Booey/CSS/Types/MediaListOrDOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;

  template <typename>
  class ExceptionOr;
}

namespace krys::boo::style
{
  class Scope;
}

namespace krys::boo::css
{
  class CSSImportRule;
  class CSSRule;
  class CSSRuleList;
  class KeyframesRule;
  class StyleRule;
  class StyleRuleWithNesting;
  class StyleSheetContents;
  struct SerialisationContext;

  /// @brief A CSS style sheet exposed to scripts, wrapping its (possibly shared) parsed contents.
  /// @see https://drafts.csswg.org/cssom/#the-cssstylesheet-interface
  // FIXME: The async Replace (needs promises).
  class CSSStyleSheet final : public StyleSheet, public CanMakeWeakPtr<CSSStyleSheet>
  {
  public:
    /// @see https://drafts.csswg.org/cssom/#dictdef-cssstylesheetinit
    struct Init
    {
      Maybe<CSSOMString> BaseURL {};
      MediaListOrDOMString Media {CSSOMString()};
      bool Disabled {false};
    };

    enum class RuleMutationType : uint8
    {
      OtherMutation,
      RuleInsertion,
      KeyframesRuleMutation,
      RuleReplace
    };

    enum class ContentsClonedForMutation : bool
    {
      No,
      Yes
    };

    /// @brief Notifies the style sheet before and after its rules change.
    class RuleMutationScope
    {
      KRYS_NON_COPY_MOVABLE(RuleMutationScope);

    private:
      RefPtr<CSSStyleSheet> _styleSheet;
      RuleMutationType _mutationType;
      ContentsClonedForMutation _contentsClonedForMutation;
      RefPtr<KeyframesRule> _insertedKeyframesRule;
      CSSOMString _modifiedKeyframesRuleName;

    public:
      explicit RuleMutationScope(CSSStyleSheet *sheet,
                                 RuleMutationType mutationType = RuleMutationType::OtherMutation,
                                 KeyframesRule *insertedKeyframesRule = nullptr) noexcept;
      explicit RuleMutationScope(CSSRule *rule) noexcept;
      ~RuleMutationScope() noexcept;
    };

  private:
    Ref<StyleSheetContents> _contents;
    bool _isDisabled {false};
    bool _wasMutated {false};
    // The constructed flag in the spec.
    bool _wasConstructedByJS {false};
    Maybe<bool> _isOriginClean;
    CSSOMString _title;
    mq::MediaQueryList _mediaQueries;
    WeakPtr<style::Scope> _styleScope;
    WeakPtr<dom::Document> _constructorDocument;
    WeakPtr<dom::Node> _ownerNode;
    WeakPtr<CSSImportRule> _ownerRule;
    // The documents and shadow roots whose adoptedStyleSheets hold this sheet.
    List<WeakPtr<dom::ContainerNode>> _adoptingTreeScopes;

    mutable RefPtr<MediaList> _mediaCSSOMWrapper;
    mutable List<RefPtr<CSSRule>> _childRuleCSSOMWrappers;
    mutable UniquePtr<CSSRuleList> _ruleListCSSOMWrapper;

    CSSStyleSheet(Ref<StyleSheetContents> &&contents, CSSImportRule *ownerRule,
                  Maybe<bool> isOriginClean) noexcept;
    CSSStyleSheet(Ref<StyleSheetContents> &&contents, dom::Node &ownerNode,
                  Maybe<bool> isOriginClean) noexcept;
    CSSStyleSheet(Ref<StyleSheetContents> &&contents, dom::Document &document, Init &&options) noexcept;

  public:
    ~CSSStyleSheet() noexcept;

    KRYS_NODISCARD static Ref<CSSStyleSheet> Create(Ref<StyleSheetContents> &&contents,
                                                    CSSImportRule *ownerRule = nullptr,
                                                    Maybe<bool> isOriginClean = null) noexcept;
    KRYS_NODISCARD static Ref<CSSStyleSheet> Create(Ref<StyleSheetContents> &&contents, dom::Node &ownerNode,
                                                    Maybe<bool> isOriginClean = null) noexcept;

    /// @brief The sheet of a `style` element (or another element whose sheet is written inline).
    KRYS_NODISCARD static Ref<CSSStyleSheet> CreateInline(Ref<StyleSheetContents> &&contents,
                                                          dom::Element &owner) noexcept;

    /// @brief The `CSSStyleSheet` constructor: an empty sheet, constructed by script, for @p document.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-cssstylesheet
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSStyleSheet>> Create(dom::Document &document,
                                                                      Init &&options) noexcept;

    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final;
    KRYS_NODISCARD dom::Node *OwnerNode() const noexcept final;
    KRYS_NODISCARD MediaList *Media() const noexcept final;
    KRYS_NODISCARD CSSOMString Href() const noexcept final;

    KRYS_NODISCARD CSSOMString Title() const noexcept final
    {
      return _title;
    }

    KRYS_NODISCARD bool Disabled() const noexcept final
    {
      return _isDisabled;
    }

    void SetDisabled(bool disabled) noexcept final;

    /// @return Null if the rules can't be accessed (a cross-origin sheet).
    KRYS_NODISCARD RefPtr<CSSRuleList> CssRules() noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSRuleList>> CssRulesForBindings() noexcept;

    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSRuleList>> Rules() noexcept;

    dom::ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;
    dom::ExceptionOr<void> DeleteRule(uint32 index) noexcept;
    dom::ExceptionOr<int32> AddRule(const CSSOMString &selector, const CSSOMString &style,
                                    Maybe<uint32> index) noexcept;
    dom::ExceptionOr<void> RemoveRule(uint32 index) noexcept;
    dom::ExceptionOr<void> ReplaceSync(CSSOMString &&text) noexcept;

    KRYS_NODISCARD bool WasMutated() const noexcept
    {
      return _wasMutated;
    }

    KRYS_NODISCARD bool WasConstructedByJS() const noexcept
    {
      return _wasConstructedByJS;
    }

    KRYS_NODISCARD dom::Document *ConstructorDocument() const noexcept;

    void RemoveAdoptingTreeScope(dom::ContainerNode &treeScope) noexcept;
    void AddAdoptingTreeScope(dom::ContainerNode &treeScope) noexcept;

    KRYS_NODISCARD const List<WeakPtr<dom::ContainerNode>> &AdoptingTreeScopes() const noexcept
    {
      return _adoptingTreeScopes;
    }

    // For CSSRuleList.
    KRYS_NODISCARD uint32 Length() const noexcept;
    KRYS_NODISCARD CSSRule *Item(uint32 index) noexcept;

    void ClearOwnerNode() noexcept final;
    KRYS_NODISCARD CSSImportRule *OwnerRule() const noexcept final;
    KRYS_NODISCARD url::URL BaseURL() const noexcept final;
    KRYS_NODISCARD bool IsLoading() const noexcept final;

    void ClearOwnerRule() noexcept
    {
      _ownerRule = nullptr;
    }

    KRYS_NODISCARD dom::Document *OwnerDocument() const noexcept;
    KRYS_NODISCARD CSSStyleSheet &RootStyleSheet() noexcept;
    KRYS_NODISCARD const CSSStyleSheet &RootStyleSheet() const noexcept;
    KRYS_NODISCARD style::Scope *StyleScope() noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept
    {
      return _mediaQueries;
    }

    void SetMediaQueries(mq::MediaQueryList &&mediaQueries) noexcept;

    void SetTitle(const CSSOMString &title) noexcept
    {
      _title = title;
    }

    /// @brief Replaces the top-level @p styleRule with a style rule that can hold nested rules.
    /// @return The new rule, or null if @p styleRule isn't one of the sheet's rules.
    KRYS_NODISCARD RefPtr<StyleRuleWithNesting>
      PrepareChildStyleRuleForNesting(StyleRule &styleRule) noexcept;

    /// @brief Makes the contents safe to change, copying them first if they're shared.
    ContentsClonedForMutation WillMutateRules() noexcept;
    void DidMutateRules(RuleMutationType mutationType, ContentsClonedForMutation contentsClonedForMutation,
                        KeyframesRule *insertedKeyframesRule,
                        const CSSOMString &modifiedKeyframesRuleName) noexcept;
    void DidMutateRuleFromCSSStyleDeclaration() noexcept;
    void DidMutate() noexcept;

    void ClearChildRuleCSSOMWrappers() noexcept;
    void ReattachChildRuleCSSOMWrappers() noexcept;

    KRYS_NODISCARD StyleSheetContents &Contents() noexcept
    {
      return *_contents;
    }

    KRYS_NODISCARD Ref<StyleSheetContents> ProtectedContents() noexcept;

    void DetachFromDocument() noexcept
    {
      ClearOwnerNode();
    }

    KRYS_NODISCARD bool CanAccessRules() const noexcept;

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &context) noexcept;

    void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept;

    KRYS_NODISCARD bool IsDetached() const noexcept;

  private:
    void ForEachStyleScope(KRYS_NOESCAPE const Func<void(style::Scope &)> &apply) noexcept;

    KRYS_NODISCARD bool IsCSSStyleSheet() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD CSSOMString Type() const noexcept final;

    KRYS_NODISCARD RefPtr<CSSRuleList> CssRulesSkippingAccessCheck() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSStyleSheet)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::StyleSheet &styleSheet) noexcept
  {
    return styleSheet.IsCSSStyleSheet();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
