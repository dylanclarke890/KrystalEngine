#pragma once

#include "Krystal.Booey/CSS/CSSOM/MediaList.hpp"
#include "Krystal.Booey/CSS/CSSOM/StyleSheet.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Types/MediaListOrDOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class CSSImportRule;
  class CSSRule;
  class CSSRuleList;
  class ContainerNode;
  class Document;
  class Element;
  template <typename>
  class ExceptionOr;
  class Node;
}

namespace krys::boo::style
{
  class Scope;
}

namespace krys::boo::css
{
  class KeyframesRule;
  struct SerialisationContext;
  class StyleRule;
  class StyleRuleWithNesting;
  class StyleSheetContents;
}

namespace krys::boo
{
  /// @brief A CSS style sheet, as scripts see it. Its rules are parsed contents, which sheets with the same
  /// text can share until one is changed, when it gets its own copy.
  /// @see https://drafts.csswg.org/cssom/#the-cssstylesheet-interface
  // NOTE: There's no replace(): replaceSync() stands in for it, as there are no promises (see
  // DIFFERENCES.md).
  class CSSStyleSheet final : public StyleSheet, public CanMakeWeakPtr<CSSStyleSheet>
  {
  public:
    /// @see https://drafts.csswg.org/cssom/#dictdef-cssstylesheetinit
    struct Init
    {
      Maybe<CSSOMString> BaseUrl;
      css::MediaListOrDOMString Media {CSSOMString()};
      bool Disabled {false};
    };

    /// @brief What a change to the rules was, which decides what the style system updates.
    enum class RuleMutationType : uint8
    {
      OtherMutation,
      RuleInsertion,
      KeyframesRuleMutation,
      RuleReplace,
    };

    /// @brief Whether the contents were copied so the sheet could change them.
    enum class ContentsClonedForMutation : bool
    {
      No,
      Yes,
    };

    /// @brief Readies a style sheet's contents to be changed, and tells the style system what changed when it
    /// goes.
    class RuleMutationScope
    {
      KRYS_NON_COPY_MOVABLE(RuleMutationScope);

    private:
      RefPtr<CSSStyleSheet> _styleSheet;
      RuleMutationType _mutationType;
      ContentsClonedForMutation _contentsClonedForMutation {ContentsClonedForMutation::No};
      RefPtr<css::KeyframesRule> _insertedKeyframesRule;
      CSSOMString _modifiedKeyframesRuleName;

    public:
      explicit RuleMutationScope(CSSStyleSheet *styleSheet,
                                 RuleMutationType mutationType = RuleMutationType::OtherMutation,
                                 css::KeyframesRule *insertedKeyframesRule = nullptr) noexcept;

      /// @brief A change to @p rule, in its style sheet if it has one.
      explicit RuleMutationScope(CSSRule *rule) noexcept;

      ~RuleMutationScope() noexcept;
    };

  private:
    Ref<css::StyleSheetContents> _contents;
    bool _isDisabled {false};
    bool _wasMutated {false};
    // The constructed flag.
    bool _wasConstructedByJS {false};
    bool _isOriginClean {true};
    CSSOMString _title;
    css::mq::MediaQueryList _mediaQueries;
    WeakPtr<style::Scope> _styleScope;
    WeakPtr<Document> _constructorDocument;
    WeakPtr<Node> _ownerNode;
    WeakPtr<CSSImportRule> _ownerRule;
    // The documents and shadow roots whose adoptedStyleSheets hold this sheet.
    List<WeakPtr<ContainerNode>> _adoptingTreeScopes;
    // The media list, the rules' wrappers, made when first asked for, and their live list.
    mutable RefPtr<MediaList> _media;
    mutable List<RefPtr<CSSRule>> _childRules;
    mutable UniquePtr<CSSRuleList> _cssRules;

    CSSStyleSheet(Ref<css::StyleSheetContents> &&contents, CSSImportRule *ownerRule,
                  bool isOriginClean) noexcept;

    CSSStyleSheet(Ref<css::StyleSheetContents> &&contents, Node &ownerNode, bool isOriginClean) noexcept;

    CSSStyleSheet(Ref<css::StyleSheetContents> &&contents, Document &document, Init options) noexcept;

  public:
    ~CSSStyleSheet() noexcept override;

    /// @brief A sheet imported by @p ownerRule, or one without an owner.
    KRYS_NODISCARD static Ref<CSSStyleSheet> Create(Ref<css::StyleSheetContents> &&contents,
                                                    CSSImportRule *ownerRule = nullptr,
                                                    bool isOriginClean = true) noexcept;

    KRYS_NODISCARD static Ref<CSSStyleSheet> Create(Ref<css::StyleSheetContents> &&contents, Node &ownerNode,
                                                    bool isOriginClean = true) noexcept;

    /// @brief The sheet of a `style` element (or another element whose sheet is written inline).
    KRYS_NODISCARD static Ref<CSSStyleSheet> CreateInline(Ref<css::StyleSheetContents> &&contents,
                                                          Element &owner) noexcept;

    /// @brief The `CSSStyleSheet` constructor: an empty sheet, constructed by script, for @p document.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-cssstylesheet
    KRYS_NODISCARD static ExceptionOr<Ref<CSSStyleSheet>> Create(Document &document, Init init) noexcept;

#pragma region StyleSheet - https://drafts.csswg.org/cssom/#the-stylesheet-interface

    KRYS_NODISCARD CSSOMString Href() const noexcept final;

    KRYS_NODISCARD Node *OwnerNode() const noexcept final;

    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept final;

    KRYS_NODISCARD CSSOMString Title() const noexcept final
    {
      return _title;
    }

    KRYS_NODISCARD MediaList *Media() const noexcept final;

