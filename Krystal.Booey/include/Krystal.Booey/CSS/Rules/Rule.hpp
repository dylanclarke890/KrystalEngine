#pragma once

// #include <WebCore/ContainerQuery.h>
// #include <WebCore/CSSVariableData.h>
// #include <WebCore/FontFeatureValues.h>
// #include <WebCore/FontPaletteValues.h>

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <map>

namespace krys::boo::css
{
  class CSSRule;
  class CSSGroupingRule;
  class CSSStyleRule;
  class CSSStyleSheet;
  class MutableStyleProperties;
  class StyleRuleKeyframe;
  class StyleProperties;
  class StyleSheetContents;

  using CascadeLayerName = SmallList<CSSOMStringAtom>;

  class Rule : public RefCounted<Rule>
  {
  private:
    // RuleType
    unsigned _type : 5;
    // This is only needed to support GetMatchedCSSRules.
    unsigned _hasDocumentSecurityOrigin : 1;

  protected:
    explicit Rule(RuleType type, bool hasDocumentSecurityOrigin = false) noexcept
        : _type(static_cast<unsigned>(type)), _hasDocumentSecurityOrigin(hasDocumentSecurityOrigin)
    {
    }

    Rule(const Rule &o) noexcept
        : RefCounted(), _type(o._type), _hasDocumentSecurityOrigin(o._hasDocumentSecurityOrigin)
    {
    }

  public:
    KRYS_NODISCARD RuleType Type() const noexcept
    {
      return static_cast<RuleType>(_type);
    }

    void operator delete(Rule *rule, std::destroying_delete_t) noexcept;

    KRYS_NODISCARD Ref<Rule> Copy() const noexcept;

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

    KRYS_NODISCARD bool IsRuleWithNesting() const noexcept
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
             || Type() == RuleType::StartingStyle || Type() == RuleType::Function
             || Type() == RuleType::InternalBaseAppearance;
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

    KRYS_NODISCARD bool IsInternalBaseAppearanceRule() const noexcept
    {
      return Type() == RuleType::InternalBaseAppearance;
    }

#pragma endregion

