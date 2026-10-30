#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  enum class PrintColorAdjust : bool
  {
    Economy,
    Exact
  };

  /// @brief The pseudo-elements that have their own style (those not backed by an element).
  enum class PseudoElementType : uint8
  {
    FirstLine,
    FirstLetter,
    GrammarError,
    Highlight,
    Marker,
    Before,
    After,
    Selection,
    Backdrop,
    SpellingError,
    TargetText,
    Checkmark,
    ViewTransition,
    ViewTransitionGroup,
    ViewTransitionImagePair,
    ViewTransitionOld,
    ViewTransitionNew,

    HighestEnumValue = ViewTransitionNew
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::PseudoElementType, 17uz);

namespace krys::boo
{
  constexpr EnumSet<PseudoElementType> AllPublicPseudoElementTypes {
    PseudoElementType::FirstLine,
    PseudoElementType::FirstLetter,
    PseudoElementType::GrammarError,
    PseudoElementType::Highlight,
    PseudoElementType::Marker,
    PseudoElementType::Before,
    PseudoElementType::After,
    PseudoElementType::Selection,
    PseudoElementType::Backdrop,
    PseudoElementType::SpellingError,
    PseudoElementType::TargetText,
    PseudoElementType::Checkmark,
    PseudoElementType::ViewTransition,
    PseudoElementType::ViewTransitionGroup,
    PseudoElementType::ViewTransitionImagePair,
    PseudoElementType::ViewTransitionOld,
    PseudoElementType::ViewTransitionNew,
  };

  constexpr EnumSet<PseudoElementType> AllPseudoElementTypes = AllPublicPseudoElementTypes;

  KRYS_NODISCARD constexpr Maybe<PseudoElementType>
    ParentPseudoElement(PseudoElementType pseudoElementType) noexcept
  {
    switch (pseudoElementType)
    {
      case PseudoElementType::FirstLetter:             return PseudoElementType::FirstLine;
      case PseudoElementType::ViewTransitionGroup:     return PseudoElementType::ViewTransition;
      case PseudoElementType::ViewTransitionImagePair: return PseudoElementType::ViewTransitionGroup;
      case PseudoElementType::ViewTransitionNew:       return PseudoElementType::ViewTransitionImagePair;
      case PseudoElementType::ViewTransitionOld:       return PseudoElementType::ViewTransitionImagePair;
      default:                                         return null;
    }
  }

  enum class ColumnFill : bool
  {
    Balance,
    Auto
  };

  enum class ColumnSpan : bool
  {
    None,
    All
  };

  enum class BorderCollapse : bool
  {
    Separate,
    Collapse
  };

  // These have been defined in the order of their precedence for border-collapsing. Do
  // not change this order! This order also must match the order in css-value-keywords.in.
  enum class BorderStyle : uint8
  {
    None,
    Hidden,
    Inset,
    Groove,
    Outset,
    Ridge,
    Dotted,
    Dashed,
    Solid,
    Double
  };

  enum class BorderPrecedence : uint8
  {
    Off,
    Table,
    ColumnGroup,
    Column,
    RowGroup,
    Row,
    Cell
  };

  enum class OutlineStyle : uint8
  {
    Auto,
    None,
    Inset,
    Groove,
    Outset,
    Ridge,
    Dotted,
    Dashed,
    Solid,
    Double
  };

  enum class PositionType : uint8
  {
    Static = 0,
    Relative = 1,
    Absolute = 2,
    Sticky = 3,
    // This value is required to pack our bits efficiently in RenderObject.
    Fixed = 6
  };

  enum class Float : uint8
  {
    None,
    Left,
    Right,
    InlineStart,
    InlineEnd,
  };

  enum class UsedFloat : uint8
  {
    None = 1 << 0,
    Left = 1 << 1,
    Right = 1 << 2
  };

  // Box decoration attributes. Not inherited.
  enum class BoxDecorationBreak : bool
  {
    Slice,
    Clone
  };

  // Box attributes. Not inherited.
  enum class BoxSizing : bool
  {
    ContentBox,
    BorderBox
  };

