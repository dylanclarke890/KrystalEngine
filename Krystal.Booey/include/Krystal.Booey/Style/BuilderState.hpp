#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <bitset>

namespace krys::boo
{
  class FontCascadeDescription;
  class StyleImage;
}

namespace krys::boo::css::calc
{
  struct RandomCachingKey;
}

namespace krys::boo::style
{
  class BuilderState;
  struct FontFamilies;
  struct FontFeatureSettings;
  struct FontPalette;
  struct FontSizeAdjust;
  struct FontStyle;
  struct FontVariantAlternates;
  struct FontVariantEastAsian;
  struct FontVariantLigatures;
  struct FontVariantNumeric;
  struct FontVariationSettings;
  struct FontWeight;
  struct FontWidth;
  struct TextAutospace;
  struct TextSpacingTrim;
  struct WebkitLocale;
  struct Zoom;

  void MaybeUpdateFontForLetterSpacingOrWordSpacing(BuilderState &, css::Value &) noexcept;

  enum class ApplyValueType : uint8
  {
    Value,
    Initial,
    Inherit
  };

  struct BuilderPositionTryFallback
  {
    RefPtr<const css::StyleProperties> Properties;
    List<PositionTryFallbackTactic> Tactics;
  };

  struct BuilderContext
  {
    RefPtr<const dom::Document> Document {};
    const RenderStyle *ParentStyle {};
    const RenderStyle *RootElementStyle {};
    RefPtr<const dom::Element> Element {};
    // FIXME: `treeResolutionState` (the anchor-positioned states) comes with the tree resolver.
    Maybe<BuilderPositionTryFallback> PositionTryFallback {};
  };

  class BuilderState : public CanMakeCheckedPtr<BuilderState>
  {
    // See the comment in maybeUpdateFontForLetterSpacingOrWordSpacing() about why this needs to be a friend.
    friend void MaybeUpdateFontForLetterSpacingOrWordSpacing(BuilderState &, css::Value &) noexcept;
    friend class Builder;
    KRYS_NON_COPY_MOVABLE(BuilderState);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(BuilderState);

  private:
    RenderStyle &_style;
    BuilderContext _context;

    const css::ToLengthConversionData _cssToLengthConversionData;

    HashSet<css::CSSOMStringAtom> _appliedCustomProperties;
    HashSet<css::CSSOMStringAtom> _inProgressCustomProperties;
    HashSet<css::CSSOMStringAtom> _inCycleCustomProperties;
    std::bitset<css::TotalPropertyIdValues> _inProgressProperties;
    std::bitset<css::TotalPropertyIdValues> _invalidAtComputedValueTimeProperties;

    const PropertyCascade::Property *_currentProperty {nullptr};
    css::SelectorChecker::LinkMatchMask _linkMatch {};

    bool _fontDirty {false};
    List<css::CSSOMStringAtom> _registeredContentAttributes;

    bool _isBuildingKeyframeStyle {false};

  public:
    explicit BuilderState(RenderStyle &) noexcept;
    BuilderState(RenderStyle &, BuilderContext &&) noexcept;
    ~BuilderState() noexcept;

    KRYS_NODISCARD RenderStyle &Style() noexcept
    {
      return _style;
    }

    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return _style;
    }

    KRYS_NODISCARD const RenderStyle &ParentStyle() const noexcept
    {
      return *_context.ParentStyle;
    }

    KRYS_NODISCARD const RenderStyle *RootElementStyle() const noexcept
    {
      return _context.RootElementStyle;
    }

    KRYS_NODISCARD const dom::Document &GetDocument() const noexcept
    {
      return *_context.Document;
    }

    KRYS_NODISCARD const dom::Element *GetElement() const noexcept
    {
      return _context.Element.get();
    }

    inline void SetZoom(Zoom) noexcept;
    inline void SetUsedZoom(float) noexcept;
    inline void SetWritingMode(StyleWritingMode) noexcept;
    inline void SetTextOrientation(TextOrientation) noexcept;

