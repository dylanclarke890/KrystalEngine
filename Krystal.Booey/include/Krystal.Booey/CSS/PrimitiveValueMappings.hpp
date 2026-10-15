#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Core/StyleAppearance.hpp"
#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Core/ThemeTypes.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/FontSizeAdjust.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/WindRule.hpp"
#include "Krystal.Booey/Rendering/Style/PositionTryOrder.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbackTactic.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionVisibility.hpp"
#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"
#include "Krystal.Booey/Style/Values/Contain/Contain.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrientation.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskMode.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLineBoxContain.hpp"
#include "Krystal.Booey/Style/Values/Overflow/ScrollBehavior.hpp"
#include "Krystal.Booey/Style/Values/PointerEvents/TouchAction.hpp"
#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"
#include "Krystal.Booey/Style/Values/Speech/SpeakAs.hpp"
#include "Krystal.Booey/Style/Values/Text/HangingPunctuation.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlign.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlignLast.hpp"
#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextEmphasisPosition.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlinePosition.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Booey/Style/Values/UI/Resize.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Visitor.hpp"

// FIXME: The mapping of `style::PositionTryOrder` is added with its type.

namespace krys::boo
{
  template <typename TargetType>
  KRYS_NODISCARD constexpr TargetType FromValueId(css::ValueId) noexcept;

  template <typename TargetType>
  KRYS_NODISCARD TargetType FromCSSValue(const css::Value &value) noexcept
  {
    return FromValueId<TargetType>(value.ValueId());
  }

#define KRYS_EMIT_TO_VALUE_ID_SWITCH_CASE(VALUE)                                                             \
  case KRYS_MAPPING_TYPE::VALUE: return css::ValueId::VALUE;
#define KRYS_EMIT_FROM_VALUE_ID_SWITCH_CASE(VALUE)                                                           \
  case css::ValueId::VALUE: return KRYS_MAPPING_TYPE::VALUE;
#define KRYS_EMIT_VALUE_REPRESENTATION_SWITCH_CASE(VALUE)                                                    \
  case boo::KRYS_MAPPING_TYPE::VALUE: return visitor(css::keywords::VALUE {});

#define KRYS_DEFINE_TO_VALUE_ID_FUNCTION                                                                     \
  KRYS_NODISCARD constexpr css::ValueId ToValueId(KRYS_MAPPING_TYPE value) noexcept                          \
  {                                                                                                          \
    switch (value)                                                                                           \
    {                                                                                                        \
      KRYS_MAPPING_FOR_EACH(KRYS_EMIT_TO_VALUE_ID_SWITCH_CASE)                                               \
    }                                                                                                        \
    krys_debug_assert_msg(false, "Unexpected value");                                                        \
    return css::ValueId::Invalid;                                                                            \
  }

#define KRYS_DEFINE_FROM_VALUE_ID_FUNCTION                                                                   \
  template <>                                                                                                \
  KRYS_NODISCARD constexpr KRYS_MAPPING_TYPE FromValueId(css::ValueId value) noexcept                        \
  {                                                                                                          \
    switch (value)                                                                                           \
    {                                                                                                        \
      KRYS_MAPPING_FOR_EACH(KRYS_EMIT_FROM_VALUE_ID_SWITCH_CASE)                                             \
      default: break;                                                                                        \
    }                                                                                                        \
    krys_debug_assert_msg(false, "Unexpected value");                                                        \
    return {};                                                                                               \
  }

#define KRYS_DEFINE_VALUE_REPRESENTATION_FUNCTION                                                            \
  template <>                                                                                                \
  struct style::ValueRepresentation<boo::KRYS_MAPPING_TYPE>                                                  \
  {                                                                                                          \
    template <typename... F>                                                                                 \
    constexpr decltype(auto) operator()(boo::KRYS_MAPPING_TYPE value, F &&...f) noexcept                     \
    {                                                                                                        \
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);                                             \
      switch (value)                                                                                         \
      {                                                                                                      \
        KRYS_MAPPING_FOR_EACH(KRYS_EMIT_VALUE_REPRESENTATION_SWITCH_CASE)                                    \
      }                                                                                                      \
      krys_unreachable();                                                                                    \
    }                                                                                                        \
  };

#define KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS                                                               \
  KRYS_DEFINE_TO_VALUE_ID_FUNCTION                                                                           \
  KRYS_DEFINE_FROM_VALUE_ID_FUNCTION                                                                         \
  KRYS_DEFINE_VALUE_REPRESENTATION_FUNCTION

