#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Core/StyleAppearance.hpp"
#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/WindRule.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridAutoFlow.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumericForward.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Types/DataRef.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class Color;
  class Element;
  class FloatPoint;
  class FloatSize;
  class FloatPoint3D;
  class FloatRect;
  class FontCascade;
  class FontCascadeDescription;
  class FontMetrics;
  class FontSelectionValue;
  class IntSize;
  class LayoutRect;
  class LayoutSize;
  class LayoutUnit;
  class RenderElement;
  class RenderStyle;
  class RenderStyleBase;
  class RenderStyleProperties;
  class SVGRenderStyle;
  class ScrollTimeline;
  class StyleInheritedData;
  class StyleNonInheritedData;
  class StyleRareInheritedData;
  class TransformationMatrix;
  class ViewTimeline;

  struct BorderData;
  struct BorderValue;
  struct CSSPropertiesBitSet;
  struct CounterDirectiveMap;
  struct GridTrackList;
  struct OutlineValue;
  struct TransformOperationData;

  template <typename>
  class RectEdges;
  template <typename>
  class RectCorners;

  using IntOutsets = RectEdges<int>;

  namespace style
  {
    class ChangedAnimatablePropertiesFunctions;
    class CustomProperty;
    class CustomPropertyData;
    class CustomPropertyRegistry;

    struct AccentColor;
    struct AlignContent;
    struct AlignItems;
    struct AlignSelf;
    struct Animation;
    struct AnchorNames;
    struct AppleColorFilter;
    struct AspectRatio;
    struct BackgroundLayer;
    struct BackgroundSize;
    struct BlockEllipsis;
    struct BlockStepSize;
    struct BorderImage;
    struct BorderImageOutset;
    struct BorderImageRepeat;
    struct BorderImageSlice;
    struct BorderImageSource;
    struct BorderImageWidth;
    struct BorderRadius;
    struct BoxShadow;
    struct Clip;
    struct ClipPath;
    struct Color;
    struct ColorScheme;
    struct ColumnCount;
    struct ColumnWidth;
    struct Contain;
    struct ContainIntrinsicSize;
    struct ContainerNames;
    struct Content;
    struct CornerShapeValue;
    struct Cursor;
    struct DynamicRangeLimit;
    struct Filter;
    struct FlexBasis;
    struct FontFamilies;
    struct FontFamiliesView;
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
    struct GapGutter;
    struct GridPosition;
    struct GridTemplateAreas;
    struct GridTemplateList;
    struct GridTrackSizes;
    struct HangingPunctuation;
    struct HyphenateCharacter;
    struct HyphenateLimitEdge;
    struct HyphenateLimitLines;
    struct ImageOrNone;
    struct InsetEdge;
    struct ItemTolerance;
    struct JustifyContent;
    struct JustifyItems;
    struct JustifySelf;
    struct LetterSpacing;
    struct LineHeight;
    struct LineWidth;
    struct LineFitEdge;
    struct ListStyleType;
    struct MarginEdge;
    struct MarginTrim;
    struct MaskBorder;
    struct MaskBorderOutset;
    struct MaskBorderRepeat;
    struct MaskBorderSlice;
    struct MaskBorderSource;
    struct MaskBorderWidth;
    struct MaskLayer;
    struct MathDepth;
    struct MaximumLines;
    struct MaximumSize;
    struct MinimumSize;
    struct NameScope;
    struct OffsetAnchor;
    struct OffsetDistance;
    struct OffsetPath;
    struct OffsetPosition;
    struct OffsetRotate;
    struct Opacity;
    struct Orphans;
    struct PaddingEdge;
    struct PageSize;
    struct Perspective;
    struct Position;
    struct PositionAnchor;
    struct PositionArea;
    struct PositionVisibility;
    struct PositionX;
    struct PositionY;
    struct PositionTryFallbacks;
    struct PreferredSize;
    struct ProgressTimelineAxes;
    struct ProgressTimelineNames;
    struct Quotes;
    struct RepeatStyle;
    struct Rotate;
    struct SVGBaselineShift;
    struct SVGCenterCoordinateComponent;
    struct SVGCoordinateComponent;
    struct SVGMarkerResource;
    struct SVGPaint;
    struct SVGPaintOrder;
    struct SVGPathData;
    struct SVGRadius;
    struct SVGRadiusComponent;
    struct SVGStrokeDasharray;
    struct SVGStrokeDashoffset;
    struct Scale;
    struct ScopedName;
    struct ScrollMarginEdge;
    struct ScrollPaddingEdge;
    struct ScrollSnapAlign;
    struct ScrollSnapType;
    struct ScrollTimelines;
    struct ScrollbarColor;
    struct ScrollbarGutter;
    struct ShapeMargin;
    struct ShapeOutside;
    struct SpeakAs;
    struct StrokeMiterlimit;
    struct StrokeWidth;
    struct TabSize;
    struct TextAutospace;
    struct TextBoxEdge;
    struct TextDecorationLine;
    struct TextDecorationThickness;
    struct TextEmphasisPosition;
    struct TextEmphasisStyle;
    struct TextIndent;
    struct TextShadow;
    struct TextSizeAdjust;
    struct TextSpacingTrim;
    struct TextTransform;
    struct TextUnderlineOffset;
    struct TextUnderlinePosition;
    struct TouchAction;
    struct Transform;
    struct TransformOrigin;
    struct Transition;
    struct Translate;
    struct VerticalAlign;
    struct ViewTimelineInsets;
    struct ViewTimelines;
    struct ViewTransitionClasses;
    struct ViewTransitionName;
    struct WebkitInitialLetter;
    struct WebkitLineBoxContain;
    struct WebkitLocale;
    struct Widows;
    struct WillChange;
    struct WordSpacing;
    struct ZIndex;
    struct Zoom;
    struct ZoomFactor;

    enum class Change : uint8;
    enum class GridTrackSizingDirection : bool;
    enum class ImageOrientation : bool;
    enum class PositionTryOrder : uint8;
    enum class Resize : uint8;
    enum class SVGGlyphOrientationHorizontal : uint8;
    enum class SVGGlyphOrientationVertical : uint8;
    enum class ScrollBehavior : bool;
    enum class ScrollbarWidth : uint8;
    enum class TextAlignLast : uint8;
    enum class TextAlign : uint8;

    template <typename>
    struct CoordinatedValueList;
    template <typename>
    struct Shadows;

    using Animations = CoordinatedValueList<Animation>;
    using BackgroundLayers = CoordinatedValueList<BackgroundLayer>;
    using BorderRadiusValue = css::MinimallySerialisingSpaceSeparatedSize<LengthPercentage<css::NonNegative>>;
    using BoxShadows = Shadows<BoxShadow>;
    using FlexGrow = Number<css::NonNegative, float>;
    using FlexShrink = Number<css::NonNegative, float>;
    using InsetBox = css::MinimallySerialisingSpaceSeparatedRectEdges<InsetEdge>;
    using LineWidthBox = css::MinimallySerialisingSpaceSeparatedRectEdges<LineWidth>;
    using MarginBox = css::MinimallySerialisingSpaceSeparatedRectEdges<MarginEdge>;
    using MaskLayers = CoordinatedValueList<MaskLayer>;
    using ObjectPosition = Position;
    using Order = Integer<>;
    using PaddingBox = css::MinimallySerialisingSpaceSeparatedRectEdges<PaddingEdge>;
    using PerspectiveOrigin = Position;
    using PerspectiveOriginX = PositionX;
    using PerspectiveOriginY = PositionY;
    using ScrollMarginBox = css::MinimallySerialisingSpaceSeparatedRectEdges<ScrollMarginEdge>;
    using ScrollPaddingBox = css::MinimallySerialisingSpaceSeparatedRectEdges<ScrollPaddingEdge>;
    using ShapeImageThreshold = Number<css::ClosedUnitRangeClampBoth, float>;
    using TextShadows = Shadows<TextShadow>;
    using TransformOriginX = PositionX;
    using TransformOriginXY = Position;
    using TransformOriginY = PositionY;
    using TransformOriginZ = Length<>;
    using Transitions = CoordinatedValueList<Transition>;
    using WebkitBorderSpacing = Length<css::NonNegativeUnzoomed>;

    constexpr auto PublicPseudoIDBits = 17;
    constexpr auto TextDecorationLineBits = 5;
    constexpr auto TextTransformBits = 6;
    constexpr auto PseudoElementTypeBits = 5;
    struct PseudoStyleCache
    {
      struct IdentifierHash
      {
        KRYS_NODISCARD size_t operator()(const PseudoElementIdentifier &identifier) const noexcept
        {
          return DefaultHash<PseudoElementIdentifier>::Hash(identifier);
        }
      };

      HashMap<PseudoElementIdentifier, UniquePtr<RenderStyle>, IdentifierHash> Styles;
    };
    class ComputedStyleBase : public CanMakeCheckedPtr<ComputedStyleBase>
    {
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ComputedStyleBase);

    public:
      enum CloneTag
      {
        Clone
      };
      enum CreateDefaultStyleTag
      {
        CreateDefaultStyle
      };

      ~ComputedStyleBase() noexcept;

#if KRYS_DEBUG
      KRYS_NODISCARD bool DeletionHasBegun() const noexcept
      {
        return _deletionHasBegun;
      }
#endif

      KRYS_NODISCARD inline bool UsesViewportUnits() const noexcept;
      inline void SetUsesViewportUnits() noexcept;

      KRYS_NODISCARD inline bool UsesContainerUnits() const noexcept;
      inline void SetUsesContainerUnits() noexcept;

      KRYS_NODISCARD inline bool UseTreeCountingFunctions() const noexcept;
      inline void SetUsesTreeCountingFunctions() noexcept;

      KRYS_NODISCARD inline InsideLink GetInsideLink() const noexcept;
      inline void SetInsideLink(InsideLink) noexcept;

      KRYS_NODISCARD inline bool IsLink() const noexcept;
      inline void SetIsLink(bool) noexcept;

      KRYS_NODISCARD inline bool EmptyState() const noexcept;
      inline void SetEmptyState(bool) noexcept;

      KRYS_NODISCARD inline bool FirstChildState() const noexcept;
      inline void SetFirstChildState() noexcept;

      KRYS_NODISCARD inline bool LastChildState() const noexcept;
      inline void SetLastChildState() noexcept;

      KRYS_NODISCARD inline bool HasExplicitlyInheritedProperties() const noexcept;
      inline void SetHasExplicitlyInheritedProperties() noexcept;

      KRYS_NODISCARD inline bool DisallowsFastPathInheritance() const noexcept;
      inline void SetDisallowsFastPathInheritance() noexcept;

      KRYS_NODISCARD inline bool HasDisplayAffectedByAnimations() const noexcept;
      inline void SetHasDisplayAffectedByAnimations() noexcept;

      KRYS_NODISCARD inline bool TransformStyleForcedToFlat() const noexcept;
      inline void SetTransformStyleForcedToFlat(bool) noexcept;

      inline void SetUsesAnchorFunctions() noexcept;
      KRYS_NODISCARD inline bool UsesAnchorFunctions() const noexcept;

      inline void SetAnchorFunctionScrollCompensatedAxes(EnumSet<BoxAxis>) noexcept;
      KRYS_NODISCARD inline EnumSet<BoxAxis> AnchorFunctionScrollCompensatedAxes() const noexcept;

      inline void SetIsPopoverInvoker() noexcept;
      KRYS_NODISCARD inline bool IsPopoverInvoker() const noexcept;

      KRYS_NODISCARD inline bool NativeAppearanceDisabled() const noexcept;
      inline void SetNativeAppearanceDisabled(bool) noexcept;

      KRYS_NODISCARD inline bool InsideDefaultButton() const noexcept;
      inline void SetInsideDefaultButton(bool) noexcept;

      KRYS_NODISCARD inline bool InsideSubmitButton() const noexcept;
      inline void SetInsideSubmitButton(bool) noexcept;

      KRYS_NODISCARD inline EnumSet<EventListenerRegionType> EventListenerRegionTypes() const noexcept;
      inline void SetEventListenerRegionTypes(EnumSet<EventListenerRegionType>) noexcept;

      // No setter. Set via `ComputedStyleProperties::setBlendMode()`.
      KRYS_NODISCARD inline bool IsInSubtreeWithBlendMode() const noexcept;

      KRYS_NODISCARD inline bool IsForceHidden() const noexcept;
      inline void SetIsForceHidden() noexcept;

      KRYS_NODISCARD inline bool AutoRevealsWhenFound() const noexcept;
      inline void SetAutoRevealsWhenFound() noexcept;

      KRYS_NODISCARD inline bool HasAttrContent() const noexcept;
      inline void SetHasAttrContent() noexcept;

      KRYS_NODISCARD inline Maybe<size_t> UsedPositionOptionIndex() const noexcept;
      inline void SetUsedPositionOptionIndex(Maybe<size_t>) noexcept;

      KRYS_NODISCARD inline bool EffectiveInert() const noexcept;
      inline void SetEffectiveInert(bool) noexcept;

      KRYS_NODISCARD inline bool
        IsEffectivelyTransparent() const noexcept; // This or any ancestor has opacity 0.
      inline void SetIsEffectivelyTransparent(bool) noexcept;

      // No setter. Set via `ComputedStyleProperties::setDisplay()`.
      KRYS_NODISCARD constexpr inline DisplayType OriginalDisplay() const noexcept;

      // `effectiveDisplay()` getter is an alias of `ComputedStyleProperties::display()`.
      KRYS_NODISCARD inline DisplayType EffectiveDisplay() const noexcept;
      inline void SetEffectiveDisplay(DisplayType) noexcept;

      KRYS_NODISCARD inline StyleAppearance UsedAppearance() const noexcept;
      inline void SetUsedAppearance(StyleAppearance) noexcept;

      // usedContentVisibility will return ContentVisibility::Hidden in a content-Visibility: hidden subtree
      // (overriding content-Visibility: auto at all times), ContentVisibility::Auto in a content-Visibility:
      // auto subtree (when the content is not user relevant and thus skipped), and ContentVisibility::Visible
      // otherwise.
      KRYS_NODISCARD inline ContentVisibility UsedContentVisibility() const noexcept;
      inline void SetUsedContentVisibility(ContentVisibility) noexcept;

      // 'touch-action' behavior depends on values in ancestors. We use an additional inherited property to
      // implement that.
      KRYS_NODISCARD inline TouchAction UsedTouchAction() const noexcept;
      inline void SetUsedTouchAction(TouchAction) noexcept;

      KRYS_NODISCARD inline ZIndex UsedZIndex() const noexcept;
      inline void SetUsedZIndex(ZIndex) noexcept;

      // MARK: - Pseudo element/style

      KRYS_NODISCARD inline Maybe<PseudoElementType> GetPseudoElementType() const noexcept;
      const css::CSSOMStringAtom &PseudoElementNameArgument() const;

      KRYS_NODISCARD Maybe<PseudoElementIdentifier> GetPseudoElementIdentifier() const noexcept;
      void SetPseudoElementIdentifier(Maybe<PseudoElementIdentifier> &&) noexcept;

      KRYS_NODISCARD inline bool HasAnyPublicPseudoStyles() const noexcept;
      KRYS_NODISCARD inline bool HasPseudoStyle(PseudoElementType) const noexcept;
      inline void SetHasPseudoStyles(EnumSet<PseudoElementType>) noexcept;

      KRYS_NODISCARD RenderStyle *GetCachedPseudoStyle(const PseudoElementIdentifier &) const noexcept;
      RenderStyle *AddCachedPseudoStyle(UniquePtr<RenderStyle>) noexcept;

      KRYS_NODISCARD bool HasCachedPseudoStyles() const noexcept
      {
        return _cachedPseudoStyles && _cachedPseudoStyles->Styles.size();
      }
      KRYS_NODISCARD const PseudoStyleCache *CachedPseudoStyles() const noexcept
      {
        return _cachedPseudoStyles.get();
      }

      // MARK: - Custom properties

      inline const CustomPropertyData &InheritedCustomProperties() const;
      inline const CustomPropertyData &NonInheritedCustomProperties() const;
      KRYS_NODISCARD const CustomProperty *CustomPropertyValue(const css::CSSOMStringAtom &) const noexcept;
      void SetCustomPropertyValue(Ref<const CustomProperty> &&, bool isInherited) noexcept;
      KRYS_NODISCARD bool CustomPropertyValueEqual(const ComputedStyleBase &,
                                                   const css::CSSOMStringAtom &) const noexcept;
      KRYS_NODISCARD bool CustomPropertiesEqual(const ComputedStyleBase &) const noexcept;
      void DeduplicateCustomProperties(const ComputedStyleBase &) noexcept;

      // MARK: - Custom paint

      void AddCustomPaintWatchProperty(const css::CSSOMStringAtom &) noexcept;

      // MARK: - Zoom

      KRYS_NODISCARD inline bool EvaluationTimeZoomEnabled() const noexcept;
      inline void SetEvaluationTimeZoomEnabled(bool) noexcept;

      KRYS_NODISCARD inline float DeviceScaleFactor() const noexcept;
      inline void SetDeviceScaleFactor(float) noexcept;

      KRYS_NODISCARD inline bool UseSVGZoomRulesForLength() const noexcept;
      inline void SetUseSVGZoomRulesForLength(bool) noexcept;

      KRYS_NODISCARD inline float UsedZoom() const noexcept;
      KRYS_NODISCARD inline bool SetUsedZoom(float) noexcept;

      KRYS_NODISCARD inline ZoomFactor UsedZoomForLength() const noexcept;

      // MARK: - Fonts

      inline const FontCascade &GetFontCascade() const;
      KRYS_NODISCARD FontCascade &MutableFontCascadeWithoutUpdate() noexcept;
      void SetFontCascade(FontCascade &&) noexcept;

      KRYS_NODISCARD const FontCascadeDescription &GetFontDescription() const noexcept;
      KRYS_NODISCARD FontCascadeDescription &MutableFontDescriptionWithoutUpdate() noexcept;
      void SetFontDescription(FontCascadeDescription &&) noexcept;
      KRYS_NODISCARD bool SetFontDescriptionWithoutUpdate(FontCascadeDescription &&) noexcept;

      KRYS_NODISCARD const FontMetrics &MetricsOfPrimaryFont() const noexcept;
      KRYS_NODISCARD std::pair<FontOrientation, NonCJKGlyphOrientation> FontAndGlyphOrientation() noexcept;
      KRYS_NODISCARD float ComputedFontSize() const noexcept;
      KRYS_NODISCARD inline WebkitLocale ComputedLocale() const noexcept;
      KRYS_NODISCARD const LineHeight &SpecifiedLineHeight() const noexcept;

      void SetLetterSpacingFromAnimation(LetterSpacing &&) noexcept;
      void SetWordSpacingFromAnimation(WordSpacing &&) noexcept;

      void SynchronizeLetterSpacingWithFontCascade() noexcept;
      void SynchronizeLetterSpacingWithFontCascadeWithoutUpdate() noexcept;
      void SynchronizeWordSpacingWithFontCascade() noexcept;
      void SynchronizeWordSpacingWithFontCascadeWithoutUpdate() noexcept;

      KRYS_NODISCARD inline float UsedLetterSpacing() const noexcept;
      KRYS_NODISCARD inline float UsedWordSpacing() const noexcept;

      // MARK: Writing Modes

      // FIXME: Rename to something that doesn't conflict with a property name.
      // Aggregates `writing-mode`, `direction` and `text-orientation`.
      KRYS_NODISCARD WritingMode GetWritingMode() const noexcept
      {
        return _inheritedFlags.WritingMode;
      }

      // FIXME: *Deprecated* Deprecated due to confusion between physical inline directions and bidi /
      // line-relative directions.
      KRYS_NODISCARD bool IsLeftToRightDirection() const noexcept
      {
        return GetWritingMode().IsBidiLTR();
      }

      // MARK: Aggregates

      inline Animations &EnsureAnimations();
      inline BackgroundLayers &EnsureBackgroundLayers();
      inline MaskLayers &EnsureMaskLayers();
      inline Transitions &EnsureTransitions();

      inline const BorderData &Border() const;
      inline const BorderValue &BorderBottom() const;
      inline const BorderValue &BorderLeft() const;
      inline const BorderValue &BorderRight() const;
      inline const BorderValue &BorderTop() const;
      inline const BorderValue &ColumnRule() const;
      inline const OutlineValue &Outline() const;
      inline const Animations &GetAnimations() const;
      inline const BackgroundLayers &GetBackgroundLayers() const;
      inline const BorderImage &GetBorderImage() const;
      inline const BorderRadius &BorderRadii() const;
      inline const InsetBox &GetInsetBox() const;
      inline const MarginBox &GetMarginBox() const;
      inline const MaskBorder &GetMaskBorder() const;
      inline const MaskLayers &GetMaskLayers() const;
      inline const PaddingBox &GetPaddingBox() const;
      inline const PerspectiveOrigin &GetPerspectiveOrigin() const;
      inline const ScrollMarginBox &GetScrollMarginBox() const;
      inline const ScrollPaddingBox &GetScrollPaddingBox() const;
      inline const ScrollTimelines &GetScrollTimelines() const;
      inline const TransformOrigin &GetTransformOrigin() const;
      inline const Transitions &GetTransitions() const;
      inline const ViewTimelines &GetViewTimelines() const;
      KRYS_NODISCARD inline LineWidthBox BorderWidth() const noexcept;

      inline void SetBackgroundLayers(BackgroundLayers &&) noexcept;
      inline void SetBorderImage(BorderImage &&) noexcept;
      inline void SetBorderRadius(BorderRadiusValue &&) noexcept;
      inline void SetBorderTop(BorderValue &&) noexcept;
      inline void SetBorderRight(BorderValue &&) noexcept;
      inline void SetBorderBottom(BorderValue &&) noexcept;
      inline void SetBorderLeft(BorderValue &&) noexcept;
      inline void SetInsetBox(InsetBox &&) noexcept;
      inline void SetMarginBox(MarginBox &&) noexcept;
      inline void SetMaskBorder(MaskBorder &&) noexcept;
      inline void SetMaskLayers(MaskLayers &&) noexcept;
      inline void SetPaddingBox(PaddingBox &&) noexcept;
      inline void SetPerspectiveOrigin(PerspectiveOrigin &&) noexcept;
      inline void SetTransformOrigin(TransformOrigin &&) noexcept;

      // MARK: - Properties/descriptors that are not yet generated

      // `caret-color`
      inline const Color &CaretColor() const;
      inline const Color &VisitedLinkCaretColor() const;
      KRYS_NODISCARD inline bool HasAutoCaretColor() const noexcept;
      KRYS_NODISCARD inline bool HasVisitedLinkAutoCaretColor() const noexcept;
      inline void SetCaretColor(Color &&) noexcept;
      inline void SetVisitedLinkCaretColor(Color &&) noexcept;
      inline void SetHasAutoCaretColor() noexcept;
      inline void SetHasVisitedLinkAutoCaretColor() noexcept;

      // `cursor`
      KRYS_NODISCARD inline CursorType GetCursorType() const noexcept;

      // `counter-*`
      KRYS_NODISCARD const CounterDirectiveMap &GetCounterDirectives() const noexcept;
      KRYS_NODISCARD CounterDirectiveMap &AccessCounterDirectives() noexcept;

      // `@page size`
      inline const PageSize &GetPageSize() const;
      inline void SetPageSize(PageSize &&) noexcept;

      struct NonInheritedFlags
      {
        KRYS_NODISCARD bool operator==(const NonInheritedFlags &) const noexcept = default;

        inline void CopyNonInheritedFrom(const NonInheritedFlags &) noexcept;

        KRYS_NODISCARD inline bool HasAnyPublicPseudoStyles() const noexcept;
        KRYS_NODISCARD bool HasPseudoStyle(boo::PseudoElementType) const noexcept;
        void SetHasPseudoStyles(EnumSet<boo::PseudoElementType>) noexcept;

        unsigned EffectiveDisplay : 5;
        unsigned OriginalDisplay : 5;
        unsigned OverflowX : 3;
        unsigned OverflowY : 3;
        unsigned Clear : 3;
        unsigned Position : 3;
        unsigned UnicodeBidi : 3;
        unsigned Floating : 3;

        unsigned UsesViewportUnits : 1;
        unsigned UsesContainerUnits : 1;
        unsigned UseTreeCountingFunctions : 1;
        unsigned HasExplicitlyInheritedProperties : 1; // Explicitly inherits a non-inherited property.
        unsigned DisallowsFastPathInheritance : 1;

        // Non-property related state bits.
        unsigned EmptyState : 1;
        unsigned FirstChildState : 1;
        unsigned LastChildState : 1;
        unsigned IsLink : 1;
        unsigned PseudoElementType : PseudoElementTypeBits;
        unsigned PseudoBits : PublicPseudoIDBits;
        unsigned TextDecorationLine
          : TextDecorationLineBits; // Text decorations defined *only* by this element. PREFERRED_TYPE elided
                                    // to avoid header inclusion.

        // If you add more style bits here, you will also need to update
        // ComputedStyleBase::NonInheritedFlags::copyNonInheritedFrom().
      };

      struct InheritedFlags
      {
        KRYS_NODISCARD bool operator==(const InheritedFlags &) const noexcept = default;

        // Writing Mode = 8 bits (can be packed into 6 if needed)
        WritingMode WritingMode;

        // Text Formatting = 19 bits aligned onto 2 bytes + 4 trailing bits
        uint8 WhiteSpaceCollapse : 3;
        uint8 TextWrapMode : 1;
        uint8 TextAlign : 4;
        uint8 TextWrapStyle : 2;
        uint8 TextTransform : TextTransformBits; // PREFERRED_TYPE elided to avoid header inclusion.
        uint8 : 1;                               // byte alignment
        uint8 TextDecorationLineInEffect
          : TextDecorationLineBits; // PREFERRED_TYPE elided to avoid header inclusion.

        // Cursors and Visibility = 13 bits aligned onto 4 bits + 1 byte + 1 bit
        uint8 PointerEvents : 4;
        uint8 Visibility : 2;
        uint8 CursorType : 6;

        // Display Type-Specific = 5 bits
        uint8 ListStylePosition : 1;
        uint8 EmptyCells : 1;
        uint8 BorderCollapse : 1;
        uint8 CaptionSide : 2;

        // -webkit- Stuff = 1 bit
        uint8 RtlOrdering : 1;

        // Color Stuff = 4 bits
        uint8 HasExplicitlySetColor : 1;
        uint8 PrintColorAdjust : 1;
        uint8 InsideLink : 2;

        // Total = 56 bits (fits in 8 bytes)
      };

      const StyleNonInheritedData &NonInheritedData() const
      {
        return _nonInheritedData;
      }
      const NonInheritedFlags &GetNonInheritedFlags() const
      {
        return _nonInheritedFlags;
      }

      const StyleRareInheritedData &RareInheritedData() const
      {
        return _rareInheritedData;
      }
      const StyleInheritedData &InheritedData() const
      {
        return _inheritedData;
      }
      const InheritedFlags &GetInheritedFlags() const
      {
        return _inheritedFlags;
      }

      const SVGRenderStyle &SvgStyle() const
      {
        return _svgStyle;
      }

    protected:
      friend class ChangedAnimatablePropertiesFunctions;
      friend class boo::RenderStyle;
      friend class boo::RenderStyleBase;
      friend class boo::RenderStyleProperties;

      ComputedStyleBase(ComputedStyleBase &&) noexcept;
      ComputedStyleBase &operator=(ComputedStyleBase &&);

      ComputedStyleBase(CreateDefaultStyleTag) noexcept;
      ComputedStyleBase(const ComputedStyleBase &, CloneTag) noexcept;

      ComputedStyleBase(ComputedStyleBase &, ComputedStyleBase &&) noexcept;

      // non-inherited attributes
      DataRef<StyleNonInheritedData> _nonInheritedData;
      NonInheritedFlags _nonInheritedFlags;

      // inherited attributes
      DataRef<StyleRareInheritedData> _rareInheritedData;
      DataRef<StyleInheritedData> _inheritedData;
      InheritedFlags _inheritedFlags;

      // list of associated pseudo Styles
      UniquePtr<PseudoStyleCache> _cachedPseudoStyles;

      DataRef<SVGRenderStyle> _svgStyle;

#if KRYS_DEBUG
      bool _deletionHasBegun {false};
#endif
    };
  }
}