    KRYS_NODISCARD bool FontDirty() const noexcept
    {
      return _fontDirty;
    }

    void SetFontDirty() noexcept
    {
      _fontDirty = true;
    }

    KRYS_NODISCARD inline const FontCascadeDescription &GetFontDescription() const noexcept;
    KRYS_NODISCARD inline const FontCascadeDescription &ParentFontDescription() const noexcept;

    KRYS_NODISCARD bool ApplyPropertyToRegularStyle() const noexcept
    {
      return _linkMatch != css::SelectorChecker::MatchVisited;
    }

    KRYS_NODISCARD bool ApplyPropertyToVisitedLinkStyle() const noexcept
    {
      return _linkMatch != css::SelectorChecker::MatchLink;
    }

    KRYS_NODISCARD float ZoomWithTextZoomFactor() noexcept;

    KRYS_NODISCARD bool UseSVGZoomRules() const noexcept;
    KRYS_NODISCARD bool UseSVGZoomRulesForLength() const noexcept;

    KRYS_NODISCARD style::ScopeOrdinal StyleScopeOrdinal() const noexcept
    {
      return _currentProperty->StyleScopeOrdinal;
    }

    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const css::Value &) const noexcept;

    KRYS_NODISCARD const List<css::CSSOMStringAtom> &RegisteredContentAttributes() const noexcept
    {
      return _registeredContentAttributes;
    }

    void RegisterContentAttribute(const css::CSSOMStringAtom &attributeLocalName) noexcept;

    KRYS_NODISCARD const css::ToLengthConversionData &CssToLengthConversionData() const noexcept
    {
      return _cssToLengthConversionData;
    }

    void SetIsBuildingKeyframeStyle() noexcept
    {
      _isBuildingKeyframeStyle = true;
    }

    KRYS_NODISCARD bool IsAuthorOrigin() const noexcept
    {
      return _currentProperty && _currentProperty->Origin == PropertyCascade::Origin::Author;
    }

    KRYS_NODISCARD css::PropertyId CssPropertyId() const noexcept;

    KRYS_NODISCARD bool IsCurrentPropertyInvalidAtComputedValueTime() const noexcept;
    void SetCurrentPropertyInvalidAtComputedValueTime() noexcept;

    void SetUsesViewportUnits() noexcept;
    void SetUsesContainerUnits() noexcept;

    KRYS_NODISCARD double LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &,
                                                   Maybe<css::keywords::ElementShared>) const noexcept;

    // Accessors for sibling information used by the sibling-count() and sibling-index() CSS functions.
    KRYS_NODISCARD uint32 SiblingCount() noexcept;
    KRYS_NODISCARD uint32 SiblingIndex() noexcept;

    // FIXME: `anchorPositionedStates()` comes with the tree resolver.
    KRYS_NODISCARD const Maybe<BuilderPositionTryFallback> &GetPositionTryFallback() const noexcept
    {
      return _context.PositionTryFallback;
    }

    // FIXME: Copying a FontCascadeDescription is really inefficient. Migrate all callers to
    // setFontDescriptionXXX() variants below, then remove these functions.
    inline void SetFontDescription(FontCascadeDescription &&) noexcept;
    void SetFontSize(FontCascadeDescription &, float size) noexcept;

    inline void SetFontDescriptionKeywordSizeFromIdentifier(css::ValueId) noexcept;
    inline void SetFontDescriptionIsAbsoluteSize(bool) noexcept;
    inline void SetFontDescriptionFontSize(float) noexcept;
    inline void SetFontDescriptionFamilies(FontFamilies &&) noexcept;
    inline void SetFontDescriptionFeatureSettings(FontFeatureSettings &&) noexcept;
    inline void SetFontDescriptionFontPalette(FontPalette &&) noexcept;
    inline void SetFontDescriptionFontSizeAdjust(FontSizeAdjust) noexcept;
    inline void SetFontDescriptionFontSmoothing(FontSmoothingMode) noexcept;
    inline void SetFontDescriptionFontStyle(FontStyle) noexcept;
    inline void SetFontDescriptionFontSynthesisSmallCaps(FontSynthesisLonghandValue) noexcept;
    inline void SetFontDescriptionFontSynthesisStyle(FontSynthesisLonghandValue) noexcept;
    inline void SetFontDescriptionFontSynthesisWeight(FontSynthesisLonghandValue) noexcept;
    inline void SetFontDescriptionKerning(Kerning) noexcept;
    inline void SetFontDescriptionOpticalSizing(FontOpticalSizing) noexcept;
    inline void SetFontDescriptionSpecifiedLocale(WebkitLocale &&) noexcept;
    inline void SetFontDescriptionTextAutospace(TextAutospace) noexcept;
    inline void SetFontDescriptionTextRenderingMode(TextRenderingMode) noexcept;
    inline void SetFontDescriptionTextSpacingTrim(TextSpacingTrim) noexcept;
    inline void SetFontDescriptionVariantCaps(FontVariantCaps) noexcept;
    inline void SetFontDescriptionVariantEmoji(FontVariantEmoji) noexcept;
    inline void SetFontDescriptionVariantPosition(FontVariantPosition) noexcept;
    inline void SetFontDescriptionVariationSettings(FontVariationSettings &&) noexcept;
    inline void SetFontDescriptionWeight(FontWeight) noexcept;
    inline void SetFontDescriptionWidth(FontWidth) noexcept;
    inline void SetFontDescriptionVariantAlternates(FontVariantAlternates &&) noexcept;
    inline void SetFontDescriptionVariantEastAsian(FontVariantEastAsian) noexcept;
    inline void SetFontDescriptionVariantEastAsianVariant(FontVariantEastAsianVariant) noexcept;
    inline void SetFontDescriptionVariantEastAsianWidth(FontVariantEastAsianWidth) noexcept;
    inline void SetFontDescriptionVariantEastAsianRuby(FontVariantEastAsianRuby) noexcept;
    inline void SetFontDescriptionKeywordSize(uint32) noexcept;
    inline void SetFontDescriptionVariantLigatures(FontVariantLigatures) noexcept;
    inline void SetFontDescriptionVariantCommonLigatures(boo::FontVariantLigatures) noexcept;
    inline void SetFontDescriptionVariantDiscretionaryLigatures(boo::FontVariantLigatures) noexcept;
    inline void SetFontDescriptionVariantHistoricalLigatures(boo::FontVariantLigatures) noexcept;
    inline void SetFontDescriptionVariantContextualAlternates(boo::FontVariantLigatures) noexcept;
    inline void SetFontDescriptionVariantNumeric(FontVariantNumeric) noexcept;
    inline void SetFontDescriptionVariantNumericFigure(FontVariantNumericFigure) noexcept;
    inline void SetFontDescriptionVariantNumericSpacing(FontVariantNumericSpacing) noexcept;
    inline void SetFontDescriptionVariantNumericFraction(FontVariantNumericFraction) noexcept;
    inline void SetFontDescriptionVariantNumericOrdinal(FontVariantNumericOrdinal) noexcept;
    inline void SetFontDescriptionVariantNumericSlashedZero(FontVariantNumericSlashedZero) noexcept;

    void DisableNativeAppearanceIfNeeded(css::PropertyId, PropertyCascade::Origin) noexcept;

  private:
    void AdjustStyleForInterCharacterRuby() noexcept;

    void UpdateFont() noexcept;
    void UpdateFontForZoomChange() noexcept;
    void UpdateFontForGenericFamilyChange() noexcept;
    void UpdateFontForOrientationChange() noexcept;
    void UpdateFontForSizeChange() noexcept;
  };
}