#define KRYS_MAPPING_TYPE ColumnFill
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Balance)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ColumnSpan
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(All) CASE(None)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE PrintColorAdjust
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Exact) CASE(Economy)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE BlockStepAlign
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Center) CASE(Start) CASE(End)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE BlockStepInsert
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(MarginBox) CASE(PaddingBox) CASE(ContentBox)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE BlockStepRound
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Up) CASE(Down) CASE(Nearest)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(BorderStyle e) noexcept
  {
    switch (e)
    {
      case BorderStyle::None:   return css::ValueId::None;
      case BorderStyle::Hidden: return css::ValueId::Hidden;
      case BorderStyle::Inset:  return css::ValueId::Inset;
      case BorderStyle::Groove: return css::ValueId::Groove;
      case BorderStyle::Ridge:  return css::ValueId::Ridge;
      case BorderStyle::Outset: return css::ValueId::Outset;
      case BorderStyle::Dotted: return css::ValueId::Dotted;
      case BorderStyle::Dashed: return css::ValueId::Dashed;
      case BorderStyle::Solid:  return css::ValueId::Solid;
      case BorderStyle::Double: return css::ValueId::Double;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr BorderStyle FromValueId(css::ValueId valueID) noexcept
  {
    return static_cast<BorderStyle>(static_cast<uint16>(valueID) - static_cast<uint16>(css::ValueId::None));
  }

#define KRYS_MAPPING_TYPE OutlineStyle
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Auto)                                                                                                 \
  CASE(None)                                                                                                 \
  CASE(Inset) CASE(Groove) CASE(Ridge) CASE(Outset) CASE(Dotted) CASE(Dashed) CASE(Solid) CASE(Double)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(CompositeOperator e) noexcept
  {
    switch (e)
    {
      case CompositeOperator::SourceOver: return css::ValueId::Add;
      case CompositeOperator::SourceIn:   return css::ValueId::Intersect;
      case CompositeOperator::SourceOut:  return css::ValueId::Subtract;
      case CompositeOperator::XOR:        return css::ValueId::Exclude;
      default:                            break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  KRYS_NODISCARD constexpr css::ValueId ToValueIdForWebkitMaskComposite(CompositeOperator e) noexcept
  {
    switch (e)
    {
      case CompositeOperator::Clear:           return css::ValueId::Clear;
      case CompositeOperator::Copy:            return css::ValueId::Copy;
      case CompositeOperator::SourceOver:      return css::ValueId::SourceOver;
      case CompositeOperator::SourceIn:        return css::ValueId::SourceIn;
      case CompositeOperator::SourceOut:       return css::ValueId::SourceOut;
      case CompositeOperator::SourceAtop:      return css::ValueId::SourceAtop;
      case CompositeOperator::DestinationOver: return css::ValueId::DestinationOver;
      case CompositeOperator::DestinationIn:   return css::ValueId::DestinationIn;
      case CompositeOperator::DestinationOut:  return css::ValueId::DestinationOut;
      case CompositeOperator::DestinationAtop: return css::ValueId::DestinationAtop;
      case CompositeOperator::XOR:             return css::ValueId::Xor;
      case CompositeOperator::PlusDarker:      return css::ValueId::PlusDarker;
      case CompositeOperator::PlusLighter:     return css::ValueId::PlusLighter;
      case CompositeOperator::Difference:      break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr CompositeOperator FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Clear:           return CompositeOperator::Clear;
      case css::ValueId::Copy:            return CompositeOperator::Copy;
      case css::ValueId::SourceOver:
      case css::ValueId::Add:             return CompositeOperator::SourceOver;
      case css::ValueId::SourceIn:
      case css::ValueId::Intersect:       return CompositeOperator::SourceIn;
      case css::ValueId::SourceOut:
      case css::ValueId::Subtract:        return CompositeOperator::SourceOut;
      case css::ValueId::SourceAtop:      return CompositeOperator::SourceAtop;
      case css::ValueId::DestinationOver: return CompositeOperator::DestinationOver;
      case css::ValueId::DestinationIn:   return CompositeOperator::DestinationIn;
      case css::ValueId::DestinationOut:  return CompositeOperator::DestinationOut;
      case css::ValueId::DestinationAtop: return CompositeOperator::DestinationAtop;
      case css::ValueId::Xor:
      case css::ValueId::Exclude:         return CompositeOperator::XOR;
      case css::ValueId::PlusDarker:      return CompositeOperator::PlusDarker;
      case css::ValueId::PlusLighter:     return CompositeOperator::PlusLighter;
      default:                            break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return CompositeOperator::Clear;
  }

  KRYS_NODISCARD constexpr css::ValueId ToValueId(StyleAppearance e) noexcept
  {
    switch (e)
    {
      case StyleAppearance::None:             return css::ValueId::None;
      case StyleAppearance::Auto:             return css::ValueId::Auto;
      case StyleAppearance::Base:             return css::ValueId::Base;
      case StyleAppearance::Checkbox:         return css::ValueId::Checkbox;
      case StyleAppearance::Radio:            return css::ValueId::Radio;
      case StyleAppearance::PushButton:       return css::ValueId::PushButton;
      case StyleAppearance::SquareButton:     return css::ValueId::SquareButton;
      case StyleAppearance::Button:           return css::ValueId::Button;
      case StyleAppearance::DefaultButton:    return css::ValueId::DefaultButton;
      case StyleAppearance::Listbox:          return css::ValueId::Listbox;
      case StyleAppearance::Menulist:         return css::ValueId::Menulist;
      case StyleAppearance::MenulistButton:   return css::ValueId::MenulistButton;
      case StyleAppearance::Meter:            return css::ValueId::Meter;
      case StyleAppearance::ProgressBar:      return css::ValueId::ProgressBar;
      case StyleAppearance::SliderHorizontal: return css::ValueId::SliderHorizontal;
      case StyleAppearance::SliderVertical:   return css::ValueId::SliderVertical;
      case StyleAppearance::SearchField:      return css::ValueId::Searchfield;
      case StyleAppearance::TextField:        return css::ValueId::Textfield;
      case StyleAppearance::TextArea:         return css::ValueId::Textarea;
      case StyleAppearance::ColorWell:
      case StyleAppearance::ColorWellSwatch:
      case StyleAppearance::ColorWellSwatchOverlay:
      case StyleAppearance::ColorWellSwatchWrapper:
      case StyleAppearance::InnerSpinButton:
      case StyleAppearance::ListButton:
      case StyleAppearance::SearchFieldDecoration:
      case StyleAppearance::SearchFieldResultsDecoration:
      case StyleAppearance::SearchFieldResultsButton:
      case StyleAppearance::SearchFieldCancelButton:
      case StyleAppearance::SliderThumbHorizontal:
      case StyleAppearance::SliderThumbVertical:
      case StyleAppearance::Switch:
      case StyleAppearance::SwitchThumb:
      case StyleAppearance::SwitchTrack:
        krys_debug_assert_msg(false, "Unexpected value");
        return css::ValueId::None;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr StyleAppearance FromValueId(css::ValueId valueID) noexcept
  {
    if (valueID == css::ValueId::None)
    {
      return StyleAppearance::None;
    }

    if (valueID == css::ValueId::Auto)
    {
      return StyleAppearance::Auto;
    }

    return static_cast<StyleAppearance>(static_cast<uint16>(valueID) - static_cast<uint16>(css::ValueId::Base)
                                        + static_cast<uint16>(StyleAppearance::Base));
  }

#define KRYS_MAPPING_TYPE BackfaceVisibility
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Visible) CASE(Hidden)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE FieldSizing
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Fixed) CASE(Content)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(FillAttachment e) noexcept
  {
    switch (e)
    {
      case FillAttachment::ScrollBackground: return css::ValueId::Scroll;
      case FillAttachment::LocalBackground:  return css::ValueId::Local;
      case FillAttachment::FixedBackground:  return css::ValueId::Fixed;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr FillAttachment FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Scroll: return FillAttachment::ScrollBackground;
      case css::ValueId::Local:  return FillAttachment::LocalBackground;
      case css::ValueId::Fixed:  return FillAttachment::FixedBackground;
      default:                   break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return FillAttachment::ScrollBackground;
  }

  KRYS_NODISCARD constexpr css::ValueId ToValueId(FillBox e) noexcept
  {
    switch (e)
    {
      case FillBox::BorderBox:  return css::ValueId::BorderBox;
      case FillBox::PaddingBox: return css::ValueId::PaddingBox;
      case FillBox::ContentBox: return css::ValueId::ContentBox;
      case FillBox::BorderArea: return css::ValueId::BorderArea;
      case FillBox::Text:       return css::ValueId::Text;
      case FillBox::NoClip:     return css::ValueId::NoClip;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr FillBox FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Border:
      case css::ValueId::BorderBox:  return FillBox::BorderBox;
      case css::ValueId::Padding:
      case css::ValueId::PaddingBox: return FillBox::PaddingBox;
      case css::ValueId::Content:
      case css::ValueId::ContentBox: return FillBox::ContentBox;
      case css::ValueId::BorderArea: return FillBox::BorderArea;
      case css::ValueId::Text:
      case css::ValueId::WebkitText: return FillBox::Text;
      case css::ValueId::NoClip:     return FillBox::NoClip;
      default:                       break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return FillBox::BorderBox;
  }

#define KRYS_MAPPING_TYPE FillRepeat
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Repeat) CASE(NoRepeat) CASE(Round) CASE(Space)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE BoxDecorationBreak
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Slice) CASE(Clone)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE Edge
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Top) CASE(Right) CASE(Bottom) CASE(Left)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE BoxSizing
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(BorderBox) CASE(ContentBox)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE CaptionSide
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Top) CASE(Bottom)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE Clear
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(None) CASE(Left) CASE(Right) CASE(InlineStart) CASE(InlineEnd) CASE(Both)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextBoxTrim
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(TrimStart) CASE(TrimEnd) CASE(TrimBoth)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(CursorType e) noexcept
  {
    switch (e)
    {
      case CursorType::Auto:         return css::ValueId::Auto;
      case CursorType::Default:      return css::ValueId::Default;
      case CursorType::None:         return css::ValueId::None;
      case CursorType::ContextMenu:  return css::ValueId::ContextMenu;
      case CursorType::Help:         return css::ValueId::Help;
      case CursorType::Pointer:      return css::ValueId::Pointer;
      case CursorType::Progress:     return css::ValueId::Progress;
      case CursorType::Wait:         return css::ValueId::Wait;
      case CursorType::Cell:         return css::ValueId::Cell;
      case CursorType::Crosshair:    return css::ValueId::Crosshair;
      case CursorType::Text:         return css::ValueId::Text;
      case CursorType::VerticalText: return css::ValueId::VerticalText;
      case CursorType::Alias:        return css::ValueId::Alias;
      case CursorType::Copy:         return css::ValueId::Copy;
      case CursorType::Move:         return css::ValueId::Move;
      case CursorType::NoDrop:       return css::ValueId::NoDrop;
      case CursorType::NotAllowed:   return css::ValueId::NotAllowed;
      case CursorType::Grab:         return css::ValueId::Grab;
      case CursorType::Grabbing:     return css::ValueId::Grabbing;
      case CursorType::EResize:      return css::ValueId::EResize;
      case CursorType::NResize:      return css::ValueId::NResize;
      case CursorType::NEResize:     return css::ValueId::NeResize;
      case CursorType::NWResize:     return css::ValueId::NwResize;
      case CursorType::SResize:      return css::ValueId::SResize;
      case CursorType::SEResize:     return css::ValueId::SeResize;
      case CursorType::SWResize:     return css::ValueId::SwResize;
      case CursorType::WResize:      return css::ValueId::WResize;
      case CursorType::EWResize:     return css::ValueId::EwResize;
      case CursorType::NSResize:     return css::ValueId::NsResize;
      case CursorType::NESWResize:   return css::ValueId::NeswResize;
      case CursorType::NWSEResize:   return css::ValueId::NwseResize;
      case CursorType::ColumnResize: return css::ValueId::ColResize;
      case CursorType::RowResize:    return css::ValueId::RowResize;
      case CursorType::AllScroll:    return css::ValueId::AllScroll;
      case CursorType::ZoomIn:       return css::ValueId::ZoomIn;
      case CursorType::ZoomOut:      return css::ValueId::ZoomOut;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr CursorType FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Copy:           return CursorType::Copy;
      case css::ValueId::WebkitGrab:     return CursorType::Grab;
      case css::ValueId::WebkitGrabbing: return CursorType::Grabbing;
      case css::ValueId::WebkitZoomIn:   return CursorType::ZoomIn;
      case css::ValueId::WebkitZoomOut:  return CursorType::ZoomOut;
      case css::ValueId::None:           return CursorType::None;
      default:
        return static_cast<CursorType>(static_cast<uint16>(valueID)
                                       - static_cast<uint16>(css::ValueId::Auto));
    }
  }

  KRYS_NODISCARD constexpr css::ValueId ToValueId(DisplayType e) noexcept
  {
    switch (e)
    {
      case DisplayType::Inline:           return css::ValueId::Inline;
      case DisplayType::Block:            return css::ValueId::Block;
      case DisplayType::ListItem:         return css::ValueId::ListItem;
      case DisplayType::InlineBlock:      return css::ValueId::InlineBlock;
      case DisplayType::Table:            return css::ValueId::Table;
      case DisplayType::InlineTable:      return css::ValueId::InlineTable;
      case DisplayType::TableRowGroup:    return css::ValueId::TableRowGroup;
      case DisplayType::TableHeaderGroup: return css::ValueId::TableHeaderGroup;
      case DisplayType::TableFooterGroup: return css::ValueId::TableFooterGroup;
      case DisplayType::TableRow:         return css::ValueId::TableRow;
      case DisplayType::TableColumnGroup: return css::ValueId::TableColumnGroup;
      case DisplayType::TableColumn:      return css::ValueId::TableColumn;
      case DisplayType::TableCell:        return css::ValueId::TableCell;
      case DisplayType::TableCaption:     return css::ValueId::TableCaption;
      case DisplayType::Box:              return css::ValueId::WebkitBox;
      case DisplayType::InlineBox:        return css::ValueId::WebkitInlineBox;
      case DisplayType::Flex:             return css::ValueId::Flex;
      case DisplayType::InlineFlex:       return css::ValueId::InlineFlex;
      case DisplayType::Grid:             return css::ValueId::Grid;
      case DisplayType::InlineGrid:       return css::ValueId::InlineGrid;
      case DisplayType::GridLanes:        return css::ValueId::GridLanes;
      case DisplayType::InlineGridLanes:  return css::ValueId::InlineGridLanes;
      case DisplayType::None:             return css::ValueId::None;
      case DisplayType::Contents:         return css::ValueId::Contents;
      case DisplayType::FlowRoot:         return css::ValueId::FlowRoot;
      case DisplayType::Ruby:             return css::ValueId::Ruby;
      case DisplayType::RubyBlock:        return css::ValueId::BlockRuby;
      case DisplayType::RubyBase:         return css::ValueId::RubyBase;
      case DisplayType::RubyAnnotation:   return css::ValueId::RubyText;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr DisplayType FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Inline:           return DisplayType::Inline;
      case css::ValueId::Block:            return DisplayType::Block;
      case css::ValueId::ListItem:         return DisplayType::ListItem;
      case css::ValueId::InlineBlock:      return DisplayType::InlineBlock;
      case css::ValueId::Table:            return DisplayType::Table;
      case css::ValueId::InlineTable:      return DisplayType::InlineTable;
      case css::ValueId::TableRowGroup:    return DisplayType::TableRowGroup;
      case css::ValueId::TableHeaderGroup: return DisplayType::TableHeaderGroup;
      case css::ValueId::TableFooterGroup: return DisplayType::TableFooterGroup;
      case css::ValueId::TableRow:         return DisplayType::TableRow;
      case css::ValueId::TableColumnGroup: return DisplayType::TableColumnGroup;
      case css::ValueId::TableColumn:      return DisplayType::TableColumn;
      case css::ValueId::TableCell:        return DisplayType::TableCell;
      case css::ValueId::TableCaption:     return DisplayType::TableCaption;
      case css::ValueId::WebkitBox:        return DisplayType::Box;
      case css::ValueId::WebkitInlineBox:  return DisplayType::InlineBox;
      case css::ValueId::Flex:             return DisplayType::Flex;
      case css::ValueId::InlineFlex:       return DisplayType::InlineFlex;
      case css::ValueId::Grid:             return DisplayType::Grid;
      case css::ValueId::InlineGrid:       return DisplayType::InlineGrid;
      case css::ValueId::GridLanes:        return DisplayType::GridLanes;
      case css::ValueId::InlineGridLanes:  return DisplayType::InlineGridLanes;
      case css::ValueId::None:             return DisplayType::None;
      case css::ValueId::Contents:         return DisplayType::Contents;
      case css::ValueId::FlowRoot:         return DisplayType::FlowRoot;
      case css::ValueId::Ruby:             return DisplayType::Ruby;
      case css::ValueId::BlockRuby:        return DisplayType::RubyBlock;
      case css::ValueId::RubyBase:         return DisplayType::RubyBase;
      case css::ValueId::RubyText:         return DisplayType::RubyAnnotation;
      default:                             break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return DisplayType::Inline;
  }

#define KRYS_MAPPING_TYPE EmptyCell
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Show) CASE(Hide)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE FlexDirection
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Row) CASE(RowReverse) CASE(Column) CASE(ColumnReverse)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(FlexWrap e) noexcept
  {
    switch (e)
    {
      case FlexWrap::NoWrap:  return css::ValueId::Nowrap;
      case FlexWrap::Wrap:    return css::ValueId::Wrap;
      case FlexWrap::Reverse: return css::ValueId::WrapReverse;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr FlexWrap FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Nowrap:      return FlexWrap::NoWrap;
      case css::ValueId::Wrap:        return FlexWrap::Wrap;
      case css::ValueId::WrapReverse: return FlexWrap::Reverse;
      default:                        break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return FlexWrap::NoWrap;
  }

#define KRYS_MAPPING_TYPE Float
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Left) CASE(Right) CASE(InlineStart) CASE(InlineEnd)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE LineBreak
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Auto) CASE(Loose) CASE(Normal) CASE(Strict) CASE(AfterWhiteSpace) CASE(Anywhere)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ListStylePosition
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Outside) CASE(Inside)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(Overflow e) noexcept
  {
    switch (e)
    {
      case Overflow::Visible: return css::ValueId::Visible;
      case Overflow::Hidden:  return css::ValueId::Hidden;
      case Overflow::Scroll:  return css::ValueId::Scroll;
      case Overflow::Auto:    return css::ValueId::Auto;
      case Overflow::PagedX:  return css::ValueId::WebkitPagedX;
      case Overflow::PagedY:  return css::ValueId::WebkitPagedY;
      case Overflow::Clip:    return css::ValueId::Clip;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr Overflow FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Visible:      return Overflow::Visible;
      case css::ValueId::Hidden:       return Overflow::Hidden;
      case css::ValueId::Scroll:       return Overflow::Scroll;
      case css::ValueId::Overlay:
      case css::ValueId::Auto:         return Overflow::Auto;
      case css::ValueId::WebkitPagedX: return Overflow::PagedX;
      case css::ValueId::WebkitPagedY: return Overflow::PagedY;
      case css::ValueId::Clip:         return Overflow::Clip;
      default:                         break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return Overflow::Visible;
  }

#define KRYS_MAPPING_TYPE OverscrollBehavior
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Contain) CASE(None) CASE(Auto)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE OverflowAnchor
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Auto)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(BreakBetween e) noexcept
  {
    switch (e)
    {
      case BreakBetween::Auto:        return css::ValueId::Auto;
      case BreakBetween::Avoid:       return css::ValueId::Avoid;
      case BreakBetween::AvoidColumn: return css::ValueId::AvoidColumn;
      case BreakBetween::AvoidPage:   return css::ValueId::AvoidPage;
      case BreakBetween::Column:      return css::ValueId::Column;
      case BreakBetween::Page:        return css::ValueId::Page;
      case BreakBetween::LeftPage:    return css::ValueId::Left;
      case BreakBetween::RightPage:   return css::ValueId::Right;
      case BreakBetween::RectoPage:   return css::ValueId::Recto;
      case BreakBetween::VersoPage:   return css::ValueId::Verso;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr BreakBetween FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Auto:        return BreakBetween::Auto;
      case css::ValueId::Avoid:       return BreakBetween::Avoid;
      case css::ValueId::AvoidColumn: return BreakBetween::AvoidColumn;
      case css::ValueId::AvoidPage:   return BreakBetween::AvoidPage;
      case css::ValueId::Column:      return BreakBetween::Column;
      case css::ValueId::Page:        return BreakBetween::Page;
      case css::ValueId::Left:        return BreakBetween::LeftPage;
      case css::ValueId::Right:       return BreakBetween::RightPage;
      case css::ValueId::Recto:       return BreakBetween::RectoPage;
      case css::ValueId::Verso:       return BreakBetween::VersoPage;
      default:                        break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return BreakBetween::Auto;
  }

#define KRYS_MAPPING_TYPE BreakInside
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Avoid) CASE(AvoidColumn) CASE(AvoidPage)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(PositionType e) noexcept
  {
    switch (e)
    {
      case PositionType::Static:   return css::ValueId::Static;
      case PositionType::Relative: return css::ValueId::Relative;
      case PositionType::Absolute: return css::ValueId::Absolute;
      case PositionType::Fixed:    return css::ValueId::Fixed;
      case PositionType::Sticky:   return css::ValueId::Sticky;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr PositionType FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Static:       return PositionType::Static;
      case css::ValueId::Relative:     return PositionType::Relative;
      case css::ValueId::Absolute:     return PositionType::Absolute;
      case css::ValueId::Fixed:        return PositionType::Fixed;
      case css::ValueId::Sticky:
      case css::ValueId::WebkitSticky: return PositionType::Sticky;
      default:                         break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return PositionType::Static;
  }

#define KRYS_MAPPING_TYPE TableLayoutType
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Fixed)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextGroupAlign
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Start) CASE(End) CASE(Left) CASE(Right) CASE(Center)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(TextJustify e) noexcept
  {
    switch (e)
    {
      case TextJustify::Auto:           return css::ValueId::Auto;
      case TextJustify::None:           return css::ValueId::None;
      case TextJustify::InterWord:      return css::ValueId::InterWord;
      case TextJustify::InterCharacter: return css::ValueId::InterCharacter;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr TextJustify FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Auto:           return TextJustify::Auto;
      case css::ValueId::None:           return TextJustify::None;
      case css::ValueId::InterWord:      return TextJustify::InterWord;
      case css::ValueId::InterCharacter:
      case css::ValueId::Distribute:     return TextJustify::InterCharacter;
      default:                           break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return TextJustify::Auto;
  }

#define KRYS_MAPPING_TYPE TextDecorationStyle
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Solid) CASE(Double) CASE(Dotted) CASE(Dashed) CASE(Wavy)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextSecurity
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Disc) CASE(Circle) CASE(Square)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextDecorationSkipInk
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Auto) CASE(All)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(UnicodeBidi e) noexcept
  {
    switch (e)
    {
      case UnicodeBidi::Normal:          return css::ValueId::Normal;
      case UnicodeBidi::Embed:           return css::ValueId::Embed;
      case UnicodeBidi::Override:        return css::ValueId::BidiOverride;
      case UnicodeBidi::Isolate:         return css::ValueId::Isolate;
      case UnicodeBidi::IsolateOverride: return css::ValueId::IsolateOverride;
      case UnicodeBidi::Plaintext:       return css::ValueId::Plaintext;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr UnicodeBidi FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Normal:                return UnicodeBidi::Normal;
      case css::ValueId::Embed:                 return UnicodeBidi::Embed;
      case css::ValueId::BidiOverride:          return UnicodeBidi::Override;
      case css::ValueId::Isolate:
      case css::ValueId::WebkitIsolate:         return UnicodeBidi::Isolate;
      case css::ValueId::IsolateOverride:
      case css::ValueId::WebkitIsolateOverride: return UnicodeBidi::IsolateOverride;
      case css::ValueId::Plaintext:
      case css::ValueId::WebkitPlaintext:       return UnicodeBidi::Plaintext;
      default:                                  break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return UnicodeBidi::Normal;
  }

#define KRYS_MAPPING_TYPE UserDrag
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(None) CASE(Element)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE UserModify
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(ReadOnly) CASE(ReadWrite) CASE(ReadWritePlaintextOnly)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(UserSelect e) noexcept
  {
    switch (e)
    {
      case UserSelect::None: return css::ValueId::None;
      case UserSelect::Text: return css::ValueId::Text;
      case UserSelect::All:  return css::ValueId::All;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr UserSelect FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Auto: return UserSelect::Text;
      case css::ValueId::None: return UserSelect::None;
      case css::ValueId::Text: return UserSelect::Text;
      case css::ValueId::All:  return UserSelect::All;
      default:                 break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return UserSelect::Text;
  }

#define KRYS_MAPPING_TYPE Visibility
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Visible) CASE(Hidden) CASE(Collapse)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE WhiteSpaceCollapse
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Collapse) CASE(Preserve) CASE(PreserveBreaks) CASE(BreakSpaces)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE WordBreak
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(BreakAll) CASE(KeepAll) CASE(BreakWord) CASE(AutoPhrase)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE OverflowWrap
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(Anywhere) CASE(BreakWord)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(TextDirection e) noexcept
  {
    switch (e)
    {
      case TextDirection::LTR: return css::ValueId::Ltr;
      case TextDirection::RTL: return css::ValueId::Rtl;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr TextDirection FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Ltr: return TextDirection::LTR;
      case css::ValueId::Rtl: return TextDirection::RTL;
      default:                break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return TextDirection::LTR;
  }

  KRYS_NODISCARD constexpr css::ValueId ToValueId(StyleWritingMode e) noexcept
  {
    switch (e)
    {
      case StyleWritingMode::HorizontalTb: return css::ValueId::HorizontalTb;
      case StyleWritingMode::VerticalRl:   return css::ValueId::VerticalRl;
      case StyleWritingMode::VerticalLr:   return css::ValueId::VerticalLr;
      case StyleWritingMode::SidewaysRl:   return css::ValueId::SidewaysRl;
      case StyleWritingMode::SidewaysLr:   return css::ValueId::SidewaysLr;
      case StyleWritingMode::HorizontalBt: return css::ValueId::HorizontalBt;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr StyleWritingMode FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::HorizontalTb:
      case css::ValueId::Lr:
      case css::ValueId::LrTb:
      case css::ValueId::Rl:
      case css::ValueId::RlTb:         return StyleWritingMode::HorizontalTb;
      case css::ValueId::VerticalRl:
      case css::ValueId::Tb:
      case css::ValueId::TbRl:         return StyleWritingMode::VerticalRl;
      case css::ValueId::VerticalLr:   return StyleWritingMode::VerticalLr;
      case css::ValueId::SidewaysLr:   return StyleWritingMode::SidewaysLr;
      case css::ValueId::SidewaysRl:   return StyleWritingMode::SidewaysRl;
      case css::ValueId::HorizontalBt: return StyleWritingMode::HorizontalBt;
      default:                         break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return StyleWritingMode::HorizontalTb;
  }

  KRYS_NODISCARD constexpr css::ValueId ToValueId(TextCombine e) noexcept
  {
    switch (e)
    {
      case TextCombine::None: return css::ValueId::None;
      case TextCombine::All:  return css::ValueId::All;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr TextCombine FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::None: return TextCombine::None;
      case css::ValueId::All:
      case css::ValueId::Horizontal: // -webkit-text-combine only
        return TextCombine::All;
      default: break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return TextCombine::None;
  }

  KRYS_NODISCARD constexpr css::ValueId ToValueId(RubyPosition e) noexcept
  {
    switch (e)
    {
      case RubyPosition::Over:                 return css::ValueId::Over;
      case RubyPosition::Under:                return css::ValueId::Under;
      case RubyPosition::InterCharacter:       return css::ValueId::InterCharacter;
      case RubyPosition::LegacyInterCharacter: return css::ValueId::LegacyInterCharacter;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr RubyPosition FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Over:
      case css::ValueId::Before: // -webkit-ruby-position only
        return RubyPosition::Over;
      case css::ValueId::Under:
      case css::ValueId::After: // -webkit-ruby-position only
        return RubyPosition::Under;
      case css::ValueId::InterCharacter:       return RubyPosition::InterCharacter;
      case css::ValueId::LegacyInterCharacter: return RubyPosition::LegacyInterCharacter;
      default:                                 break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return RubyPosition::Over;
  }

#define KRYS_MAPPING_TYPE RubyAlign
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Start) CASE(Center) CASE(SpaceBetween) CASE(SpaceAround)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE RubyOverhang
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(None)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextOverflow
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Clip) CASE(Ellipsis)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(TextWrapMode wrap) noexcept
  {
    switch (wrap)
    {
      case TextWrapMode::Wrap:   return css::ValueId::Wrap;
      case TextWrapMode::NoWrap: return css::ValueId::Nowrap;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr TextWrapMode FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Wrap:   return TextWrapMode::Wrap;
      case css::ValueId::Nowrap: return TextWrapMode::NoWrap;
      default:                   break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return TextWrapMode::Wrap;
  }

#define KRYS_MAPPING_TYPE TextWrapStyle
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Balance) CASE(Pretty) CASE(Stable)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextEmphasisFill
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Filled) CASE(Open)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(TextEmphasisMark mark) noexcept
  {
    switch (mark)
    {
      case TextEmphasisMark::Dot:          return css::ValueId::Dot;
      case TextEmphasisMark::Circle:       return css::ValueId::Circle;
      case TextEmphasisMark::DoubleCircle: return css::ValueId::DoubleCircle;
      case TextEmphasisMark::Triangle:     return css::ValueId::Triangle;
      case TextEmphasisMark::Sesame:       return css::ValueId::Sesame;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr TextEmphasisMark FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Dot:          return TextEmphasisMark::Dot;
      case css::ValueId::Circle:       return TextEmphasisMark::Circle;
      case css::ValueId::DoubleCircle: return TextEmphasisMark::DoubleCircle;
      case css::ValueId::Triangle:     return TextEmphasisMark::Triangle;
      case css::ValueId::Sesame:       return TextEmphasisMark::Sesame;
      default:                         break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return TextEmphasisMark::Dot;
  }

#define KRYS_MAPPING_TYPE TextOrientation
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Sideways) CASE(Mixed) CASE(Upright)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE PointerEvents
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(None)                                                                                                 \
  CASE(Stroke)                                                                                               \
  CASE(Fill)                                                                                                 \
  CASE(Painted)                                                                                              \
  CASE(Visible)                                                                                              \
  CASE(VisibleStroke) CASE(VisibleFill) CASE(VisiblePainted) CASE(BoundingBox) CASE(Auto) CASE(All)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(Kerning kerning) noexcept
  {
    switch (kerning)
    {
      case Kerning::Auto:    return css::ValueId::Auto;
      case Kerning::Normal:  return css::ValueId::Normal;
      case Kerning::NoShift: return css::ValueId::None;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr Kerning FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Auto:   return Kerning::Auto;
      case css::ValueId::Normal: return Kerning::Normal;
      case css::ValueId::None:   return Kerning::NoShift;
      default:                   break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return Kerning::Auto;
  }

#define KRYS_MAPPING_TYPE ObjectFit
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Fill) CASE(Contain) CASE(Cover) CASE(None) CASE(ScaleDown)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE FontSmoothingMode
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(None) CASE(Antialiased) CASE(SubpixelAntialiased)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextRenderingMode
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Auto) CASE(OptimizeSpeed) CASE(OptimizeLegibility) CASE(GeometricPrecision)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE Hyphens
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Manual) CASE(Auto)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE LineAlign
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Edges)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE Order
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Logical) CASE(Visual)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE BlendMode
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Normal)                                                                                               \
  CASE(Multiply)                                                                                             \
  CASE(Screen)                                                                                               \
  CASE(Overlay)                                                                                              \
  CASE(Darken)                                                                                               \
  CASE(Lighten)                                                                                              \
  CASE(ColorDodge)                                                                                           \
  CASE(ColorBurn)                                                                                            \
  CASE(HardLight)                                                                                            \
  CASE(SoftLight)                                                                                            \
  CASE(Difference)                                                                                           \
  CASE(Exclusion) CASE(Hue) CASE(Saturation) CASE(Color) CASE(Luminosity) CASE(PlusDarker) CASE(PlusLighter)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE Isolation
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Isolate)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE LineCap
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Butt) CASE(Round) CASE(Square)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE LineJoin
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Miter) CASE(Round) CASE(Bevel)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(WindRule e) noexcept
  {
    switch (e)
    {
      case WindRule::NonZero: return css::ValueId::Nonzero;
      case WindRule::EvenOdd: return css::ValueId::Evenodd;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr WindRule FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Nonzero: return WindRule::NonZero;
      case css::ValueId::Evenodd: return WindRule::EvenOdd;
      default:                    break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return WindRule::NonZero;
  }

#define KRYS_MAPPING_TYPE AlignmentBaseline
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(AfterEdge)                                                                                            \
  CASE(Alphabetic)                                                                                           \
  CASE(Baseline)                                                                                             \
  CASE(BeforeEdge)                                                                                           \
  CASE(Central)                                                                                              \
  CASE(Hanging) CASE(Ideographic) CASE(Mathematical) CASE(Middle) CASE(TextAfterEdge) CASE(TextBeforeEdge)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE BorderCollapse
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Separate) CASE(Collapse)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(ImageRendering imageRendering) noexcept
  {
    switch (imageRendering)
    {
      case ImageRendering::Auto:            return css::ValueId::Auto;
      case ImageRendering::CrispEdges:      return css::ValueId::CrispEdges;
      case ImageRendering::Pixelated:       return css::ValueId::Pixelated;
      case ImageRendering::OptimizeSpeed:   return css::ValueId::OptimizeSpeed;
      case ImageRendering::OptimizeQuality: return css::ValueId::OptimizeQuality;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr ImageRendering FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Auto:                   return ImageRendering::Auto;
      case css::ValueId::WebkitOptimizeContrast:
      case css::ValueId::CrispEdges:
      case css::ValueId::WebkitCrispEdges:       return ImageRendering::CrispEdges;
      case css::ValueId::Pixelated:              return ImageRendering::Pixelated;
      case css::ValueId::OptimizeSpeed:          return ImageRendering::OptimizeSpeed;
      case css::ValueId::OptimizeQuality:        return ImageRendering::OptimizeQuality;
      default:                                   break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return ImageRendering::Auto;
  }

#define KRYS_MAPPING_TYPE InputSecurity
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(None)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(TransformStyle3D e) noexcept
  {
    switch (e)
    {
      case TransformStyle3D::Flat:       return css::ValueId::Flat;
      case TransformStyle3D::Preserve3D: return css::ValueId::Preserve3d;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr TransformStyle3D FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Flat:       return TransformStyle3D::Flat;
      case css::ValueId::Preserve3d: return TransformStyle3D::Preserve3D;
      default:                       break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return TransformStyle3D::Flat;
  }

#define KRYS_MAPPING_TYPE TransformBox
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(StrokeBox) CASE(ContentBox) CASE(BorderBox) CASE(FillBox) CASE(ViewBox)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ColumnProgression
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(Reverse)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE BufferedRendering
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Dynamic) CASE(Static)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ColorInterpolation
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(SRGB) CASE(LinearRGB)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE DominantBaseline
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Auto)                                                                                                 \
  CASE(UseScript)                                                                                            \
  CASE(NoChange)                                                                                             \
  CASE(ResetSize)                                                                                            \
  CASE(Central)                                                                                              \
  CASE(Middle)                                                                                               \
  CASE(TextBeforeEdge) CASE(TextAfterEdge) CASE(Ideographic) CASE(Alphabetic) CASE(Hanging) CASE(Mathematical)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(ShapeRendering e) noexcept
  {
    switch (e)
    {
      case ShapeRendering::Auto:               return css::ValueId::Auto;
      case ShapeRendering::OptimizeSpeed:      return css::ValueId::OptimizeSpeed;
      case ShapeRendering::CrispEdges:         return css::ValueId::Crispedges; // "crispedges", not "crisp-edges"
      case ShapeRendering::GeometricPrecision: return css::ValueId::GeometricPrecision;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr ShapeRendering FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Auto:          return ShapeRendering::Auto;
      case css::ValueId::OptimizeSpeed: return ShapeRendering::OptimizeSpeed;
      case css::ValueId::Crispedges: // "crispedges", not "crisp-edges"
        return ShapeRendering::CrispEdges;
      case css::ValueId::GeometricPrecision: return ShapeRendering::GeometricPrecision;
      default:                               break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return ShapeRendering::Auto;
  }

#define KRYS_MAPPING_TYPE TextAnchor
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Start) CASE(Middle) CASE(End)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE VectorEffect
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(NonScalingStroke)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE MaskType
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Luminance) CASE(Alpha)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(CSSBoxType cssBox) noexcept
  {
    switch (cssBox)
    {
      case CSSBoxType::MarginBox:  return css::ValueId::MarginBox;
      case CSSBoxType::BorderBox:  return css::ValueId::BorderBox;
      case CSSBoxType::PaddingBox: return css::ValueId::PaddingBox;
      case CSSBoxType::ContentBox: return css::ValueId::ContentBox;
      case CSSBoxType::FillBox:    return css::ValueId::FillBox;
      case CSSBoxType::StrokeBox:  return css::ValueId::StrokeBox;
      case CSSBoxType::ViewBox:    return css::ValueId::ViewBox;
      case CSSBoxType::BoxMissing:
        krys_debug_assert_msg(false, "Unexpected value");
        return css::ValueId::None;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr CSSBoxType FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::MarginBox:  return CSSBoxType::MarginBox;
      case css::ValueId::BorderBox:  return CSSBoxType::BorderBox;
      case css::ValueId::PaddingBox: return CSSBoxType::PaddingBox;
      case css::ValueId::ContentBox: return CSSBoxType::ContentBox;
      // The following are used in an SVG context.
      case css::ValueId::FillBox:    return CSSBoxType::FillBox;
      case css::ValueId::StrokeBox:  return CSSBoxType::StrokeBox;
      case css::ValueId::ViewBox:    return CSSBoxType::ViewBox;
      default:                       break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return CSSBoxType::BoxMissing;
  }

#define KRYS_MAPPING_TYPE ItemPosition
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Legacy)                                                                                               \
  CASE(Auto)                                                                                                 \
  CASE(Normal)                                                                                               \
  CASE(Stretch)                                                                                              \
  CASE(Baseline)                                                                                             \
  CASE(LastBaseline)                                                                                         \
  CASE(Center)                                                                                               \
  CASE(Start)                                                                                                \
  CASE(End)                                                                                                  \
  CASE(SelfStart) CASE(SelfEnd) CASE(FlexStart) CASE(FlexEnd) CASE(Left) CASE(Right) CASE(AnchorCenter)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE OverflowAlignment
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Default) CASE(Unsafe) CASE(Safe)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ContentPosition
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Normal)                                                                                               \
  CASE(Baseline)                                                                                             \
  CASE(LastBaseline) CASE(Center) CASE(Start) CASE(End) CASE(FlexStart) CASE(FlexEnd) CASE(Left) CASE(Right)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ContentDistribution
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Default) CASE(SpaceBetween) CASE(SpaceAround) CASE(SpaceEvenly) CASE(Stretch)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ScrollSnapStrictness
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Proximity) CASE(Mandatory)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(ScrollSnapAxis axis) noexcept
  {
    switch (axis)
    {
      case ScrollSnapAxis::XAxis:  return css::ValueId::X;
      case ScrollSnapAxis::YAxis:  return css::ValueId::Y;
      case ScrollSnapAxis::Block:  return css::ValueId::Block;
      case ScrollSnapAxis::Inline: return css::ValueId::Inline;
      case ScrollSnapAxis::Both:   return css::ValueId::Both;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr ScrollSnapAxis FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::X:      return ScrollSnapAxis::XAxis;
      case css::ValueId::Y:      return ScrollSnapAxis::YAxis;
      case css::ValueId::Block:  return ScrollSnapAxis::Block;
      case css::ValueId::Inline: return ScrollSnapAxis::Inline;
      case css::ValueId::Both:   return ScrollSnapAxis::Both;
      default:                   break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return ScrollSnapAxis::Both;
  }

#define KRYS_MAPPING_TYPE ScrollSnapAxisAlignType
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Start) CASE(Center) CASE(End)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ScrollSnapStop
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(Always)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(FontVariantPosition position) noexcept
  {
    switch (position)
    {
      case FontVariantPosition::Normal:      return css::ValueId::Normal;
      case FontVariantPosition::Subscript:   return css::ValueId::Sub;
      case FontVariantPosition::Superscript: return css::ValueId::Super;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr FontVariantPosition FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Normal: return FontVariantPosition::Normal;
      case css::ValueId::Sub:    return FontVariantPosition::Subscript;
      case css::ValueId::Super:  return FontVariantPosition::Superscript;
      default:                   krys_debug_assert_msg(false, "Unexpected value"); return FontVariantPosition::Normal;
    }
  }

  KRYS_NODISCARD constexpr css::ValueId ToValueId(FontVariantCaps caps) noexcept
  {
    switch (caps)
    {
      case FontVariantCaps::Normal:    return css::ValueId::Normal;
      case FontVariantCaps::Small:     return css::ValueId::SmallCaps;
      case FontVariantCaps::AllSmall:  return css::ValueId::AllSmallCaps;
      case FontVariantCaps::Petite:    return css::ValueId::PetiteCaps;
      case FontVariantCaps::AllPetite: return css::ValueId::AllPetiteCaps;
      case FontVariantCaps::Unicase:   return css::ValueId::Unicase;
      case FontVariantCaps::Titling:   return css::ValueId::TitlingCaps;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr FontVariantCaps FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::Normal:        return FontVariantCaps::Normal;
      case css::ValueId::SmallCaps:     return FontVariantCaps::Small;
      case css::ValueId::AllSmallCaps:  return FontVariantCaps::AllSmall;
      case css::ValueId::PetiteCaps:    return FontVariantCaps::Petite;
      case css::ValueId::AllPetiteCaps: return FontVariantCaps::AllPetite;
      case css::ValueId::Unicase:       return FontVariantCaps::Unicase;
      case css::ValueId::TitlingCaps:   return FontVariantCaps::Titling;
      default:                          break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return FontVariantCaps::Normal;
  }

#define KRYS_MAPPING_TYPE FontVariantEmoji
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(Text) CASE(Emoji) CASE(Unicode)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE FontOpticalSizing
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(None)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  template <>
  KRYS_NODISCARD constexpr css::FontTechnology FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::ColorColrv0:      return css::FontTechnology::ColorColrv0;
      case css::ValueId::ColorColrv1:      return css::FontTechnology::ColorColrv1;
      case css::ValueId::ColorCbdt:        return css::FontTechnology::ColorCbdt;
      case css::ValueId::ColorSbix:        return css::FontTechnology::ColorSbix;
      case css::ValueId::ColorSvg:         return css::FontTechnology::ColorSvg;
      case css::ValueId::FeaturesAat:      return css::FontTechnology::FeaturesAat;
      case css::ValueId::FeaturesGraphite: return css::FontTechnology::FeaturesGraphite;
      case css::ValueId::FeaturesOpentype: return css::FontTechnology::FeaturesOpentype;
      case css::ValueId::Incremental:      return css::FontTechnology::Incremental;
      case css::ValueId::Palettes:         return css::FontTechnology::Palettes;
      case css::ValueId::Variations:       return css::FontTechnology::Variations;
      default:                             break;
    }
    return css::FontTechnology::Invalid;
  }

