#pragma once

#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/Graphics/FontFeatureValues.hpp"
#include "Krystal.Booey/Graphics/FontPaletteValues.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.Core/Utils/AllowCompactPointers.hpp"
#include <new>

namespace krys::boo::css
{
  class CSSGroupingRule;
  class CSSRule;
  class CSSStyleRule;
  class CSSStyleSheet;
  class MutableStyleProperties;
  class StyleProperties;
  class StyleSheetContents;

  using CascadeLayerName = List<CSSOMStringAtom>;

  /// @brief The base of the internal rule model: a parsed rule, shared by the CSSOM wrappers created for it.
  class Rule : public RefCounted<Rule>
  {
    // Needs access to the protected operator delete.
    friend class RefCounted<Rule>;
    KRYS_ENABLE_COMPACT_POINTERS;

  private:
    RuleType _type : 5;
    // This is only needed to support getMatchedCSSRules.
    bool _hasDocumentSecurityOrigin : 1;

  protected:
    explicit Rule(RuleType type, bool hasDocumentSecurityOrigin = false) noexcept
        : _type(type), _hasDocumentSecurityOrigin(hasDocumentSecurityOrigin)
    {
    }

    Rule(const Rule &other) noexcept
        : RefCounted(), _type(other._type), _hasDocumentSecurityOrigin(other._hasDocumentSecurityOrigin)
    {
    }

  public:
    KRYS_NODISCARD RuleType Type() const noexcept
    {
      return _type;
    }

#pragma region Type Checks

    KRYS_NODISCARD bool IsCharsetRule() const noexcept
    {
      return Type() == RuleType::Charset;
    }

    KRYS_NODISCARD bool IsCounterStyleRule() const noexcept
    {
      return Type() == RuleType::CounterStyle;
    }

    KRYS_NODISCARD bool IsFontFaceRule() const noexcept
    {
      return Type() == RuleType::FontFace;
    }

    KRYS_NODISCARD bool IsFontPaletteValuesRule() const noexcept
    {
      return Type() == RuleType::FontPaletteValues;
    }

    KRYS_NODISCARD bool IsFontFeatureValuesRule() const noexcept
    {
      return Type() == RuleType::FontFeatureValues;
    }

    KRYS_NODISCARD bool IsFontFeatureValuesBlockRule() const noexcept
    {
      return Type() == RuleType::FontFeatureValuesBlock;
    }

    KRYS_NODISCARD bool IsKeyframesRule() const noexcept
    {
      return Type() == RuleType::Keyframes;
    }

    KRYS_NODISCARD bool IsKeyframeRule() const noexcept
    {
      return Type() == RuleType::Keyframe;
    }

    KRYS_NODISCARD bool IsNamespaceRule() const noexcept
    {
      return Type() == RuleType::Namespace;
    }

    KRYS_NODISCARD bool IsMediaRule() const noexcept
    {
      return Type() == RuleType::Media;
    }

    KRYS_NODISCARD bool IsPageRule() const noexcept
    {
      return Type() == RuleType::Page;
    }

    KRYS_NODISCARD bool IsStyleRule() const noexcept
    {
      return Type() == RuleType::Style || Type() == RuleType::StyleWithNesting
             || Type() == RuleType::NestedDeclarations;
    }

    KRYS_NODISCARD bool IsStyleRuleWithNesting() const noexcept
    {
      return Type() == RuleType::StyleWithNesting;
    }

    KRYS_NODISCARD bool IsNestedDeclarationsRule() const noexcept
    {
      return Type() == RuleType::NestedDeclarations;
    }

    KRYS_NODISCARD bool IsGroupRule() const noexcept
    {
      return Type() == RuleType::Media || Type() == RuleType::Supports || Type() == RuleType::LayerBlock
             || Type() == RuleType::Container || Type() == RuleType::Scope
             || Type() == RuleType::StartingStyle || Type() == RuleType::Function;
    }

    KRYS_NODISCARD bool IsSupportsRule() const noexcept
    {
      return Type() == RuleType::Supports;
    }

    KRYS_NODISCARD bool IsImportRule() const noexcept
    {
      return Type() == RuleType::Import;
    }

