#pragma once

#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontFeatureValues.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontPaletteValues.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/AllowCompactPointers.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <new>

namespace krys::boo
{
  class CSSGroupingRule;
  class CSSRule;
  class CSSStyleRule;
  class CSSStyleSheet;
}

namespace krys::boo::css
{
  class MutableStyleProperties;
  class StyleProperties;
  class StyleSheetContents;

  /// @brief A [cascade layer](https://drafts.csswg.org/css-cascade-5/#layering) name, one identifier per
  /// dot-separated part.
  using CascadeLayerName = List<CSSOMStringAtom>;

  /// @brief A parsed rule, the base of the internal rule model. The CSSOM `CSSRule` objects created for it
  /// wrap it; several sheets' wrappers can share one until a client modifies it.
  /// @note Rules have no virtual functions: `Type()` says which class a rule is, and `VisitDerived`
  /// dispatches on it, for destruction too.
  class Rule : public RefCounted<Rule>
  {
    // Calls the destroying operator delete.
    friend class RefCounted<Rule>;
    KRYS_ENABLE_COMPACT_POINTERS;

  private:
    RuleType _type;

  protected:
    explicit Rule(RuleType type) noexcept : _type(type)
    {
    }

    // A copy starts with its own reference count.
    Rule(const Rule &other) noexcept : _type(other._type)
    {
    }

  public:
    KRYS_NODISCARD RuleType Type() const noexcept
    {
      return _type;
    }

#pragma region Type checks

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

    /// @brief Whether this is a `StyleRule`, including its nesting and nested-declarations subclasses.
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

    /// @brief Whether this is a `GroupRule`, one holding child rules.
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

    /// @brief Whether this is an `@layer` statement or block.
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

    KRYS_NODISCARD bool IsPositionTryRule() const noexcept
    {
      return Type() == RuleType::PositionTry;
    }

#pragma endregion

    /// @brief A deep copy (declarations and child rules included), for a sheet that will modify it.
    KRYS_NODISCARD Ref<Rule> Copy() const noexcept;

    /// @brief Creates the CSSOM wrapper for this rule as a rule of @p parentSheet.
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleSheet &parentSheet) noexcept;