#define KRYS_MAPPING_TYPE FontSynthesisLonghandValue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(None)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE FontLoadingBehavior
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Block) CASE(Swap) CASE(Fallback) CASE(Optional)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE MathShift
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(Compact)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE MathStyle
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(Compact)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ContainerType
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(Size) CASE(InlineSize)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ContentVisibility
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Visible) CASE(Hidden) CASE(Auto)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE ScrollAxis
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Block) CASE(Inline) CASE(X) CASE(Y)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE QuoteType
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(OpenQuote) CASE(CloseQuote) CASE(NoOpenQuote) CASE(NoCloseQuote)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE OverflowContinue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Discard)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE Scroller
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Nearest) CASE(Root) CASE(Self)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE NinePieceImageRule
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Stretch) CASE(Round) CASE(Space) CASE(Repeat)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE AnimationDirection
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(Alternate) CASE(Reverse) CASE(AlternateReverse)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE AnimationFillMode
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(None) CASE(Forwards) CASE(Backwards) CASE(Both)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE AnimationPlayState
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Running) CASE(Paused)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE CompositeOperation
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Replace) CASE(Add) CASE(Accumulate)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TransitionBehavior
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Normal) CASE(AllowDiscrete)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextEdgeOver
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Text) CASE(Ideographic) CASE(IdeographicInk) CASE(Cap) CASE(Ex)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextEdgeUnder
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Text) CASE(Ideographic) CASE(IdeographicInk) CASE(Alphabetic)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE TextSpacingTrim::TrimType
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(SpaceAll) CASE(TrimAll) CASE(Auto)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::SpeakAsValue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(SpellOut) CASE(Digits) CASE(LiteralPunctuation) CASE(NoPunctuation)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::TouchActionValue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(PanX) CASE(PanY) CASE(PinchZoom)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::ScrollbarWidth
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Thin) CASE(None)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::ScrollBehavior
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Auto) CASE(Smooth)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::MarginTrimSide
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(BlockStart) CASE(InlineStart) CASE(BlockEnd) CASE(InlineEnd)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::ContainValue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Size) CASE(InlineSize) CASE(Layout) CASE(Style) CASE(Paint)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::Resize
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Both) CASE(Horizontal) CASE(Vertical) CASE(Block) CASE(Inline) CASE(None)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(style::TextAlign e) noexcept
  {
    switch (e)
    {
      case style::TextAlign::Start:        return css::ValueId::Start;
      case style::TextAlign::End:          return css::ValueId::End;
      case style::TextAlign::Left:         return css::ValueId::Left;
      case style::TextAlign::Right:        return css::ValueId::Right;
      case style::TextAlign::Center:       return css::ValueId::Center;
      case style::TextAlign::Justify:      return css::ValueId::Justify;
      case style::TextAlign::WebKitLeft:   return css::ValueId::WebkitLeft;
      case style::TextAlign::WebKitRight:  return css::ValueId::WebkitRight;
      case style::TextAlign::WebKitCenter: return css::ValueId::WebkitCenter;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Invalid;
  }

  template <>
  KRYS_NODISCARD constexpr style::TextAlign FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::WebkitAuto: // Legacy -webkit-auto. Equivalent to start.
      case css::ValueId::Start:        return style::TextAlign::Start;
      case css::ValueId::End:          return style::TextAlign::End;
      case css::ValueId::Left:         return style::TextAlign::Left;
      case css::ValueId::Right:        return style::TextAlign::Right;
      case css::ValueId::Center:       return style::TextAlign::Center;
      case css::ValueId::Justify:      return style::TextAlign::Justify;
      case css::ValueId::WebkitLeft:   return style::TextAlign::WebKitLeft;
      case css::ValueId::WebkitRight:  return style::TextAlign::WebKitRight;
      case css::ValueId::WebkitCenter: return style::TextAlign::WebKitCenter;
      default:                         break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return style::TextAlign::Start;
  }

  template <>
  struct style::ValueRepresentation<style::TextAlign>
  {
    template <typename... F>
    constexpr decltype(auto) operator()(style::TextAlign value, F &&...f)
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      switch (value)
      {
        case style::TextAlign::Start:        return visitor(css::keywords::Start {});
        case style::TextAlign::End:          return visitor(css::keywords::End {});
        case style::TextAlign::Left:         return visitor(css::keywords::Left {});
        case style::TextAlign::Right:        return visitor(css::keywords::Right {});
        case style::TextAlign::Center:       return visitor(css::keywords::Center {});
        case style::TextAlign::Justify:      return visitor(css::keywords::Justify {});
        case style::TextAlign::WebKitLeft:   return visitor(css::keywords::WebkitLeft {});
        case style::TextAlign::WebKitRight:  return visitor(css::keywords::WebkitRight {});
        case style::TextAlign::WebKitCenter: return visitor(css::keywords::WebkitCenter {});
      }
      krys_unreachable();
    }
  };