  // Random visual rendering model attributes. Not inherited.
  enum class Overflow : uint8
  {
    Visible,
    Hidden,
    Clip,
    Scroll,
    Auto,
    PagedX,
    PagedY
  };

  enum class Clear : uint8
  {
    None,
    Left,
    Right,
    InlineStart,
    InlineEnd,
    Both
  };

  enum class UsedClear : uint8
  {
    None,
    Left,
    Right,
    Both
  };

  enum class TableLayoutType : bool
  {
    Auto,
    Fixed
  };

  enum class TextCombine : bool
  {
    None,
    All
  };

  enum class FillAttachment : uint8
  {
    ScrollBackground,
    LocalBackground,
    FixedBackground
  };

  enum class FillBox : uint8
  {
    BorderBox,
    PaddingBox,
    ContentBox,
    BorderArea,
    Text,
    NoClip
  };

  constexpr uint32 FillBoxBitWidth = 3;

  KRYS_NODISCARD constexpr FillBox ClipMax(FillBox clipA, FillBox clipB) noexcept
  {
    if (clipA == FillBox::BorderBox || clipB == FillBox::BorderBox)
    {
      return FillBox::BorderBox;
    }
    if (clipA == FillBox::PaddingBox || clipB == FillBox::PaddingBox)
    {
      return FillBox::PaddingBox;
    }
    if (clipA == FillBox::ContentBox || clipB == FillBox::ContentBox)
    {
      return FillBox::ContentBox;
    }
    return FillBox::NoClip;
  }

  enum class FillRepeat : uint8
  {
    Repeat,
    NoRepeat,
    Round,
    Space
  };

  // CSS3 Background Values
  enum class FillSizeType : uint8
  {
    Contain,
    Cover,
    Size,
    None
  };

  // CSS3 <position>
  enum class Edge : uint8
  {
    Top,
    Right,
    Bottom,
    Left
  };

  // CSS3 Flexbox Properties
  enum class FlexDirection : uint8
  {
    Row,
    RowReverse,
    Column,
    ColumnReverse
  };

  enum class FlexWrap : uint8
  {
    NoWrap,
    Wrap,
    Reverse
  };

  enum class ItemPosition : uint8
  {
    Legacy,
    Auto,
    Normal,
    Stretch,
    Baseline,
    LastBaseline,
    Center,
    Start,
    End,
    SelfStart,
    SelfEnd,
    FlexStart,
    FlexEnd,
    Left,
    Right,
    AnchorCenter,
  };

  enum class OverflowAlignment : uint8
  {
    Default,
    Unsafe,
    Safe
  };

  enum class ItemPositionType : bool
  {
    NonLegacy,
    Legacy
  };

  enum class ContentPosition : uint8
  {
    Normal,
    Baseline,
    LastBaseline,
    Center,
    Start,
    End,
    FlexStart,
    FlexEnd,
    Left,
    Right
  };

  enum class ContentDistribution : uint8
  {
    Default,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly,
    Stretch
  };

  enum class TextSecurity : uint8
  {
    None,
    Disc,
    Circle,
    Square
  };

  enum class InputSecurity : bool
  {
    Auto,
    None
  };

  // CSS3 User Modify Properties
  enum class UserModify : uint8
  {
    ReadOnly,
    ReadWrite,
    ReadWritePlaintextOnly
  };

  // CSS3 User Drag Values
  enum class UserDrag : uint8
  {
    Auto,
    None,
    Element
  };

  // CSS3 User Select Values
  enum class UserSelect : uint8
  {
    None,
    Text,
    All
  };

  // CSS3 Image Values
  enum class ObjectFit : uint8
  {
    Fill,
    Contain,
    Cover,
    None,
    ScaleDown
  };

  enum class AspectRatioType : uint8
  {
    Auto,
    Ratio,
    AutoAndRatio,
    AutoZero
  };

  enum class WordBreak : uint8
  {
    Normal,
    BreakAll,
    KeepAll,
    BreakWord,
    AutoPhrase
  };

  enum class OverflowWrap : uint8
  {
    Normal,
    BreakWord,
    Anywhere
  };