    /// @brief Creates the CSSOM wrapper for this rule as a child of @p parentRule.
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSGroupingRule &parentRule) noexcept;

    /// @brief Creates the CSSOM wrapper for this rule as a rule nested in @p parentRule.
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleRule &parentRule) noexcept;

    /// @brief Creates a CSSOM wrapper with no parent style sheet or rule.
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper() noexcept;

  protected:
    void operator delete(Rule *rule, std::destroying_delete_t) noexcept;

    /// @brief Lets a subclass sharing its base's constructor set its own type.
    void SetType(RuleType type) noexcept
    {
      _type = type;
    }

    /// @brief Clears the resolved selector lists of this rule and the rules in it, after the selectors they
    /// were resolved against changed.
    void InvalidateResolvedSelectorListRecursively() noexcept;

  private:
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleSheet *parentSheet, CSSRule *parentRule) noexcept;

    /// @brief Calls @p visitor with this rule as its most derived class.
    template <typename TVisitor>
    constexpr decltype(auto) VisitDerived(TVisitor &&visitor) noexcept;

    template <typename TVisitor>
    constexpr decltype(auto) VisitDerived(TVisitor &&visitor) const noexcept;
  };

  /// @brief A [style rule](https://drafts.csswg.org/css-syntax-3/#style-rule): a selector list and a
  /// declaration block.
  class StyleRule : public Rule
  {
  private:
    bool _isSplitRule {false};
    bool _isLastRuleInSplitRule {false};
    mutable Ref<StyleProperties> _properties;
    SelectorList _selectorList;

  protected:
    StyleRule(Ref<StyleProperties> properties, SelectorList selectors) noexcept;

    StyleRule(const StyleRule &other) noexcept;

  public:
    ~StyleRule() noexcept;

    KRYS_NODISCARD static Ref<StyleRule> Create(Ref<StyleProperties> properties,
                                                SelectorList selectors) noexcept;

    KRYS_NODISCARD Ref<StyleRule> Copy() const noexcept;

    /// @brief The selectors the rule matches with; for a nesting rule, resolved against its parents'.
    KRYS_NODISCARD const SelectorList &GetSelectorList() const noexcept
    {
      return _selectorList;
    }

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return *_properties;
    }

    /// @brief The declarations, first replaced by a mutable copy if they aren't mutable.
    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;

    /// @brief Whether the parser split this rule from one with too many selector components.
    KRYS_NODISCARD bool IsSplitRule() const noexcept
    {
      return _isSplitRule;
    }

    void MarkAsSplitRule() noexcept
    {
      _isSplitRule = true;
    }

    /// @brief Whether this is the last of the rules split from one.
    KRYS_NODISCARD bool IsLastRuleInSplitRule() const noexcept
    {
      return _isLastRuleInSplitRule;
    }

    void MarkAsLastRuleInSplitRule() noexcept
    {
      _isLastRuleInSplitRule = true;
    }

    /// @brief Splits the rule into rules of at most @p maxCount selector components each (whole complex
    /// selectors only), sharing this rule's declarations.
    KRYS_NODISCARD List<Ref<StyleRule>>
      SplitIntoMultipleRulesWithMaximumSelectorComponentCount(uint32 maxCount) const noexcept;

    /// @brief Replaces the selectors, for the CSSOM's `selectorText` setter.
    void WrapperAdoptSelectorList(SelectorList selectors) noexcept;

    void AdoptSelectorList(SelectorList selectors) noexcept
    {
      _selectorList = krys::move(selectors);
    }

    /// @brief A typical style rule's size, for estimating a sheet's.
    KRYS_NODISCARD static uint32 AverageSizeInBytes() noexcept;

  private:
    /// @brief A rule matching the complex selectors whose components are @p selectors, rightmost first.
    KRYS_NODISCARD static Ref<StyleRule> CreateForSplitting(const List<const Selector *> &selectors,
                                                            Ref<StyleProperties> properties) noexcept;
  };

  /// @brief A style rule with [nested rules](https://drafts.csswg.org/css-nesting-1/). It keeps the selectors
  /// as written; `GetSelectorList` holds them resolved against its parents', once a rule set has been built
  /// from the sheet.
  class StyleRuleWithNesting final : public StyleRule
  {
  private:
    List<Ref<Rule>> _nestedRules;
    SelectorList _originalSelectorList;

    StyleRuleWithNesting(Ref<StyleProperties> properties, SelectorList selectors,
                         List<Ref<Rule>> nestedRules) noexcept;

    explicit StyleRuleWithNesting(StyleRule &&rule) noexcept;

  protected:
    StyleRuleWithNesting(const StyleRuleWithNesting &other) noexcept;

  public:
    ~StyleRuleWithNesting() noexcept;

    KRYS_NODISCARD static Ref<StyleRuleWithNesting>
      Create(Ref<StyleProperties> properties, SelectorList selectors, List<Ref<Rule>> nestedRules) noexcept;

    /// @brief A nesting rule with @p rule's declarations and selectors, to which nested rules are being
    /// added.
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

    /// @brief The selectors as written, which may contain `&`.
    KRYS_NODISCARD const SelectorList &OriginalSelectorList() const noexcept
    {
      return _originalSelectorList;
    }

    /// @brief Replaces the selectors as written, for the CSSOM's `selectorText` setter, clearing the
    /// resolved selectors here and in the nested rules.
    void WrapperAdoptOriginalSelectorList(SelectorList selectors) noexcept;
  };

  /// @brief Declarations that follow a nested rule in a style rule, or that appear directly in a group rule
  /// nested in one
  /// ([`CSSNestedDeclarations`](https://drafts.csswg.org/css-nesting-1/#cssnesteddeclarations)). Its
  /// selectors are its parent style rule's, once resolved.
  class NestedDeclarationsRule final : public StyleRule
  {
  private:
    explicit NestedDeclarationsRule(Ref<StyleProperties> properties) noexcept;

    NestedDeclarationsRule(const NestedDeclarationsRule &other) noexcept;

  public:
    KRYS_NODISCARD static Ref<NestedDeclarationsRule> Create(Ref<StyleProperties> properties) noexcept;

    KRYS_NODISCARD Ref<NestedDeclarationsRule> Copy() const noexcept;
  };

  /// @brief [`@font-face`](https://drafts.csswg.org/css-fonts-4/#font-face-rule).
  class FontFaceRule final : public Rule
  {
  private:
    Ref<StyleProperties> _properties;

    explicit FontFaceRule(Ref<StyleProperties> properties) noexcept;

    FontFaceRule(const FontFaceRule &other) noexcept;

  public:
    ~FontFaceRule() noexcept;

    KRYS_NODISCARD static Ref<FontFaceRule> Create(Ref<StyleProperties> properties) noexcept;

    KRYS_NODISCARD Ref<FontFaceRule> Copy() const noexcept;

    /// @brief The descriptors.
    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return *_properties;
    }

    /// @brief The descriptors, first replaced by a mutable copy if they aren't mutable.
    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;
  };

  /// @brief [`@font-palette-values`](https://drafts.csswg.org/css-fonts-4/#font-palette-values).
  class FontPaletteValuesRule final : public Rule
  {
  private:
    CSSOMStringAtom _name;
    List<CSSOMStringAtom> _fontFamilies;
    boo::FontPaletteValues _fontPaletteValues;

    FontPaletteValuesRule(CSSOMStringAtom name, List<CSSOMStringAtom> fontFamilies,
                          Maybe<FontPaletteIndex> basePalette,
                          List<boo::FontPaletteValues::OverriddenColor> overrideColors) noexcept;

    FontPaletteValuesRule(const FontPaletteValuesRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<FontPaletteValuesRule>
      Create(CSSOMStringAtom name, List<CSSOMStringAtom> fontFamilies, Maybe<FontPaletteIndex> basePalette,
             List<boo::FontPaletteValues::OverriddenColor> overrideColors) noexcept;

    KRYS_NODISCARD Ref<FontPaletteValuesRule> Copy() const noexcept;

    /// @brief The palette's `<dashed-ident>` name.
    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    /// @brief The families the palette applies to (its `font-family` descriptor).
    KRYS_NODISCARD const List<CSSOMStringAtom> &FontFamilies() const noexcept
    {
      return _fontFamilies;
    }

    KRYS_NODISCARD const boo::FontPaletteValues &FontPaletteValues() const noexcept
    {
      return _fontPaletteValues;
    }

    /// @brief The font palette the palette starts from (its `base-palette` descriptor).
    KRYS_NODISCARD Maybe<FontPaletteIndex> BasePalette() const noexcept
    {
      return _fontPaletteValues.BasePalette();
    }

    /// @brief The colours it replaces (its `override-colors` descriptor).
    KRYS_NODISCARD const List<boo::FontPaletteValues::OverriddenColor> &OverrideColors() const noexcept
    {
      return _fontPaletteValues.OverrideColors();
    }
  };

  /// @brief A [feature value block](https://drafts.csswg.org/css-fonts-4/#font-feature-values-syntax) in
  /// `@font-feature-values`, such as `@styleset { nice-style: 12; }`.
  class FontFeatureValuesBlockRule final : public Rule
  {
  private:
    boo::FontFeatureValuesType _type;
    List<FontFeatureValuesTag> _tags;

    FontFeatureValuesBlockRule(boo::FontFeatureValuesType type, List<FontFeatureValuesTag> tags) noexcept;

    FontFeatureValuesBlockRule(const FontFeatureValuesBlockRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<FontFeatureValuesBlockRule> Create(boo::FontFeatureValuesType type,
                                                                 List<FontFeatureValuesTag> tags) noexcept;

    KRYS_NODISCARD Ref<FontFeatureValuesBlockRule> Copy() const noexcept;

    /// @brief Which feature the block names values for (`@styleset`, `@swash` and so on).
    KRYS_NODISCARD boo::FontFeatureValuesType FontFeatureValuesType() const noexcept
    {
      return _type;
    }

    /// @brief The block's names and the values each stands for.
    KRYS_NODISCARD const List<FontFeatureValuesTag> &Tags() const noexcept
    {
      return _tags;
    }
  };

  /// @brief [`@font-feature-values`](https://drafts.csswg.org/css-fonts-4/#font-feature-values).
  class FontFeatureValuesRule final : public Rule
  {
  private:
    List<CSSOMStringAtom> _fontFamilies;
    Ref<boo::FontFeatureValues> _value;

    FontFeatureValuesRule(List<CSSOMStringAtom> fontFamilies, Ref<boo::FontFeatureValues> value) noexcept;

    FontFeatureValuesRule(const FontFeatureValuesRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<FontFeatureValuesRule> Create(List<CSSOMStringAtom> fontFamilies,
                                                            Ref<boo::FontFeatureValues> value) noexcept;

    KRYS_NODISCARD Ref<FontFeatureValuesRule> Copy() const noexcept;

    /// @brief The families the values apply to.
    KRYS_NODISCARD const List<CSSOMStringAtom> &FontFamilies() const noexcept
    {
      return _fontFamilies;
    }

    /// @brief The named values of all the rule's blocks.
    KRYS_NODISCARD Ref<boo::FontFeatureValues> Value() const noexcept
    {
      return _value;
    }
  };

  /// @brief The base of the rules that hold a list of child rules (the conditional rules, `@layer` blocks,
  /// `@scope`, `@starting-style` and `@function`).
  class GroupRule : public Rule
  {
    friend class boo::CSSGroupingRule;
    friend class boo::CSSStyleSheet;

  private:
    mutable List<Ref<Rule>> _childRules;

  protected:
    GroupRule(RuleType type, List<Ref<Rule>> childRules) noexcept;

    GroupRule(const GroupRule &other) noexcept;

  public:
    KRYS_NODISCARD const List<Ref<Rule>> &ChildRules() const noexcept
    {
      return _childRules;
    }

    /// @brief Inserts @p rule at @p index, for the CSSOM's `insertRule`, which has checked it's allowed.
    void WrapperInsertRule(uint32 index, Ref<Rule> rule) noexcept;

    /// @brief Removes the rule at @p index, for the CSSOM's `deleteRule`.
    void WrapperRemoveRule(uint32 index) noexcept;
  };

  /// @brief [`@media`](https://drafts.csswg.org/css-conditional-3/#at-media).
  class MediaRule final : public GroupRule
  {
  private:
    mq::MediaQueryList _mediaQueries;

    MediaRule(mq::MediaQueryList mediaQueries, List<Ref<Rule>> childRules) noexcept;

    MediaRule(const MediaRule &other) noexcept;

  public:
    KRYS_NODISCARD static Ref<MediaRule> Create(mq::MediaQueryList mediaQueries,
                                                List<Ref<Rule>> childRules) noexcept;

    KRYS_NODISCARD Ref<MediaRule> Copy() const noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept
    {
      return _mediaQueries;
    }

    void SetMediaQueries(mq::MediaQueryList queries) noexcept
    {
      _mediaQueries = krys::move(queries);
    }
  };

  /// @brief [`@supports`](https://drafts.csswg.org/css-conditional-3/#at-supports).
  class SupportsRule final : public GroupRule
  {
  private:
    CSSOMString _conditionText;
    bool _conditionIsSupported;

    SupportsRule(CSSOMString conditionText, bool conditionIsSupported, List<Ref<Rule>> rules) noexcept;

    SupportsRule(const SupportsRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<SupportsRule> Create(CSSOMString conditionText, bool conditionIsSupported,
                                                   List<Ref<Rule>> rules) noexcept;

    KRYS_NODISCARD Ref<SupportsRule> Copy() const noexcept;

    /// @brief The condition, serialised.
    KRYS_NODISCARD CSSOMString ConditionText() const noexcept
    {
      return _conditionText;
    }

    /// @brief Whether the condition held when the rule was parsed.
    KRYS_NODISCARD bool ConditionIsSupported() const noexcept
    {
      return _conditionIsSupported;
    }
  };

  /// @brief [`@layer`](https://drafts.csswg.org/css-cascade-5/#layering): a statement declaring layer names,
  /// or a block of rules in one layer (anonymous if its name is empty).
  class LayerRule final : public GroupRule
  {
  private:
    Variant<CascadeLayerName, List<CascadeLayerName>> _nameVariant;

    explicit LayerRule(List<CascadeLayerName> nameList) noexcept;

    LayerRule(CascadeLayerName name, List<Ref<Rule>> rules) noexcept;

    LayerRule(const LayerRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<LayerRule> CreateStatement(List<CascadeLayerName> nameList) noexcept;

    KRYS_NODISCARD static Ref<LayerRule> CreateBlock(CascadeLayerName name, List<Ref<Rule>> rules) noexcept;

    KRYS_NODISCARD Ref<LayerRule> Copy() const noexcept;

    KRYS_NODISCARD bool IsStatement() const noexcept
    {
      return Type() == RuleType::LayerStatement;
    }

    /// @brief A block's layer name.
    KRYS_NODISCARD const CascadeLayerName &Name() const noexcept
    {
      return std::get<CascadeLayerName>(_nameVariant);
    }

    /// @brief A statement's layer names.
    KRYS_NODISCARD const List<CascadeLayerName> &NameList() const noexcept
    {
      return std::get<List<CascadeLayerName>>(_nameVariant);
    }
  };

  /// @brief [`@container`](https://drafts.csswg.org/css-conditional-5/#container-rule).
  class ContainerRule final : public GroupRule
  {
  private:
    cq::ContainerQueryList _containerQueries;

    ContainerRule(cq::ContainerQueryList queries, List<Ref<Rule>> rules) noexcept;

    ContainerRule(const ContainerRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<ContainerRule> Create(cq::ContainerQueryList queries,
                                                    List<Ref<Rule>> rules) noexcept;

    KRYS_NODISCARD Ref<ContainerRule> Copy() const noexcept;

    /// @brief The rule's queries, of which any must be true.
    KRYS_NODISCARD const cq::ContainerQueryList &ContainerQueries() const noexcept
    {
      return _containerQueries;
    }
  };

  /// @brief [`@property`](https://drafts.css-houdini.org/css-properties-values-api/#at-property-rule).
  class PropertyRule final : public Rule
  {
  public:
    /// @brief The rule's name and descriptors; each descriptor is empty if it's missing.
    struct Descriptor
    {
      CSSOMStringAtom Name;
      CSSOMString Syntax;
      Maybe<bool> Inherits;
      RefPtr<const VariableData> InitialValue;
    };

  private:
    Descriptor _descriptor;

    explicit PropertyRule(Descriptor descriptor) noexcept;

    PropertyRule(const PropertyRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<PropertyRule> Create(Descriptor descriptor) noexcept;

    KRYS_NODISCARD Ref<PropertyRule> Copy() const noexcept;

    KRYS_NODISCARD const Descriptor &GetDescriptor() const noexcept
    {
      return _descriptor;
    }
  };

  /// @brief [`@scope`](https://drafts.csswg.org/css-cascade-6/#scoped-styles). Like a nesting rule, it keeps
  /// its scope selectors as written and, once a rule set has been built from the sheet, resolved against an
  /// enclosing style rule's.
  class ScopeRule final : public GroupRule
  {
  private:
    SelectorList _scopeStart;
    SelectorList _scopeEnd;
    SelectorList _originalScopeStart;
    SelectorList _originalScopeEnd;
    // Without a <scope-start>, the scope root is the parent element of the sheet's owner node.
    WeakPtr<const StyleSheetContents> _styleSheetOwner;

    ScopeRule(SelectorList scopeStart, SelectorList scopeEnd, List<Ref<Rule>> rules) noexcept;

    ScopeRule(const ScopeRule &other) noexcept;

  public:
    ~ScopeRule() noexcept;

    KRYS_NODISCARD static Ref<ScopeRule> Create(SelectorList scopeStart, SelectorList scopeEnd,
                                                List<Ref<Rule>> rules) noexcept;

    KRYS_NODISCARD Ref<ScopeRule> Copy() const noexcept;

    /// @brief The resolved `<scope-start>` selectors.
    KRYS_NODISCARD const SelectorList &ScopeStart() const noexcept
    {
      return _scopeStart;
    }

    /// @brief The resolved `<scope-end>` selectors.
    KRYS_NODISCARD const SelectorList &ScopeEnd() const noexcept
    {
      return _scopeEnd;
    }

    /// @brief The `<scope-start>` selectors as written.
    KRYS_NODISCARD const SelectorList &OriginalScopeStart() const noexcept
    {
      return _originalScopeStart;
    }

    /// @brief The `<scope-end>` selectors as written.
    KRYS_NODISCARD const SelectorList &OriginalScopeEnd() const noexcept
    {
      return _originalScopeEnd;
    }

    void SetScopeStart(SelectorList scopeStart) noexcept
    {
      _scopeStart = krys::move(scopeStart);
    }

    void SetScopeEnd(SelectorList scopeEnd) noexcept
    {
      _scopeEnd = krys::move(scopeEnd);
    }

    /// @brief The sheet the rule is in, whose owner node sets an implicit scope.
    KRYS_NODISCARD WeakPtr<const StyleSheetContents> GetStyleSheetContents() const noexcept;

    void SetStyleSheetContents(const StyleSheetContents &sheet) noexcept;
  };

  /// @brief [`@starting-style`](https://drafts.csswg.org/css-transitions-2/#defining-before-change-style).
  class StartingStyleRule final : public GroupRule
  {
  private:
    explicit StartingStyleRule(List<Ref<Rule>> rules) noexcept;

    StartingStyleRule(const StartingStyleRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<StartingStyleRule> Create(List<Ref<Rule>> rules) noexcept;

    KRYS_NODISCARD Ref<StartingStyleRule> Copy() const noexcept;
  };

  /// @brief [`@charset`](https://drafts.csswg.org/css-syntax-3/#charset-rule), which the parser accepts at
  /// the start of a sheet and then drops; it's never in a sheet's rules.
  class CharsetRule final : public Rule
  {
  private:
    CharsetRule() noexcept;

    CharsetRule(const CharsetRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<CharsetRule> Create() noexcept;

    KRYS_NODISCARD Ref<CharsetRule> Copy() const noexcept;
  };

  /// @brief [`@namespace`](https://drafts.csswg.org/css-namespaces/#declaration).
  class NamespaceRule final : public Rule
  {
  private:
    CSSOMStringAtom _prefix;
    CSSOMStringAtom _uri;

    NamespaceRule(CSSOMStringAtom prefix, CSSOMStringAtom uri) noexcept;

    NamespaceRule(const NamespaceRule &other) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<NamespaceRule> Create(CSSOMStringAtom prefix, CSSOMStringAtom uri) noexcept;

    KRYS_NODISCARD Ref<NamespaceRule> Copy() const noexcept;

    /// @brief The prefix, or null for the default namespace.
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