#define KRYS_MAPPING_TYPE style::TextAlignLast
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Start) CASE(End) CASE(Left) CASE(Right) CASE(Center) CASE(Justify) CASE(Auto)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::HangingPunctuationValue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(First) CASE(ForceEnd) CASE(AllowEnd) CASE(Last)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::TextTransformValue
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Capitalize) CASE(Uppercase) CASE(Lowercase) CASE(FullWidth) CASE(FullSizeKana) CASE(MathAuto)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

  KRYS_NODISCARD constexpr css::ValueId ToValueId(FontSizeAdjust::Metric metric) noexcept
  {
    switch (metric)
    {
      case FontSizeAdjust::Metric::ExHeight:  return css::ValueId::ExHeight;
      case FontSizeAdjust::Metric::CapHeight: return css::ValueId::CapHeight;
      case FontSizeAdjust::Metric::ChWidth:   return css::ValueId::ChWidth;
      case FontSizeAdjust::Metric::IcWidth:   return css::ValueId::IcWidth;
      case FontSizeAdjust::Metric::IcHeight:  return css::ValueId::IcHeight;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return css::ValueId::Auto;
  }

  template <>
  KRYS_NODISCARD constexpr FontSizeAdjust::Metric FromValueId(css::ValueId valueID) noexcept
  {
    switch (valueID)
    {
      case css::ValueId::ExHeight:  return FontSizeAdjust::Metric::ExHeight;
      case css::ValueId::CapHeight: return FontSizeAdjust::Metric::CapHeight;
      case css::ValueId::ChWidth:   return FontSizeAdjust::Metric::ChWidth;
      case css::ValueId::IcWidth:   return FontSizeAdjust::Metric::IcWidth;
      case css::ValueId::IcHeight:  return FontSizeAdjust::Metric::IcHeight;
      default:                      break;
    }
    krys_debug_assert_msg(false, "Unexpected value");
    return FontSizeAdjust::Metric::ExHeight;
  }

#define KRYS_MAPPING_TYPE style::PositionVisibilityValue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(AnchorsValid) CASE(AnchorsVisible) CASE(NoOverflow)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::PositionTryFallbackTactic
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(FlipBlock) CASE(FlipInline) CASE(FlipStart) CASE(FlipX) CASE(FlipY)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::ImageOrientation
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(FromImage) CASE(None)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::MaskMode
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Alpha) CASE(Luminance) CASE(MatchSource)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::TextDecorationLine::Flag
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Underline) CASE(Overline) CASE(LineThrough) CASE(Blink)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::TextEmphasisPositionValue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(Over) CASE(Under) CASE(Left) CASE(Right)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::TextUnderlinePositionValue
#define KRYS_MAPPING_FOR_EACH(CASE) CASE(FromFont) CASE(Under) CASE(Left) CASE(Right)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::WebkitLineBoxContainValue
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Block) CASE(Inline) CASE(Font) CASE(Glyphs) CASE(Replaced) CASE(InlineBox) CASE(InitialLetter)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#define KRYS_MAPPING_TYPE style::PositionTryOrder
#define KRYS_MAPPING_FOR_EACH(CASE)                                                                          \
  CASE(Normal) CASE(MostWidth) CASE(MostHeight) CASE(MostBlockSize) CASE(MostInlineSize)
  KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
#undef KRYS_MAPPING_TYPE
#undef KRYS_MAPPING_FOR_EACH

#undef KRYS_EMIT_TO_VALUE_ID_SWITCH_CASE
#undef KRYS_EMIT_FROM_VALUE_ID_SWITCH_CASE
#undef KRYS_EMIT_VALUE_REPRESENTATION_SWITCH_CASE
#undef KRYS_DEFINE_TO_VALUE_ID_FUNCTION
#undef KRYS_DEFINE_FROM_VALUE_ID_FUNCTION
#undef KRYS_DEFINE_VALUE_REPRESENTATION_FUNCTION
#undef KRYS_DEFINE_TO_FROM_VALUE_ID_FUNCTIONS
}