  enum class LineBreak : uint8
  {
    Auto,
    Loose,
    Normal,
    Strict,
    AfterWhiteSpace,
    Anywhere
  };

  enum class QuoteType : uint8
  {
    OpenQuote,
    CloseQuote,
    NoOpenQuote,
    NoCloseQuote
  };

  enum class AnimationFillMode : uint8
  {
    None,
    Forwards,
    Backwards,
    Both
  };

  enum class AnimationPlayState : bool
  {
    Running,
    Paused
  };

  enum class WhiteSpace : uint8
  {
    Normal,
    Pre,
    PreWrap,
    PreLine,
    NoWrap,
    BreakSpaces
  };

  enum class WhiteSpaceCollapse : uint8
  {
    Collapse,
    Preserve,
    PreserveBreaks,
    BreakSpaces
  };

  enum class TextDecorationStyle : uint8
  {
    Solid,
    Double,
    Dotted,
    Dashed,
    Wavy
  };

  enum class TextJustify : uint8
  {
    Auto,
    None,
    InterWord,
    InterCharacter
  };

  enum class TextDecorationSkipInk : uint8
  {
    None,
    Auto,
    All
  };

  enum class TextGroupAlign : uint8
  {
    None,
    Start,
    End,
    Left,
    Right,
    Center
  };

  enum class TextBoxTrim : uint8
  {
    None,
    TrimStart,
    TrimEnd,
    TrimBoth
  };

  enum class TextEdgeOver : uint8
  {
    Text,
    Ideographic,
    IdeographicInk,
    Cap,
    Ex
  };

  enum class TextEdgeUnder : uint8
  {
    Text,
    Ideographic,
    IdeographicInk,
    Alphabetic,
  };

  enum class BreakBetween : uint8
  {
    Auto,
    Avoid,
    AvoidColumn,
    AvoidPage,
    Column,
    Page,
    LeftPage,
    RightPage,
    RectoPage,
    VersoPage
  };

  KRYS_NODISCARD bool AlwaysPageBreak(BreakBetween) noexcept;

  enum class BreakInside : uint8
  {
    Auto,
    Avoid,
    AvoidColumn,
    AvoidPage
  };

  enum class EmptyCell : bool
  {
    Show,
    Hide
  };

  enum class CaptionSide : uint8
  {
    Top,
    Bottom
  };

  enum class ListStylePosition : bool
  {
    Outside,
    Inside
  };

  enum class Visibility : uint8
  {
    Visible,
    Hidden,
    Collapse
  };

  enum class CursorType : uint8
  {
    // The following must match the order in css-value-keywords.in.
    Auto,
    Default,
    // None
    ContextMenu,
    Help,
    Pointer,
    Progress,
    Wait,
    Cell,
    Crosshair,
    Text,
    VerticalText,
    Alias,
    // Copy
    Move,
    NoDrop,
    NotAllowed,
    Grab,
    Grabbing,
    EResize,
    NResize,
    NEResize,
    NWResize,
    SResize,
    SEResize,
    SWResize,
    WResize,
    EWResize,
    NSResize,
    NESWResize,
    NWSEResize,
    ColumnResize,
    RowResize,
    AllScroll,
    ZoomIn,
    ZoomOut,
    // The following are handled as exceptions so don't need to match.
    Copy,
    None
  };

  enum class DisplayType : uint8
  {
    Inline,
    Block,
    ListItem,
    InlineBlock,
    Table,
    InlineTable,
    TableRowGroup,
    TableHeaderGroup,
    TableFooterGroup,
    TableRow,
    TableColumnGroup,
    TableColumn,
    TableCell,
    TableCaption,
    Box,
    InlineBox,
    Flex,
    InlineFlex,
    Contents,
    Grid,
    InlineGrid,
    GridLanes,
    InlineGridLanes,
    FlowRoot,
    Ruby,
    RubyBlock,
    RubyBase,
    RubyAnnotation,
    None
  };

  enum class InsideLink : uint8
  {
    NotInside,
    InsideUnvisited,
    InsideVisited
  };

  enum class PointerEvents : uint8
  {
    None,
    Auto,
    Stroke,
    Fill,
    Painted,
    Visible,
    VisibleStroke,
    VisibleFill,
    VisiblePainted,
    BoundingBox,
    All
  };

