#include "Krystal.Booey/Rendering/Style/StyleRareNonInheritedData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleRareNonInheritedData::StyleRareNonInheritedData() noexcept
      : ContainIntrinsicWidth(style::ComputedStyle::InitialContainIntrinsicWidth()),
        ContainIntrinsicHeight(style::ComputedStyle::InitialContainIntrinsicHeight()),
        Zoom(style::ComputedStyle::InitialZoom()), MaxLines(style::ComputedStyle::InitialMaxLines()),
        TouchAction(style::ComputedStyle::InitialTouchAction()),
        InitialLetter(style::ComputedStyle::InitialInitialLetter()),
        BackdropFilter(StyleBackdropFilterData::Create()), Grid(StyleGridData::Create()),
        GridItem(StyleGridItemData::Create()), MaskBorder(StyleMaskBorderData::Create()),
        Clip(style::ComputedStyle::InitialClip())
        // ScrollMargin
        // ScrollPadding
        // CounterDirectives
        ,
        WillChange(style::ComputedStyle::InitialWillChange()),
        PageSize(style::ComputedStyle::InitialPageSize()),
        ShapeOutside(style::ComputedStyle::InitialShapeOutside()),
        ShapeMargin(style::ComputedStyle::InitialShapeMargin()),
        ShapeImageThreshold(style::ComputedStyle::InitialShapeImageThreshold()),
        Perspective(style::ComputedStyle::InitialPerspective()),
        PerspectiveOrigin({style::ComputedStyle::InitialPerspectiveOriginX(),
                           style::ComputedStyle::InitialPerspectiveOriginY()}),
        ClipPath(style::ComputedStyle::InitialClipPath()),
        CustomProperties(style::CustomPropertyData::Create())
        // CustomPaintWatchedProperties
        ,
        Rotate(style::ComputedStyle::InitialRotate()), Scale(style::ComputedStyle::InitialScale()),
        Translate(style::ComputedStyle::InitialTranslate()),
        ContainerNames(style::ComputedStyle::InitialContainerNames()),
        ViewTransitionClasses(style::ComputedStyle::InitialViewTransitionClasses()),
        ViewTransitionName(style::ComputedStyle::InitialViewTransitionName()),
        ColumnGap(style::ComputedStyle::InitialColumnGap()), RowGap(style::ComputedStyle::InitialRowGap()),
        ItemTolerance(style::ComputedStyle::InitialItemTolerance()),
        OffsetPath(style::ComputedStyle::InitialOffsetPath()),
        OffsetDistance(style::ComputedStyle::InitialOffsetDistance()),
        OffsetPosition(style::ComputedStyle::InitialOffsetPosition()),
        OffsetAnchor(style::ComputedStyle::InitialOffsetAnchor()),
        OffsetRotate(style::ComputedStyle::InitialOffsetRotate()),
        TextDecorationColor(style::ComputedStyle::InitialTextDecorationColor()),
        TextDecorationThickness(style::ComputedStyle::InitialTextDecorationThickness())
        // ScrollTimelines
        ,
        ScrollTimelineAxes(style::ComputedStyle::InitialScrollTimelineAxes()),
        ScrollTimelineNames(style::ComputedStyle::InitialScrollTimelineNames())
        // ViewTimelines
        ,
        ViewTimelineInsets(style::ComputedStyle::InitialViewTimelineInsets()),
        ViewTimelineAxes(style::ComputedStyle::InitialViewTimelineAxes()),
        ViewTimelineNames(style::ComputedStyle::InitialViewTimelineNames()),
        TimelineScope(style::ComputedStyle::InitialTimelineScope()),
        ScrollbarGutter(style::ComputedStyle::InitialScrollbarGutter()),
        ScrollSnapType(style::ComputedStyle::InitialScrollSnapType()),
        ScrollSnapAlign(style::ComputedStyle::InitialScrollSnapAlign()),
        PseudoElementNameArgument(css::CSSOMStringAtom::Null()),
        AnchorNames(style::ComputedStyle::InitialAnchorNames()),
        AnchorScope(style::ComputedStyle::InitialAnchorScope()),
        PositionAnchor(style::ComputedStyle::InitialPositionAnchor()),
        PositionArea(style::ComputedStyle::InitialPositionArea()),
        PositionTryFallbacks(style::ComputedStyle::InitialPositionTryFallbacks()), UsedPositionOptionIndex(),
        BlockStepSize(style::ComputedStyle::InitialBlockStepSize()),
        BlockStepAlign(static_cast<unsigned>(style::ComputedStyle::InitialBlockStepAlign())),
        BlockStepInsert(static_cast<unsigned>(style::ComputedStyle::InitialBlockStepInsert())),
        BlockStepRound(static_cast<unsigned>(style::ComputedStyle::InitialBlockStepRound())),
        OverscrollBehaviorX(static_cast<unsigned>(style::ComputedStyle::InitialOverscrollBehaviorX())),
        OverscrollBehaviorY(static_cast<unsigned>(style::ComputedStyle::InitialOverscrollBehaviorY())),
        TransformStyle3D(static_cast<unsigned>(style::ComputedStyle::InitialTransformStyle3D())),
        TransformStyleForcedToFlat(false),
        BackfaceVisibility(static_cast<unsigned>(style::ComputedStyle::InitialBackfaceVisibility())),
        ScrollBehavior(static_cast<unsigned>(style::ComputedStyle::InitialScrollBehavior())),
        TextDecorationStyle(static_cast<unsigned>(style::ComputedStyle::InitialTextDecorationStyle())),
        TextGroupAlign(static_cast<unsigned>(style::ComputedStyle::InitialTextGroupAlign())),
        ContentVisibility(static_cast<unsigned>(style::ComputedStyle::InitialContentVisibility())),
        EffectiveBlendMode(static_cast<unsigned>(style::ComputedStyle::InitialBlendMode())),
        Isolation(static_cast<unsigned>(style::ComputedStyle::InitialIsolation())),
        Overlay(static_cast<unsigned>(style::ComputedStyle::InitialOverlay())),
        InputSecurity(static_cast<unsigned>(style::ComputedStyle::InitialInputSecurity())),
        BreakBefore(static_cast<unsigned>(style::ComputedStyle::InitialBreakBefore())),
        BreakAfter(static_cast<unsigned>(style::ComputedStyle::InitialBreakAfter())),
        BreakInside(static_cast<unsigned>(style::ComputedStyle::InitialBreakInside())),
        ContainerType(static_cast<unsigned>(style::ComputedStyle::InitialContainerType())),
        TextBoxTrim(static_cast<unsigned>(style::ComputedStyle::InitialTextBoxTrim())),
        OverflowAnchor(static_cast<unsigned>(style::ComputedStyle::InitialOverflowAnchor())),
        PositionTryOrder(static_cast<unsigned>(style::ComputedStyle::InitialPositionTryOrder())),
        PositionVisibility(static_cast<unsigned>(style::ComputedStyle::InitialPositionVisibility().ToRaw())),
        FieldSizing(static_cast<unsigned>(style::ComputedStyle::InitialFieldSizing())),
        NativeAppearanceDisabled(static_cast<unsigned>(false)),
        ScrollbarWidth(static_cast<unsigned>(style::ComputedStyle::InitialScrollbarWidth())),
        UsesAnchorFunctions(false), AnchorFunctionScrollCompensatedAxes(0), IsPopoverInvoker(false),
        UseSVGZoomRulesForLength(false),
        MarginTrim(static_cast<unsigned>(style::ComputedStyle::InitialMarginTrim().ToRaw())),
        Contain(static_cast<unsigned>(style::ComputedStyle::InitialContain().ToRaw())),
        OverflowContinue(static_cast<unsigned>(style::ComputedStyle::InitialOverflowContinue())),
        ScrollSnapStop(static_cast<unsigned>(style::ComputedStyle::InitialScrollSnapStop()))
  {
  }

  inline StyleRareNonInheritedData::StyleRareNonInheritedData(const StyleRareNonInheritedData &o) noexcept
      : RefCounted<StyleRareNonInheritedData>(), ContainIntrinsicWidth(o.ContainIntrinsicWidth),
        ContainIntrinsicHeight(o.ContainIntrinsicHeight), Zoom(o.Zoom), MaxLines(o.MaxLines),
        TouchAction(o.TouchAction), InitialLetter(o.InitialLetter), BackdropFilter(o.BackdropFilter),
        Grid(o.Grid), GridItem(o.GridItem), MaskBorder(o.MaskBorder), Clip(o.Clip),
        ScrollMargin(o.ScrollMargin), ScrollPadding(o.ScrollPadding), CounterDirectives(o.CounterDirectives),
        WillChange(o.WillChange), PageSize(o.PageSize), ShapeOutside(o.ShapeOutside),
        ShapeMargin(o.ShapeMargin), ShapeImageThreshold(o.ShapeImageThreshold), Perspective(o.Perspective),
        PerspectiveOrigin(o.PerspectiveOrigin), ClipPath(o.ClipPath), CustomProperties(o.CustomProperties),
        CustomPaintWatchedProperties(o.CustomPaintWatchedProperties), Rotate(o.Rotate), Scale(o.Scale),
        Translate(o.Translate), ContainerNames(o.ContainerNames),
        ViewTransitionClasses(o.ViewTransitionClasses), ViewTransitionName(o.ViewTransitionName),
        ColumnGap(o.ColumnGap), RowGap(o.RowGap), ItemTolerance(o.ItemTolerance), OffsetPath(o.OffsetPath),
        OffsetDistance(o.OffsetDistance), OffsetPosition(o.OffsetPosition), OffsetAnchor(o.OffsetAnchor),
        OffsetRotate(o.OffsetRotate), TextDecorationColor(o.TextDecorationColor),
        TextDecorationThickness(o.TextDecorationThickness), ScrollTimelines(o.ScrollTimelines),
        ScrollTimelineAxes(o.ScrollTimelineAxes), ScrollTimelineNames(o.ScrollTimelineNames),
        ViewTimelines(o.ViewTimelines), ViewTimelineInsets(o.ViewTimelineInsets),
        ViewTimelineAxes(o.ViewTimelineAxes), ViewTimelineNames(o.ViewTimelineNames),
        TimelineScope(o.TimelineScope), ScrollbarGutter(o.ScrollbarGutter), ScrollSnapType(o.ScrollSnapType),
        ScrollSnapAlign(o.ScrollSnapAlign), PseudoElementNameArgument(o.PseudoElementNameArgument),
        AnchorNames(o.AnchorNames), AnchorScope(o.AnchorScope), PositionAnchor(o.PositionAnchor),
        PositionArea(o.PositionArea), PositionTryFallbacks(o.PositionTryFallbacks),
        UsedPositionOptionIndex(o.UsedPositionOptionIndex), BlockStepSize(o.BlockStepSize),
        BlockStepAlign(o.BlockStepAlign), BlockStepInsert(o.BlockStepInsert),
        BlockStepRound(o.BlockStepRound), OverscrollBehaviorX(o.OverscrollBehaviorX),
        OverscrollBehaviorY(o.OverscrollBehaviorY), TransformStyle3D(o.TransformStyle3D),
        TransformStyleForcedToFlat(o.TransformStyleForcedToFlat), BackfaceVisibility(o.BackfaceVisibility),
        ScrollBehavior(o.ScrollBehavior), TextDecorationStyle(o.TextDecorationStyle),
        TextGroupAlign(o.TextGroupAlign), ContentVisibility(o.ContentVisibility),
        EffectiveBlendMode(o.EffectiveBlendMode), Isolation(o.Isolation), Overlay(o.Overlay),
        InputSecurity(o.InputSecurity), BreakBefore(o.BreakBefore), BreakAfter(o.BreakAfter),
        BreakInside(o.BreakInside), ContainerType(o.ContainerType), TextBoxTrim(o.TextBoxTrim),
        OverflowAnchor(o.OverflowAnchor), PositionTryOrder(o.PositionTryOrder),
        PositionVisibility(o.PositionVisibility), FieldSizing(o.FieldSizing),
        NativeAppearanceDisabled(o.NativeAppearanceDisabled), ScrollbarWidth(o.ScrollbarWidth),
        UsesAnchorFunctions(o.UsesAnchorFunctions),
        AnchorFunctionScrollCompensatedAxes(o.AnchorFunctionScrollCompensatedAxes),
        IsPopoverInvoker(o.IsPopoverInvoker), UseSVGZoomRulesForLength(o.UseSVGZoomRulesForLength),
        MarginTrim(o.MarginTrim), Contain(o.Contain), OverflowContinue(o.OverflowContinue),
        ScrollSnapStop(o.ScrollSnapStop)
  {
  }

  Ref<StyleRareNonInheritedData> StyleRareNonInheritedData::Copy() const noexcept
  {
    return AdoptRef(*new StyleRareNonInheritedData(*this));
  }

  StyleRareNonInheritedData::~StyleRareNonInheritedData() noexcept = default;

  bool StyleRareNonInheritedData::operator==(const StyleRareNonInheritedData &o) const noexcept
  {
    return ContainIntrinsicWidth == o.ContainIntrinsicWidth
           && ContainIntrinsicHeight == o.ContainIntrinsicHeight && Zoom == o.Zoom && MaxLines == o.MaxLines
           && TouchAction == o.TouchAction && InitialLetter == o.InitialLetter
           && BackdropFilter == o.BackdropFilter && Grid == o.Grid && GridItem == o.GridItem
           && MaskBorder == o.MaskBorder && Clip == o.Clip && ScrollMargin == o.ScrollMargin
           && ScrollPadding == o.ScrollPadding && CounterDirectives == o.CounterDirectives
           && WillChange == o.WillChange && PageSize == o.PageSize && ShapeOutside == o.ShapeOutside
           && ShapeMargin == o.ShapeMargin && ShapeImageThreshold == o.ShapeImageThreshold
           && Perspective == o.Perspective && PerspectiveOrigin == o.PerspectiveOrigin
           && ClipPath == o.ClipPath && TextDecorationColor == o.TextDecorationColor
           && CustomProperties == o.CustomProperties
           && CustomPaintWatchedProperties == o.CustomPaintWatchedProperties && Rotate == o.Rotate
           && Scale == o.Scale && Translate == o.Translate && ContainerNames == o.ContainerNames
           && ColumnGap == o.ColumnGap && RowGap == o.RowGap && ItemTolerance == o.ItemTolerance
           && OffsetPath == o.OffsetPath && OffsetDistance == o.OffsetDistance
           && OffsetPosition == o.OffsetPosition && OffsetAnchor == o.OffsetAnchor
           && OffsetRotate == o.OffsetRotate && TextDecorationThickness == o.TextDecorationThickness
           && ScrollTimelines == o.ScrollTimelines && ScrollTimelineAxes == o.ScrollTimelineAxes
           && ScrollTimelineNames == o.ScrollTimelineNames && ViewTimelines == o.ViewTimelines
           && ViewTimelineInsets == o.ViewTimelineInsets && ViewTimelineAxes == o.ViewTimelineAxes
           && ViewTimelineNames == o.ViewTimelineNames && TimelineScope == o.TimelineScope
           && ScrollbarGutter == o.ScrollbarGutter && ScrollSnapType == o.ScrollSnapType
           && ScrollSnapAlign == o.ScrollSnapAlign && PseudoElementNameArgument == o.PseudoElementNameArgument
           && AnchorNames == o.AnchorNames && AnchorScope == o.AnchorScope
           && PositionAnchor == o.PositionAnchor && PositionArea == o.PositionArea
           && PositionTryFallbacks == o.PositionTryFallbacks
           && UsedPositionOptionIndex == o.UsedPositionOptionIndex && BlockStepSize == o.BlockStepSize
           && BlockStepAlign == o.BlockStepAlign && BlockStepInsert == o.BlockStepInsert
           && BlockStepRound == o.BlockStepRound && OverscrollBehaviorX == o.OverscrollBehaviorX
           && OverscrollBehaviorY == o.OverscrollBehaviorY && TransformStyle3D == o.TransformStyle3D
           && TransformStyleForcedToFlat == o.TransformStyleForcedToFlat
           && BackfaceVisibility == o.BackfaceVisibility && ScrollBehavior == o.ScrollBehavior
           && TextDecorationStyle == o.TextDecorationStyle && TextGroupAlign == o.TextGroupAlign
           && EffectiveBlendMode == o.EffectiveBlendMode && Isolation == o.Isolation && Overlay == o.Overlay
           && InputSecurity == o.InputSecurity && ContentVisibility == o.ContentVisibility
           && BreakAfter == o.BreakAfter && BreakBefore == o.BreakBefore && BreakInside == o.BreakInside
           && ContainerType == o.ContainerType && TextBoxTrim == o.TextBoxTrim
           && OverflowAnchor == o.OverflowAnchor && ViewTransitionClasses == o.ViewTransitionClasses
           && ViewTransitionName == o.ViewTransitionName && PositionTryOrder == o.PositionTryOrder
           && PositionVisibility == o.PositionVisibility && FieldSizing == o.FieldSizing
           && NativeAppearanceDisabled == o.NativeAppearanceDisabled && ScrollbarWidth == o.ScrollbarWidth
           && UsesAnchorFunctions == o.UsesAnchorFunctions
           && AnchorFunctionScrollCompensatedAxes == o.AnchorFunctionScrollCompensatedAxes
           && IsPopoverInvoker == o.IsPopoverInvoker && UseSVGZoomRulesForLength == o.UseSVGZoomRulesForLength
           && MarginTrim == o.MarginTrim && Contain == o.Contain && OverflowContinue == o.OverflowContinue
           && ScrollSnapStop == o.ScrollSnapStop;
  }

  style::Contain StyleRareNonInheritedData::UsedContain() const noexcept
  {
    auto result = style::Contain::FromRaw(Contain);

    switch (static_cast<boo::ContainerType>(ContainerType))
    {
      case boo::ContainerType::Normal: break;
      case boo::ContainerType::Size:
        result.Add({style::ContainValue::Style, style::ContainValue::Size});
        break;
      case boo::ContainerType::InlineSize:
        result.Add({style::ContainValue::Style, style::ContainValue::InlineSize});
        break;
    };

    return result;
  }
}