    KRYS_NODISCARD bool IsLayerRule() const noexcept
    {
      return Type() == RuleType::LayerBlock || Type() == RuleType::LayerStatement;
    }

    KRYS_NODISCARD bool IsContainerRule() const noexcept
    {
      return Type() == RuleType::Container;
    }

    KRYS_NODISCARD bool IsPropertyRule() const noexcept
    {
      return Type() == RuleType::Property;
    }

    KRYS_NODISCARD bool IsScopeRule() const noexcept
    {
      return Type() == RuleType::Scope;
    }

    KRYS_NODISCARD bool IsStartingStyleRule() const noexcept
    {
      return Type() == RuleType::StartingStyle;
    }

    KRYS_NODISCARD bool IsViewTransitionRule() const noexcept
    {
      return Type() == RuleType::ViewTransition;
    }

    KRYS_NODISCARD bool IsPositionTryRule() const noexcept
    {
      return Type() == RuleType::PositionTry;
    }

#pragma endregion

    /// @brief Deep-copies the rule (its declarations and child rules), whatever its type.
    KRYS_NODISCARD Ref<Rule> Copy() const noexcept;

    /// @brief Creates the CSSOM wrapper exposing this rule to scripts.
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleSheet &parentSheet) const noexcept;
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSGroupingRule &parentRule) const noexcept;
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleRule &parentRule) const noexcept;

    // This is only needed to support getMatchedCSSRules.
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper() const noexcept;

  protected:
    void operator delete(Rule *rule, std::destroying_delete_t) noexcept;

    KRYS_NODISCARD bool HasDocumentSecurityOrigin() const noexcept
    {
      return _hasDocumentSecurityOrigin;
    }

    void SetType(RuleType type) noexcept
    {
      _type = type;
    }

    void InvalidateResolvedSelectorListRecursively() noexcept;

  private:
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleSheet *parentSheet,
                                                   CSSRule *parentRule) const noexcept;

    template <typename Visitor>
    constexpr decltype(auto) VisitDerived(Visitor &&) noexcept;

    template <typename Visitor>
    constexpr decltype(auto) VisitDerived(Visitor &&) const noexcept;
  };

  /// @brief A style rule: a selector list and a declaration block.
  class StyleRule : public Rule
  {
  private:
    bool _isSplitRule {false};
    bool _isLastRuleInSplitRule {false};

    mutable Ref<StyleProperties> _properties;
    SelectorList _selectorList;

  protected:
    StyleRule(Ref<StyleProperties> &&properties, bool hasDocumentSecurityOrigin,
              SelectorList &&selectors) noexcept;
    StyleRule(const StyleRule &other) noexcept;

  public:
    ~StyleRule() noexcept;

    KRYS_NODISCARD static Ref<StyleRule> Create(Ref<StyleProperties> &&properties,
                                                bool hasDocumentSecurityOrigin,
                                                SelectorList &&selectors) noexcept;

    KRYS_NODISCARD Ref<StyleRule> Copy() const noexcept;

    KRYS_NODISCARD const SelectorList &GetSelectorList() const noexcept
    {
      return _selectorList;
    }

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return *_properties;
    }

    KRYS_NODISCARD Ref<const StyleProperties> ProtectedProperties() const noexcept;

    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;

    KRYS_NODISCARD bool IsSplitRule() const noexcept
    {
      return _isSplitRule;
    }

    void MarkAsSplitRule() noexcept
    {
      _isSplitRule = true;
    }

    KRYS_NODISCARD bool IsLastRuleInSplitRule() const noexcept
    {
      return _isLastRuleInSplitRule;
    }

    void MarkAsLastRuleInSplitRule() noexcept
    {
      _isLastRuleInSplitRule = true;
    }

    using Rule::HasDocumentSecurityOrigin;

    // Used for CSSOM.
    void WrapperAdoptSelectorList(SelectorList &&selectors) noexcept;

    /// @brief Splits the rule into rules of at most @p maxCount selector components each (whole complex
    /// selectors only), sharing this rule's declarations.
    KRYS_NODISCARD List<Ref<StyleRule>>
      SplitIntoMultipleRulesWithMaximumSelectorComponentCount(uint32 maxCount) const noexcept;

    void AdoptSelectorList(SelectorList &&selectors) noexcept
    {
      _selectorList = krys::move(selectors);
    }

    KRYS_NODISCARD static uint32 AverageSizeInBytes() noexcept;

    void SetProperties(Ref<StyleProperties> &&properties) noexcept;

  private:
    KRYS_NODISCARD static Ref<StyleRule> CreateForSplitting(const List<const Selector *> &selectors,
                                                            Ref<StyleProperties> &&properties,
                                                            bool hasDocumentSecurityOrigin) noexcept;
  };

  /// @brief A style rule with nested rules. Its selector list is resolved against its parents' when the
  /// rule set is built; until then it holds only the author-written list.
  class StyleRuleWithNesting final : public StyleRule
  {
  private:
    List<Ref<Rule>> _nestedRules;
    SelectorList _originalSelectorList;

    StyleRuleWithNesting(Ref<StyleProperties> &&properties, bool hasDocumentSecurityOrigin,
                         SelectorList &&selectors, List<Ref<Rule>> &&nestedRules) noexcept;
    explicit StyleRuleWithNesting(StyleRule &&rule) noexcept;

  protected:
    StyleRuleWithNesting(const StyleRuleWithNesting &other) noexcept;

  public:
    ~StyleRuleWithNesting() noexcept;

    KRYS_NODISCARD static Ref<StyleRuleWithNesting> Create(Ref<StyleProperties> &&properties,
                                                           bool hasDocumentSecurityOrigin,
                                                           SelectorList &&selectors,
                                                           List<Ref<Rule>> &&nestedRules) noexcept;
    KRYS_NODISCARD static Ref<StyleRuleWithNesting> Create(StyleRule &&rule) noexcept;

    KRYS_NODISCARD Ref<StyleRuleWithNesting> Copy() const noexcept;

    KRYS_NODISCARD const List<Ref<Rule>> &NestedRules() const noexcept
    {
      return _nestedRules;
    }

    KRYS_NODISCARD List<Ref<Rule>> &NestedRules() noexcept
    {
      return _nestedRules;
    }

    KRYS_NODISCARD const SelectorList &OriginalSelectorList() const noexcept
    {
      return _originalSelectorList;
    }

    // Used by CSSOM.
    void WrapperAdoptOriginalSelectorList(SelectorList &&selectors) noexcept;
  };

  /// @brief Declarations that follow nested rules inside a style rule (or appear directly in a group rule).
  class NestedDeclarationsRule final : public StyleRule
  {
  private:
    explicit NestedDeclarationsRule(Ref<StyleProperties> &&properties) noexcept;
    NestedDeclarationsRule(const NestedDeclarationsRule &other) noexcept;

  public:
    KRYS_NODISCARD static Ref<NestedDeclarationsRule> Create(Ref<StyleProperties> &&properties) noexcept;

    KRYS_NODISCARD Ref<NestedDeclarationsRule> Copy() const noexcept;
  };

  /// @brief `@font-face`.
  class FontFaceRule final : public Rule
  {
  private:
    Ref<StyleProperties> _properties;

    explicit FontFaceRule(Ref<StyleProperties> &&properties) noexcept;
    FontFaceRule(const FontFaceRule &other) noexcept;

  public:
    ~FontFaceRule() noexcept;

    KRYS_NODISCARD static Ref<FontFaceRule> Create(Ref<StyleProperties> &&properties) noexcept;

    KRYS_NODISCARD Ref<FontFaceRule> Copy() const noexcept;

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return *_properties;
    }

    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;
  };

  /// @brief `@font-palette-values`.
  class FontPaletteValuesRule final : public Rule
  {
  private:
    CSSOMStringAtom _name;
    List<CSSOMStringAtom> _fontFamilies;
    boo::FontPaletteValues _fontPaletteValues;

    FontPaletteValuesRule(const CSSOMStringAtom &name, List<CSSOMStringAtom> &&fontFamilies,
                          Maybe<FontPaletteIndex> basePalette,
                          List<boo::FontPaletteValues::OverriddenColor> &&overrideColors) noexcept;
    FontPaletteValuesRule(const FontPaletteValuesRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<FontPaletteValuesRule>
      Create(const CSSOMStringAtom &name, List<CSSOMStringAtom> &&fontFamilies,
             Maybe<FontPaletteIndex> basePalette,
             List<boo::FontPaletteValues::OverriddenColor> &&overrideColors) noexcept;

    KRYS_NODISCARD Ref<FontPaletteValuesRule> Copy() const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const List<CSSOMStringAtom> &FontFamilies() const noexcept
    {
      return _fontFamilies;
    }

    KRYS_NODISCARD const boo::FontPaletteValues &FontPaletteValues() const noexcept
    {
      return _fontPaletteValues;
    }

    KRYS_NODISCARD Maybe<FontPaletteIndex> BasePalette() const noexcept
    {
      return _fontPaletteValues.BasePalette();
    }

    KRYS_NODISCARD const List<boo::FontPaletteValues::OverriddenColor> &OverrideColors() const noexcept
    {
      return _fontPaletteValues.OverrideColors();
    }
  };

  /// @brief A feature value block inside `@font-feature-values`, e.g. `@styleset { ... }`.
  class FontFeatureValuesBlockRule final : public Rule
  {
  private:
    boo::FontFeatureValuesType _type;
    List<FontFeatureValuesTag> _tags;

    FontFeatureValuesBlockRule(boo::FontFeatureValuesType type,
                               const List<FontFeatureValuesTag> &tags) noexcept;
    FontFeatureValuesBlockRule(const FontFeatureValuesBlockRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<FontFeatureValuesBlockRule>
      Create(boo::FontFeatureValuesType type, const List<FontFeatureValuesTag> &tags) noexcept;

    KRYS_NODISCARD Ref<FontFeatureValuesBlockRule> Copy() const noexcept;

    KRYS_NODISCARD boo::FontFeatureValuesType FontFeatureValuesType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD const List<FontFeatureValuesTag> &Tags() const noexcept
    {
      return _tags;
    }
  };

  /// @brief `@font-feature-values`.
  class FontFeatureValuesRule final : public Rule
  {
  private:
    List<CSSOMStringAtom> _fontFamilies;
    Ref<boo::FontFeatureValues> _value;

    FontFeatureValuesRule(const List<CSSOMStringAtom> &fontFamilies,
                          Ref<boo::FontFeatureValues> &&value) noexcept;
    FontFeatureValuesRule(const FontFeatureValuesRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<FontFeatureValuesRule> Create(const List<CSSOMStringAtom> &fontFamilies,
                                                            Ref<boo::FontFeatureValues> &&values) noexcept;

    KRYS_NODISCARD Ref<FontFeatureValuesRule> Copy() const noexcept;

    KRYS_NODISCARD const List<CSSOMStringAtom> &FontFamilies() const noexcept
    {
      return _fontFamilies;
    }

    KRYS_NODISCARD Ref<boo::FontFeatureValues> Value() const noexcept
    {
      return _value;
    }
  };

  /// @brief `@page`.
  class PageRule final : public Rule
  {
  private:
    Ref<StyleProperties> _properties;
    SelectorList _selectorList;

    PageRule(Ref<StyleProperties> &&properties, SelectorList &&selectors) noexcept;
    PageRule(const PageRule &other) noexcept;

  public:
    ~PageRule() noexcept;

    KRYS_NODISCARD static Ref<PageRule> Create(Ref<StyleProperties> &&properties,
                                               SelectorList &&selectors) noexcept;

    KRYS_NODISCARD Ref<PageRule> Copy() const noexcept;

    KRYS_NODISCARD const css::Selector *Selector() const noexcept
    {
      return _selectorList.FirstSelectorComponent();
    }

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return *_properties;
    }

    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;

    void WrapperAdoptSelectorList(SelectorList &&selectors) noexcept
    {
      _selectorList = krys::move(selectors);
    }
  };

  /// @brief The base of the rules that contain other rules.
  class GroupRule : public Rule
  {
    friend class CSSGroupingRule;
    friend class CSSStyleSheet;

  private:
    mutable List<Ref<Rule>> _childRules;

  protected:
    GroupRule(RuleType type, List<Ref<Rule>> &&childRules) noexcept;
    GroupRule(const GroupRule &other) noexcept;

  public:
    KRYS_NODISCARD const List<Ref<Rule>> &ChildRules() const noexcept
    {
      return _childRules;
    }

    void WrapperInsertRule(uint32 index, Ref<Rule> &&rule) noexcept;
    void WrapperRemoveRule(uint32 index) noexcept;
  };

  /// @brief `@media`.
  class MediaRule final : public GroupRule
  {
  private:
    mq::MediaQueryList _mediaQueries;

    MediaRule(mq::MediaQueryList &&mediaQueries, List<Ref<Rule>> &&childRules) noexcept;
    MediaRule(const MediaRule &other) noexcept;

  public:
    KRYS_NODISCARD static Ref<MediaRule> Create(mq::MediaQueryList &&mediaQueries,
                                                List<Ref<Rule>> &&childRules) noexcept;

    KRYS_NODISCARD Ref<MediaRule> Copy() const noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept
    {
      return _mediaQueries;
    }

    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept
    {
      _mediaQueries = krys::move(queries);
    }
  };

  /// @brief `@supports`.
  class SupportsRule final : public GroupRule
  {
  private:
    CSSOMString _conditionText;
    bool _conditionIsSupported;

    SupportsRule(const CSSOMString &conditionText, bool conditionIsSupported,
                 List<Ref<Rule>> &&rules) noexcept;
    SupportsRule(const SupportsRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<SupportsRule>
      Create(const CSSOMString &conditionText, bool conditionIsSupported, List<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD Ref<SupportsRule> Copy() const noexcept;

    KRYS_NODISCARD CSSOMString ConditionText() const noexcept
    {
      return _conditionText;
    }

    KRYS_NODISCARD bool ConditionIsSupported() const noexcept
    {
      return _conditionIsSupported;
    }
  };

  /// @brief `@layer`, as a statement (a list of names) or a block (one name and child rules).
  class LayerRule final : public GroupRule
  {
  private:
    Variant<CascadeLayerName, List<CascadeLayerName>> _nameVariant;

    explicit LayerRule(List<CascadeLayerName> &&nameList) noexcept;
    LayerRule(CascadeLayerName &&name, List<Ref<Rule>> &&rules) noexcept;
    LayerRule(const LayerRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<LayerRule> CreateStatement(List<CascadeLayerName> &&nameList) noexcept;
    KRYS_NODISCARD static Ref<LayerRule> CreateBlock(CascadeLayerName &&name,
                                                     List<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD Ref<LayerRule> Copy() const noexcept;

    KRYS_NODISCARD bool IsStatement() const noexcept
    {
      return Type() == RuleType::LayerStatement;
    }

    KRYS_NODISCARD const CascadeLayerName &Name() const noexcept
    {
      return std::get<CascadeLayerName>(_nameVariant);
    }

    KRYS_NODISCARD const List<CascadeLayerName> &NameList() const noexcept
    {
      return std::get<List<CascadeLayerName>>(_nameVariant);
    }
  };

  /// @brief `@container`.
  class ContainerRule final : public GroupRule
  {
  private:
    cq::ContainerQuery _containerQuery;

    ContainerRule(cq::ContainerQuery &&query, List<Ref<Rule>> &&rules) noexcept;
    ContainerRule(const ContainerRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<ContainerRule> Create(cq::ContainerQuery &&query,
                                                    List<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD Ref<ContainerRule> Copy() const noexcept;

    KRYS_NODISCARD const cq::ContainerQuery &ContainerQuery() const noexcept
    {
      return _containerQuery;
    }
  };

  /// @brief `@property`.
  class PropertyRule final : public Rule
  {
  public:
    struct Descriptor
    {
      CSSOMStringAtom Name;
      CSSOMString Syntax {};
      Maybe<bool> Inherits {};
      RefPtr<const VariableData> InitialValue {};
    };

  private:
    PropertyRule::Descriptor _descriptor;

    explicit PropertyRule(PropertyRule::Descriptor &&descriptor) noexcept;
    PropertyRule(const PropertyRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<PropertyRule> Create(PropertyRule::Descriptor &&descriptor) noexcept;

    KRYS_NODISCARD Ref<PropertyRule> Copy() const noexcept;

    KRYS_NODISCARD const PropertyRule::Descriptor &GetDescriptor() const noexcept
    {
      return _descriptor;
    }
  };

  /// @brief `@scope`.
  class ScopeRule final : public GroupRule
  {
  private:
    // Resolved selector lists
    SelectorList _scopeStart;
    SelectorList _scopeEnd;
    // Author written selector lists
    SelectorList _originalScopeStart;
    SelectorList _originalScopeEnd;
    // Pointer to the owner StyleSheetContent to find the implicit scope (when there is no <scope-start>)
    WeakPtr<const StyleSheetContents> _styleSheetOwner;

    ScopeRule(SelectorList &&scopeStart, SelectorList &&scopeEnd, List<Ref<Rule>> &&rules) noexcept;
    ScopeRule(const ScopeRule &other) noexcept;

  public:
    ~ScopeRule() noexcept;

    KRYS_NODISCARD static Ref<ScopeRule> Create(SelectorList &&scopeStart, SelectorList &&scopeEnd,
                                                List<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD Ref<ScopeRule> Copy() const noexcept;

    KRYS_NODISCARD const SelectorList &ScopeStart() const noexcept
    {
      return _scopeStart;
    }

    KRYS_NODISCARD const SelectorList &ScopeEnd() const noexcept
    {
      return _scopeEnd;
    }

    KRYS_NODISCARD const SelectorList &OriginalScopeStart() const noexcept
    {
      return _originalScopeStart;
    }

    KRYS_NODISCARD const SelectorList &OriginalScopeEnd() const noexcept
    {
      return _originalScopeEnd;
    }

    void SetScopeStart(SelectorList &&scopeStart) noexcept
    {
      _scopeStart = krys::move(scopeStart);
    }

    void SetScopeEnd(SelectorList &&scopeEnd) noexcept
    {
      _scopeEnd = krys::move(scopeEnd);
    }

    KRYS_NODISCARD WeakPtr<const StyleSheetContents> GetStyleSheetContents() const noexcept;

    void SetStyleSheetContents(const StyleSheetContents &sheet) noexcept;
  };

  /// @brief `@starting-style`.
  class StartingStyleRule final : public GroupRule
  {
  private:
    explicit StartingStyleRule(List<Ref<Rule>> &&rules) noexcept;
    StartingStyleRule(const StartingStyleRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<StartingStyleRule> Create(List<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD Ref<StartingStyleRule> Copy() const noexcept;
  };

  /// @brief `@charset`.
  /// @note This is only used by the CSS parser.
  class CharsetRule final : public Rule
  {
  private:
    CharsetRule() noexcept;
    CharsetRule(const CharsetRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<CharsetRule> Create() noexcept;

    KRYS_NODISCARD Ref<CharsetRule> Copy() const noexcept;
  };

  /// @brief `@namespace`.
  class NamespaceRule final : public Rule
  {
  private:
    CSSOMStringAtom _prefix;
    CSSOMStringAtom _uri;

    NamespaceRule(const CSSOMStringAtom &prefix, const CSSOMStringAtom &uri) noexcept;
    NamespaceRule(const NamespaceRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<NamespaceRule> Create(const CSSOMStringAtom &prefix,
                                                    const CSSOMStringAtom &uri) noexcept;

    KRYS_NODISCARD Ref<NamespaceRule> Copy() const noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Prefix() const noexcept
    {
      return _prefix;
    }

    KRYS_NODISCARD const CSSOMStringAtom &Uri() const noexcept
    {
      return _uri;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::StyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::StyleRuleWithNesting)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsStyleRuleWithNesting();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::NestedDeclarationsRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsNestedDeclarationsRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GroupRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsGroupRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFaceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsFontFaceRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFeatureValuesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsFontFeatureValuesRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFeatureValuesBlockRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsFontFeatureValuesBlockRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontPaletteValuesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsFontPaletteValuesRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::MediaRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsMediaRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PageRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsPageRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::SupportsRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsSupportsRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::NamespaceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsNamespaceRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CharsetRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsCharsetRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::LayerRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsLayerRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ContainerRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsContainerRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PropertyRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsPropertyRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ScopeRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsScopeRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::StartingStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsStartingStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