  enum class TransformStyle3D : uint8
  {
    Flat,
    Preserve3D,
  };

  enum class BackfaceVisibility : uint8
  {
    Visible,
    Hidden
  };

  enum class TransformBox : uint8
  {
    StrokeBox,
    ContentBox,
    BorderBox,
    FillBox,
    ViewBox
  };

  enum class OverflowContinue : bool
  {
    Auto,
    Discard
  };

  enum class Hyphens : uint8
  {
    None,
    Manual,
    Auto
  };

  enum class TextEmphasisFill : bool
  {
    Filled,
    Open
  };

  enum class TextEmphasisMark : uint8
  {
    Dot,
    Circle,
    DoubleCircle,
    Triangle,
    Sesame
  };

  enum class TextOverflow : bool
  {
    Clip,
    Ellipsis
  };

  enum class TextWrapMode : bool
  {
    Wrap,
    NoWrap
  };

  enum class TextWrapStyle : uint8
  {
    Auto,
    Balance,
    Pretty,
    Stable
  };

  enum class ImageRendering : uint8
  {
    Auto = 0,
    OptimizeSpeed,
    OptimizeQuality,
    CrispEdges,
    Pixelated
  };

  TextStream &operator<<(TextStream &ts, ImageRendering imageRendering) noexcept;

  enum class Order : bool
  {
    Logical,
    Visual
  };

  enum class ColumnProgression : bool
  {
    Normal,
    Reverse
  };

  enum class LineAlign : bool
  {
    None,
    Edges
  };

  enum class RubyPosition : uint8
  {
    Over,
    Under,
    InterCharacter,
    LegacyInterCharacter
  };

  enum class RubyAlign : uint8
  {
    Start,
    Center,
    SpaceBetween,
    SpaceAround
  };

  enum class RubyOverhang : bool
  {
    Auto,
    None
  };

  enum class ColorScheme : uint8
  {
    Light,
    Dark
  };

  constexpr size_t ColorSchemeBits = 2;

  enum class AutoRepeatType : uint8
  {
    None,
    Fill,
    Fit
  };

  // Reasonable maximum to prevent insane font sizes from causing crashes on some platforms (such as Windows).
  constexpr float MaximumAllowedFontSize = 1000000.0f;

  enum class Isolation : bool
  {
    Auto,
    Isolate
  };

  /// @see https://drafts.csswg.org/css-position-4/#overlay
  enum class Overlay : bool
  {
    None,
    Auto
  };

  // Fill, Stroke, ViewBox are just used for SVG.
  enum class CSSBoxType : uint8
  {
    BoxMissing = 0,
    MarginBox,
    BorderBox,
    PaddingBox,
    ContentBox,
    FillBox,
    StrokeBox,
    ViewBox
  };

  enum class ScrollSnapStrictness : bool
  {
    Proximity,
    Mandatory
  };

  enum class ScrollSnapAxis : uint8
  {
    XAxis,
    YAxis,
    Block,
    Inline,
    Both
  };

  enum class ScrollSnapAxisAlignType : uint8
  {
    None,
    Start,
    Center,
    End
  };

  enum class ScrollSnapStop : bool
  {
    Normal,
    Always,
  };

  enum class FontLoadingBehavior : uint8
  {
    Auto,
    Block,
    Swap,
    Fallback,
    Optional
  };