    // This is only needed to support GetMatchedCSSRules.
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper() const noexcept;

    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleSheet &parentSheet) const noexcept;

    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSGroupingRule &parentRule) const noexcept;

    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleRule &parentRule) const noexcept;

    KRYS_NODISCARD CSSOMString DebugDescription() const noexcept;

  protected:
    void SetType(RuleType type) noexcept
    {
      _type = static_cast<unsigned>(type);
    }

    KRYS_NODISCARD bool HasDocumentSecurityOrigin() const noexcept
    {
      return _hasDocumentSecurityOrigin;
    }

    void InvalidateResolvedSelectorListRecursively() noexcept;

  private:
    KRYS_NODISCARD Ref<CSSRule> CreateCSSOMWrapper(CSSStyleSheet *parentSheet,
                                                   CSSRule *parentRule) const noexcept;

    template <typename Visitor>
    KRYS_NODISCARD constexpr decltype(auto) VisitDerived(Visitor &&) noexcept;

    template <typename Visitor>
    KRYS_NODISCARD constexpr decltype(auto) VisitDerived(Visitor &&) const noexcept;
  };

  class StyleRule : public Rule
  {
  private:
    bool _isSplitRule {false};
    bool _isLastRuleInSplitRule {false};

    mutable Ref<StyleProperties> _properties;
    SelectorList _selectors;

  protected:
    StyleRule(Ref<StyleProperties> &&properties, bool hasDocumentSecurityOrigin,
              SelectorList &&selectors) noexcept;

    StyleRule(const StyleRule &rule) noexcept;

  public:
    ~StyleRule() noexcept;

    KRYS_NODISCARD static Ref<StyleRule> Create(Ref<StyleProperties> &&properties,
                                                bool hasDocumentSecurityOrigin,
                                                SelectorList &&selectors) noexcept;

    KRYS_NODISCARD Ref<StyleRule> Copy() const noexcept;

    KRYS_NODISCARD const SelectorList &Selectors() const noexcept
    {
      return _selectors;
    }

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return _properties.get();
    }

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

    KRYS_NODISCARD SmallList<Ref<StyleRule>>
      SplitIntoMultipleRulesWithMaximumSelectorComponentCount(size_t) const noexcept;

    void AdoptSelectorList(SelectorList &&selectors) noexcept
    {
      _selectors = krys::move(selectors);
    }

    void SetProperties(Ref<StyleProperties> &&properties) noexcept;

    KRYS_NODISCARD CSSOMString DebugDescription() const noexcept;

    KRYS_NODISCARD static size_t AverageSizeInBytes() noexcept;

  private:
    KRYS_NODISCARD static Ref<StyleRule> CreateForSplitting(const SmallList<const Selector *> &selectors,
                                                            Ref<StyleProperties> &&properties,
                                                            bool hasDocumentSecurityOrigin) noexcept;
  };

  class RuleWithNesting final : public StyleRule
  {
  private:
    SmallList<Ref<Rule>> _nestedRules;
    SelectorList _originalSelectors;

    RuleWithNesting(Ref<StyleProperties> &&properties, bool hasDocumentSecurityOrigin,
                    SelectorList &&selectors, SmallList<Ref<Rule>> &&nestedRules) noexcept;

    RuleWithNesting(StyleRule &&rule) noexcept;

  protected:
    RuleWithNesting(const RuleWithNesting &o) noexcept;

  public:
    ~RuleWithNesting();

    KRYS_NODISCARD Ref<RuleWithNesting> Copy() const noexcept;

    KRYS_NODISCARD static Ref<RuleWithNesting> Create(Ref<StyleProperties> &&properties,
                                                      bool hasDocumentSecurityOrigin,
                                                      SelectorList &&selectors,
                                                      SmallList<Ref<Rule>> &&nestedRules) noexcept;

    KRYS_NODISCARD static Ref<RuleWithNesting> Create(StyleRule &&rule) noexcept;

    KRYS_NODISCARD const SmallList<Ref<Rule>> &NestedRules() const noexcept
    {
      return _nestedRules;
    }

    KRYS_NODISCARD SmallList<Ref<Rule>> &NestedRules() noexcept
    {
      return _nestedRules;
    }

    KRYS_NODISCARD const SelectorList &OriginalSelectorList() const noexcept
    {
      return _originalSelectors;
    }

    // Used by CSSOM.
    void WrapperAdoptOriginalSelectorList(SelectorList &&) noexcept;

    KRYS_NODISCARD CSSOMString DebugDescription() const noexcept;
  };

  class NestedDeclarationsRule final : public StyleRule
  {
  private:
    explicit NestedDeclarationsRule(Ref<StyleProperties> &&properties) noexcept;

    NestedDeclarationsRule(const NestedDeclarationsRule &o) noexcept = default;

  public:
    KRYS_NODISCARD Ref<NestedDeclarationsRule> Copy() const noexcept
    {
      return AdoptRef(*new NestedDeclarationsRule(*this));
    }

    KRYS_NODISCARD static Ref<NestedDeclarationsRule> Create(Ref<StyleProperties> &&properties) noexcept
    {
      return AdoptRef(*new NestedDeclarationsRule(::krys::move(properties)));
    }

    KRYS_NODISCARD CSSOMString DebugDescription() const noexcept;
  };

  class FontFaceRule final : public Rule
  {
  private:
    Ref<StyleProperties> _properties;

  private:
    explicit FontFaceRule(Ref<StyleProperties> &&) noexcept;

    FontFaceRule(const FontFaceRule &) noexcept;

  public:
    KRYS_NODISCARD static Ref<FontFaceRule> Create(Ref<StyleProperties> &&properties) noexcept
    {
      return AdoptRef(*new FontFaceRule(::krys::move(properties)));
    }

    ~FontFaceRule() noexcept;

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return _properties;
    }

    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept;

    KRYS_NODISCARD Ref<FontFaceRule> Copy() const noexcept
    {
      return AdoptRef(*new FontFaceRule(*this));
    }
  };

  class FontPaletteValuesRule final : public Rule
  {
  private:
    CSSOMStringAtom _name;
    SmallList<CSSOMStringAtom> _fontFamilies;
    FontPaletteValues _fontPaletteValues;

    FontPaletteValuesRule(const CSSOMStringAtom &name, SmallList<CSSOMStringAtom> &&fontFamilies,
                          Maybe<FontPaletteIndex> basePalette,
                          SmallList<FontPaletteValues::OverriddenColor> &&overrideColors) noexcept;

    FontPaletteValuesRule(const FontPaletteValuesRule &o) noexcept = default;

  public:
    KRYS_NODISCARD Ref<FontPaletteValuesRule> Copy() const noexcept
    {
      return AdoptRef(*new FontPaletteValuesRule(*this));
    }

    KRYS_NODISCARD static Ref<FontPaletteValuesRule>
      Create(const CSSOMStringAtom &name, SmallList<CSSOMStringAtom> &&fontFamilies,
             Maybe<FontPaletteIndex> basePalette,
             SmallList<FontPaletteValues::OverriddenColor> &&overrideColors) noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const SmallList<CSSOMStringAtom> &FontFamilies() const noexcept
    {
      return _fontFamilies;
    }

    KRYS_NODISCARD const FontPaletteValues &FontPaletteValues() const noexcept
    {
      return _fontPaletteValues;
    }

    KRYS_NODISCARD Maybe<FontPaletteIndex> BasePalette() const noexcept
    {
      return _fontPaletteValues.basePalette();
    }

    KRYS_NODISCARD const SmallList<FontPaletteValues::OverriddenColor> &OverrideColors() const noexcept
    {
      return _fontPaletteValues.overrideColors();
    }
  };

  class FontFeaturesValuesBlockRule final : public Rule
  {
  private:
    FontFeatureValuesType _type;
    SmallList<FontFeatureValuesTag> _tags;

    FontFeaturesValuesBlockRule(FontFeatureValuesType type,
                                const SmallList<FontFeatureValuesTag> &tags) noexcept;
    FontFeaturesValuesBlockRule(const FontFeaturesValuesBlockRule &o) noexcept = default;

  public:
    KRYS_NODISCARD Ref<FontFeaturesValuesBlockRule> Copy() const noexcept
    {
      return AdoptRef(*new FontFeaturesValuesBlockRule(*this));
    }

    KRYS_NODISCARD static Ref<FontFeaturesValuesBlockRule>
      Create(FontFeatureValuesType type, const SmallList<FontFeatureValuesTag> &tags) noexcept
    {
      return AdoptRef(*new FontFeaturesValuesBlockRule(type, tags));
    }

    KRYS_NODISCARD FontFeatureValuesType FontFeatureValuesType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD const SmallList<FontFeatureValuesTag> &Tags() const noexcept
    {
      return _tags;
    }
  };

  class FontFeaturesValuesRule final : public Rule
  {
  private:
    SmallList<CSSOMStringAtom> _fontFamilies;
    Ref<FontFeatureValues> _value;

    FontFeaturesValuesRule(const SmallList<CSSOMStringAtom> &, Ref<FontFeatureValues> &&) noexcept;
    FontFeaturesValuesRule(const FontFeaturesValuesRule &) noexcept = default;

  public:
    KRYS_NODISCARD Ref<FontFeaturesValuesRule> Copy() const noexcept
    {
      return AdoptRef(*new FontFeaturesValuesRule(*this));
    }

    KRYS_NODISCARD static Ref<FontFeaturesValuesRule> Create(const SmallList<CSSOMStringAtom> &fontFamilies,
                                                             Ref<FontFeatureValues> &&) noexcept;

    KRYS_NODISCARD const SmallList<CSSOMStringAtom> &FontFamilies() const noexcept
    {
      return _fontFamilies;
    }

    KRYS_NODISCARD Ref<FontFeatureValues> Value() const noexcept
    {
      return _value;
    }
  };

  class PageRule final : public Rule
  {
  private:
    Ref<StyleProperties> _properties;
    SelectorList _selectors;

    explicit PageRule(Ref<StyleProperties> &&properties, SelectorList &&selectors) noexcept;
    PageRule(const PageRule &o) noexcept;

  public:
    ~PageRule() noexcept;

    KRYS_NODISCARD Ref<PageRule> Copy() const noexcept
    {
      return AdoptRef(*new PageRule(*this));
    }

    KRYS_NODISCARD static Ref<PageRule> Create(Ref<StyleProperties> &&properties,
                                               SelectorList &&selectors) noexcept;

    KRYS_NODISCARD const Selector *Selector() const noexcept
    {
      return _selectors.FirstSelectorComponent();
    }

    KRYS_NODISCARD const StyleProperties &Properties() const noexcept
    {
      return _properties;
    }

    KRYS_NODISCARD MutableStyleProperties &MutableProperties() noexcept
    {
      return _properties;
    }

    void WrapperAdoptSelectorList(SelectorList &&selectors) noexcept
    {
      _selectors = krys::move(selectors);
    }
  };

  class GroupRule : public Rule
  {
    friend class CSSGroupingRule;
    friend class CSSStyleSheet;

  private:
    mutable SmallList<Ref<Rule>> _childRules;

  protected:
    GroupRule(RuleType type, SmallList<Ref<Rule>> &&childRules) noexcept;

    GroupRule(const GroupRule &o) noexcept;

  public:
    KRYS_NODISCARD const SmallList<Ref<Rule>> &ChildRules() const noexcept;

    void WrapperInsertRule(size_t index, Ref<Rule> &&rule) noexcept;

    void WrapperRemoveRule(size_t index) noexcept;

    KRYS_NODISCARD CSSOMString DebugDescription() const noexcept;
  };

  class MediaRule final : public GroupRule
  {
  private:
    mq::MediaQueryList _mediaQueries;

    MediaRule(mq::MediaQueryList &&mediaQueries, SmallList<Ref<Rule>> &&childRules) noexcept;

    MediaRule(const MediaRule &) noexcept;

  public:
    KRYS_NODISCARD Ref<MediaRule> Copy() const noexcept;

    KRYS_NODISCARD static Ref<MediaRule> Create(mq::MediaQueryList &&mediaQueries,
                                                SmallList<Ref<Rule>> &&childRules) noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept
    {
      return _mediaQueries;
    }

    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept
    {
      _mediaQueries = krys::move(queries);
    }

    KRYS_NODISCARD CSSOMString DebugDescription() const noexcept;
  };

  class SupportsRule final : public GroupRule
  {
  private:
    CSSOMString _conditionText;
    bool _conditionIsSupported;

    SupportsRule(const CSSOMString &conditionText, bool conditionIsSupported,
                 SmallList<Ref<Rule>> &&rules) noexcept;

    SupportsRule(const SupportsRule &o) noexcept = default;

  public:
    KRYS_NODISCARD Ref<SupportsRule> Copy() const noexcept
    {
      return AdoptRef(*new SupportsRule(*this));
    }

    KRYS_NODISCARD static Ref<SupportsRule> Create(const CSSOMString &conditionText,
                                                   bool conditionIsSupported,
                                                   SmallList<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD CSSOMString ConditionText() const noexcept
    {
      return _conditionText;
    }

    KRYS_NODISCARD bool ConditionIsSupported() const noexcept
    {
      return _conditionIsSupported;
    }
  };

  class LayerRule final : public GroupRule
  {
  private:
    Variant<CascadeLayerName, SmallList<CascadeLayerName>> _name;

    LayerRule(SmallList<CascadeLayerName> &&names) noexcept;
    LayerRule(CascadeLayerName &&name, SmallList<Ref<Rule>> &&rules) noexcept;
    LayerRule(const LayerRule &o) noexcept = default;

  public:
    KRYS_NODISCARD Ref<LayerRule> Copy() const noexcept
    {
      return AdoptRef(*new LayerRule(*this));
    }

    KRYS_NODISCARD static Ref<LayerRule> CreateStatement(SmallList<CascadeLayerName> &&names) noexcept;

    KRYS_NODISCARD static Ref<LayerRule> CreateBlock(CascadeLayerName &&name,
                                                     SmallList<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD bool IsStatement() const noexcept
    {
      return Type() == RuleType::LayerStatement;
    }

    KRYS_NODISCARD const CascadeLayerName &Name() const noexcept
    {
      return std::get<CascadeLayerName>(_name);
    }

    KRYS_NODISCARD const SmallList<CascadeLayerName> &NameList() const noexcept
    {
      return std::get<SmallList<CascadeLayerName>>(_name);
    }
  };

  class ContainerRule final : public GroupRule
  {
  private:
    ContainerRule(cq::ContainerQuery &&query, SmallList<Ref<Rule>> &&rules) noexcept;
    ContainerRule(const ContainerRule &o) noexcept = default;

    cq::ContainerQuery _containerQuery;

  public:
    KRYS_NODISCARD Ref<ContainerRule> Copy() const noexcept
    {
      return AdoptRef(*new ContainerRule(*this));
    }

    KRYS_NODISCARD static Ref<ContainerRule> Create(cq::ContainerQuery &&query, SmallList<Ref<Rule>> &&rules) noexcept;

    KRYS_NODISCARD const cq::ContainerQuery &containerQuery() const noexcept
    {
      return _containerQuery;
    }
  };

  class PropertyRule final : public Rule
  {
  public:
    struct RuleDescriptor
    {
      CSSOMStringAtom name;
      CSSOMString syntax {};
      Maybe<bool> inherits {};
      RefPtr<const VariableData> initialValue {};
    };

  private:
    RuleDescriptor _descriptor;

    PropertyRule(RuleDescriptor &&) noexcept;
    PropertyRule(const PropertyRule &) noexcept = default;

  public:
    KRYS_NODISCARD Ref<PropertyRule> Copy() const noexcept
    {
      return AdoptRef(*new PropertyRule(*this));
    }

    KRYS_NODISCARD static Ref<PropertyRule> Create(RuleDescriptor &&) noexcept;

    KRYS_NODISCARD const RuleDescriptor &Descriptor() const noexcept
    {
      return _descriptor;
    }
  };

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

    ScopeRule(SelectorList &&, SelectorList &&, SmallList<Ref<Rule>> &&) noexcept;

    ScopeRule(const ScopeRule &o) noexcept;

  public:
    ~ScopeRule() noexcept;

    KRYS_NODISCARD Ref<ScopeRule> Copy() const noexcept;

    KRYS_NODISCARD static Ref<ScopeRule> Create(SelectorList &&, SelectorList &&,
                                                SmallList<Ref<Rule>> &&) noexcept;

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

    WeakPtr<const StyleSheetContents> GetStyleSheetContents() const noexcept;

    void SetStyleSheetContents(const StyleSheetContents &contents) noexcept;
  };

  class StartingStyleRule final : public GroupRule
  {
  private:
    StartingStyleRule(SmallList<Ref<Rule>> &&rules) noexcept;

    StartingStyleRule(const StartingStyleRule &o) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<StartingStyleRule> Create(SmallList<Ref<Rule>> &&rules) noexcept;

    Ref<StartingStyleRule> Copy() const noexcept
    {
      return AdoptRef(*new StartingStyleRule(*this));
    }
  };

  // This is only used by the CSS parser.
  class CharsetRule final : public Rule
  {
  private:
    CharsetRule() noexcept;
    CharsetRule(const CharsetRule &) noexcept = default;

  public:
    KRYS_NODISCARD static Ref<CharsetRule> Create() noexcept
    {
      return AdoptRef(*new CharsetRule);
    }

    KRYS_NODISCARD Ref<CharsetRule> Copy() const noexcept
    {
      return AdoptRef(*new CharsetRule(*this));
    }
  };

  class InternalBaseAppearanceRule final : public GroupRule
  {
  private:
    InternalBaseAppearanceRule(SmallList<Ref<Rule>> &&rules) noexcept;
    InternalBaseAppearanceRule(const InternalBaseAppearanceRule &o) noexcept = default;

  public:
    KRYS_NODISCARD Ref<InternalBaseAppearanceRule> Copy() const noexcept
    {
      return AdoptRef(*new InternalBaseAppearanceRule(*this));
    }

    KRYS_NODISCARD static Ref<InternalBaseAppearanceRule> Create(SmallList<Ref<Rule>> &&rules) noexcept;
  };

  class NamespaceRule final : public Rule
  {
  private:
    CSSOMStringAtom _prefix;
    CSSOMStringAtom _uri;

    NamespaceRule(const CSSOMStringAtom &prefix, const CSSOMStringAtom &uri) noexcept;

    NamespaceRule(const NamespaceRule &o) noexcept = default;

  public:
    KRYS_NODISCARD Ref<NamespaceRule> Copy() const noexcept
    {
      return AdoptRef(*new NamespaceRule(*this));
    }

    KRYS_NODISCARD static Ref<NamespaceRule> Create(const CSSOMStringAtom &prefix,
                                                    const CSSOMStringAtom &uri) noexcept;

    KRYS_NODISCARD CSSOMStringAtom Prefix() const noexcept
    {
      return _prefix;
    }

    KRYS_NODISCARD CSSOMStringAtom Uri() const noexcept
    {
      return _uri;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::StyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::RuleWithNesting)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsRuleWithNesting();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::NestedDeclarationsRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsNestedDeclarationsRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GroupRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsGroupRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFaceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsFontFaceRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFeaturesValuesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsFontFeatureValuesRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFeaturesValuesBlockRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsFontFeatureValuesBlockRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontPaletteValuesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsFontPaletteValuesRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::MediaRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsMediaRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PageRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsPageRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::SupportsRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsSupportsRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::NamespaceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsNamespaceRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::StyleRuleKeyframe)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsKeyframeRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CharsetRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsCharsetRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::LayerRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsLayerRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ContainerRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsContainerRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::PropertyRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsPropertyRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ScopeRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsScopeRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::StartingStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsStartingStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::InternalBaseAppearanceRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule)
  {
    return rule.IsInternalBaseAppearanceRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