    KRYS_NODISCARD bool Disabled() const noexcept final
    {
      return _isDisabled;
    }

    void SetDisabled(bool disabled) noexcept final;

#pragma endregion

#pragma region CSSStyleSheet - https://drafts.csswg.org/cssom/#the-cssstylesheet-interface

    /// @brief The `@import` rule that imported the sheet, if any.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-ownerrule
    KRYS_NODISCARD CSSImportRule *OwnerRule() const noexcept final;

    /// @brief The rules, as a live list; null if the sheet isn't origin-clean.
    KRYS_NODISCARD RefPtr<CSSRuleList> CssRules() noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-cssrules
    KRYS_NODISCARD ExceptionOr<Ref<CSSRuleList>> CssRulesForBindings() noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-insertrule
    ExceptionOr<uint32> InsertRule(const CSSOMString &rule, uint32 index) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-deleterule
    ExceptionOr<void> DeleteRule(uint32 index) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-replacesync
    ExceptionOr<void> ReplaceSync(const CSSOMString &text) noexcept;

#pragma endregion

#pragma region Deprecated CSSStyleSheet members - https://drafts.csswg.org/cssom/#legacy-css-style-sheet-members

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-rules
    KRYS_NODISCARD ExceptionOr<Ref<CSSRuleList>> Rules() noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-addrule
    ExceptionOr<int32> AddRule(const CSSOMString &selector, const CSSOMString &block,
                               Maybe<uint32> optionalIndex) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-cssstylesheet-removerule
    ExceptionOr<void> RemoveRule(uint32 index) noexcept;

#pragma endregion

    KRYS_NODISCARD bool WasMutated() const noexcept
    {
      return _wasMutated;
    }

    KRYS_NODISCARD bool WasConstructedByJS() const noexcept
    {
      return _wasConstructedByJS;
    }

    KRYS_NODISCARD Document *ConstructorDocument() const noexcept;

    void RemoveAdoptingTreeScope(ContainerNode &treeScope) noexcept;

    void AddAdoptingTreeScope(ContainerNode &treeScope) noexcept;

    KRYS_NODISCARD const List<WeakPtr<ContainerNode>> &AdoptingTreeScopes() const noexcept
    {
      return _adoptingTreeScopes;
    }

    // For CSSRuleList.
    KRYS_NODISCARD uint32 Length() const noexcept;

    KRYS_NODISCARD CSSRule *Item(uint32 index) noexcept;

    void ClearOwnerNode() noexcept final;

    KRYS_NODISCARD net::Url BaseUrl() const noexcept final;

    KRYS_NODISCARD bool IsLoading() const noexcept final;

    void ClearOwnerRule() noexcept
    {
      _ownerRule = nullptr;
    }

    /// @brief The document of the owner node of the sheet at the top of the `@import` chain, if any.
    KRYS_NODISCARD Document *OwnerDocument() const noexcept;

    /// @brief The sheet at the top of the `@import` chain.
    KRYS_NODISCARD CSSStyleSheet &RootStyleSheet() noexcept;

    KRYS_NODISCARD const CSSStyleSheet &RootStyleSheet() const noexcept;

    KRYS_NODISCARD style::Scope *StyleScope() noexcept;

    KRYS_NODISCARD const css::mq::MediaQueryList &MediaQueries() const noexcept
    {
      return _mediaQueries;
    }

    void SetMediaQueries(css::mq::MediaQueryList &&mediaQueries) noexcept;

    void SetTitle(const CSSOMString &title) noexcept
    {
      _title = title;
    }

    /// @brief Replaces the top-level @p styleRule with a style rule that can hold nested rules.
    /// @return The new rule, or null if @p styleRule isn't one of the sheet's rules.
    KRYS_NODISCARD RefPtr<css::StyleRuleWithNesting>
      PrepareChildStyleRuleForNesting(css::StyleRule &styleRule) noexcept;

    /// @brief Makes the contents safe to change, copying them first if they're shared.
    ContentsClonedForMutation WillMutateRules() noexcept;

    /// @brief Tells the style system the rules changed.
    void DidMutateRules(RuleMutationType mutationType, ContentsClonedForMutation contentsClonedForMutation,
                        css::KeyframesRule *insertedKeyframesRule,
                        const CSSOMString &modifiedKeyframesRuleName) noexcept;

    /// @brief Tells the style system a rule's declarations changed.
    void DidMutateRuleFromCSSStyleDeclaration() noexcept;

    void DidMutate() noexcept;

    void ClearChildRuleCSSOMWrappers() noexcept;

    /// @brief Points the rules' wrappers at the rules of copied contents.
    void ReattachChildRuleCSSOMWrappers() noexcept;

    KRYS_NODISCARD css::StyleSheetContents &Contents() noexcept
    {
      return *_contents;
    }

    KRYS_NODISCARD Ref<css::StyleSheetContents> ProtectedContents() noexcept;

    void DetachFromDocument() noexcept
    {
      ClearOwnerNode();
    }

    /// @brief Whether the origin-clean flag is set, which lets scripts read and change the rules.
    KRYS_NODISCARD bool CanAccessRules() const noexcept
    {
      return _isOriginClean;
    }

    /// @brief The rules serialised in @p context, whether or not they can be accessed.
    KRYS_NODISCARD CSSOMString CssText(const css::SerialisationContext &context) noexcept;

    /// @brief Adds the style sheets this one imports, and those they import, to @p childStyleSheets.
    void GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept;

    /// @brief Whether the sheet has no owner node or rule and no document or shadow root has adopted it.
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CSSStyleSheet)
  KRYS_NODISCARD static bool IsType(const krys::boo::StyleSheet &styleSheet) noexcept
  {
    return styleSheet.IsCSSStyleSheet();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