  enum class EventListenerRegionType : uint64
  {
    Wheel = 1ull << 0,
    NonPassiveWheel = 1ull << 1,
    MouseClick = 1ull << 2,
    TouchStart = 1ull << 3,
    NonPassiveTouchStart = 1ull << 4,
    TouchEnd = 1ull << 5,
    NonPassiveTouchEnd = 1ull << 6,
    TouchCancel = 1ull << 7,
    TouchMove = 1ull << 8,
    NonPassiveTouchMove = 1ull << 9,
    TouchForceChange = 1ull << 10,
    NonPassiveTouchForceChange = 1ull << 11,
    PointerDown = 1ull << 12,
    NonPassivePointerDown = 1ull << 13,
    PointerEnter = 1ull << 14,
    NonPassivePointerEnter = 1ull << 15,
    PointerLeave = 1ull << 16,
    NonPassivePointerLeave = 1ull << 17,
    PointerMove = 1ull << 18,
    NonPassivePointerMove = 1ull << 19,
    PointerOut = 1ull << 20,
    NonPassivePointerOut = 1ull << 21,
    PointerOver = 1ull << 22,
    NonPassivePointerOver = 1ull << 23,
    PointerUp = 1ull << 24,
    NonPassivePointerUp = 1ull << 25,
    MouseDown = 1ull << 26,
    NonPassiveMouseDown = 1ull << 27,
    MouseUp = 1ull << 28,
    NonPassiveMouseUp = 1ull << 29,
    MouseMove = 1ull << 30,
    NonPassiveMouseMove = 1ull << 31,
    GestureChange = 1ull << 32,
    NonPassiveGestureChange = 1ull << 33,
    GestureEnd = 1ull << 34,
    NonPassiveGestureEnd = 1ull << 35,
    GestureStart = 1ull << 36,
    NonPassiveGestureStart = 1ull << 37,
  };

  enum class MathShift : bool
  {
    Normal,
    Compact,
  };

  enum class MathStyle : bool
  {
    Normal,
    Compact,
  };

  enum class ContainerType : uint8
  {
    Normal,
    Size,
    InlineSize,
  };

  enum class ContainIntrinsicSizeType : uint8
  {
    None,
    Length,
    AutoAndLength,
    AutoAndNone,
  };

  enum class ContentVisibility : uint8
  {
    Visible,
    Auto,
    Hidden,
  };

  enum class BlockStepAlign : uint8
  {
    Auto,
    Center,
    Start,
    End
  };

  enum class BlockStepInsert : uint8
  {
    MarginBox,
    PaddingBox,
    ContentBox
  };

  enum class BlockStepRound : uint8
  {
    Up,
    Down,
    Nearest
  };

  enum class FieldSizing : bool
  {
    Fixed,
    Content
  };

  enum class NinePieceImageRule : uint8
  {
    Stretch,
    Round,
    Space,
    Repeat,
  };

  enum class AnimationDirection : uint8
  {
    Normal,
    Alternate,
    Reverse,
    AlternateReverse
  };

  enum class TransitionBehavior : bool
  {
    Normal,
    AllowDiscrete,
  };

  enum class Scroller : uint8
  {
    Nearest,
    Root,
    Self
  };

  enum class TextAnchor : uint8
  {
    Start,
    Middle,
    End
  };

  enum class ColorInterpolation : uint8
  {
    Auto,
    SRGB,
    LinearRGB
  };

  enum class ShapeRendering : uint8
  {
    Auto,
    OptimizeSpeed,
    CrispEdges,
    GeometricPrecision
  };

  enum class GlyphOrientation : uint8
  {
    Degrees0,
    Degrees90,
    Degrees180,
    Degrees270,
    Auto
  };

  enum class AlignmentBaseline : uint8
  {
    Baseline,
    BeforeEdge,
    TextBeforeEdge,
    Middle,
    Central,
    AfterEdge,
    TextAfterEdge,
    Ideographic,
    Alphabetic,
    Hanging,
    Mathematical
  };

  enum class DominantBaseline : uint8
  {
    Auto,
    UseScript,
    NoChange,
    ResetSize,
    Ideographic,
    Alphabetic,
    Hanging,
    Mathematical,
    Central,
    Middle,
    TextAfterEdge,
    TextBeforeEdge
  };

  enum class VectorEffect : uint8
  {
    None,
    NonScalingStroke
  };

  enum class BufferedRendering : uint8
  {
    Auto,
    Dynamic,
    Static
  };

  enum class MaskType : uint8
  {
    Luminance,
    Alpha
  };

  KRYS_NODISCARD CSSBoxType TransformBoxToCSSBoxType(TransformBox) noexcept;

  constexpr float DefaultMiterLimit = 4;

  enum class UsesSVGZoomRulesForLength : bool
  {
    No,
    Yes
  };
}
