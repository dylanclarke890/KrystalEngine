#include "Krystal.UI/Layout/Api/NodeStyle.hpp"
#include "Krystal.Lib/StronglyTypedValue.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include "Krystal.UI/Styles/Style.hpp"

namespace
{
  using namespace Krys;
  using namespace Krys::UI;

  struct DirtiesLayout : public StronglyTypedBool<DirtiesLayout>
  {
    using StronglyTypedBool<DirtiesLayout>::StronglyTypedBool;
  };

  struct DirtiesStyle : public StronglyTypedBool<DirtiesStyle>
  {
    using StronglyTypedBool<DirtiesStyle>::StronglyTypedBool;
  };

  template <auto GetterT, auto SetterT, DirtiesLayout dirtiesLayout, DirtiesStyle dirtiesStyle,
            typename ValueT>
  void UpdateProperty(NodeRef node, ValueT value)
  {
    auto &style = node->GetStyle();
    if ((style.*GetterT)() != value)
    {
      (style.*SetterT)(value);
      if constexpr (dirtiesLayout)
      {
        node->MarkLayoutDirtyAndPropagate();
      }

      if constexpr (dirtiesStyle)
      {
        node->MarkStyleDirtyAndPropagate();
      }
    }
  }

  template <auto GetterT, auto SetterT, DirtiesLayout dirtiesLayout, DirtiesStyle dirtiesStyle, typename IdxT,
            typename ValueT>
  void UpdateProperty(NodeRef node, IdxT idx, ValueT value)
  {
    auto &style = node->GetStyle();
    if ((style.*GetterT)(idx) != value)
    {
      (style.*SetterT)(idx, value);
      if constexpr (dirtiesLayout)
      {
        node->MarkLayoutDirtyAndPropagate();
      }

      if constexpr (dirtiesStyle)
      {
        node->MarkStyleDirtyAndPropagate();
      }
    }
  }
}

namespace Krys::UI
{
  void NodeCopyStyle(NodeRef dstNode, NodeConstRef srcNode)
  {
    if (dstNode->GetStyle() != srcNode->GetStyle())
    {
      dstNode->SetStyle(srcNode->GetStyle());
      // TODO: Optimize by only marking dirty if properties that affect layout/style have changed.
      dstNode->MarkLayoutDirtyAndPropagate();
      dstNode->MarkStyleDirtyAndPropagate();
    }
  }

  void NodeStyleSetDirection(NodeRef node, Direction direction)
  {
    UpdateProperty<&Style::GetDirection, &Style::SetDirection, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, direction);
  }

  Direction NodeStyleGetDirection(NodeConstRef node)
  {
    return node->GetStyle().GetDirection();
  }

  void NodeStyleSetFlexDirection(NodeRef node, FlexDirection flexDirection)
  {
    UpdateProperty<&Style::GetFlexDirection, &Style::SetFlexDirection, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, flexDirection);
  }

  FlexDirection NodeStyleGetFlexDirection(NodeConstRef node)
  {
    return node->GetStyle().GetFlexDirection();
  }

  void NodeStyleSetJustifyContent(NodeRef node, Justify justifyContent)
  {
    UpdateProperty<&Style::GetJustifyContent, &Style::SetJustifyContent, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, justifyContent);
  }

  Justify NodeStyleGetJustifyContent(NodeConstRef node)
  {
    return node->GetStyle().GetJustifyContent();
  }

  void NodeStyleSetAlignContent(NodeRef node, Align alignContent)
  {
    UpdateProperty<&Style::GetAlignContent, &Style::SetAlignContent, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, alignContent);
  }

  Align NodeStyleGetAlignContent(NodeConstRef node)
  {
    return node->GetStyle().GetAlignContent();
  }

  void NodeStyleSetAlignItems(NodeRef node, Align alignItems)
  {
    UpdateProperty<&Style::GetAlignItems, &Style::SetAlignItems, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, alignItems);
  }

  Align NodeStyleGetAlignItems(NodeConstRef node)
  {
    return node->GetStyle().GetAlignItems();
  }

  void NodeStyleSetAlignSelf(NodeRef node, Align alignSelf)
  {
    UpdateProperty<&Style::GetAlignSelf, &Style::SetAlignSelf, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, alignSelf);
  }

  Align NodeStyleGetAlignSelf(NodeConstRef node)
  {
    return node->GetStyle().GetAlignSelf();
  }

  void NodeStyleSetPositionType(NodeRef node, PositionType positionType)
  {
    UpdateProperty<&Style::GetPositionType, &Style::SetPositionType, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, positionType);
  }

  PositionType NodeStyleGetPositionType(NodeConstRef node)
  {
    return node->GetStyle().GetPositionType();
  }

  void NodeStyleSetFlexWrap(NodeRef node, Wrap flexWrap)
  {
    UpdateProperty<&Style::GetFlexWrap, &Style::SetFlexWrap, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, flexWrap);
  }

  Wrap NodeStyleGetFlexWrap(NodeConstRef node)
  {
    return node->GetStyle().GetFlexWrap();
  }

  void NodeStyleSetOverflow(NodeRef node, Overflow overflow)
  {
    UpdateProperty<&Style::GetOverflow, &Style::SetOverflow, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, overflow);
  }

  Overflow NodeStyleGetOverflow(NodeConstRef node)
  {
    return node->GetStyle().GetOverflow();
  }

  void NodeStyleSetDisplay(NodeRef node, Display display)
  {
    UpdateProperty<&Style::GetDisplay, &Style::SetDisplay, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, display);
  }

  Display NodeStyleGetDisplay(NodeConstRef node)
  {
    return node->GetStyle().GetDisplay();
  }

  void NodeStyleSetFlex(NodeRef node, float flex)
  {
    UpdateProperty<&Style::GetFlex, &Style::SetFlex, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, NullableFloat {flex});
  }

  float NodeStyleGetFlex(NodeConstRef node)
  {
    return node->GetStyle().GetFlex().Value();
  }

  void NodeStyleSetFlexGrow(NodeRef node, float flexGrow)
  {
    UpdateProperty<&Style::GetFlexGrow, &Style::SetFlexGrow, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, NullableFloat {flexGrow});
  }

  float NodeStyleGetFlexGrow(NodeConstRef node)
  {
    return node->GetStyle().GetFlexGrow().ValueOrDefault(Style::DefaultFlexGrow);
  }

  void NodeStyleSetFlexShrink(NodeRef node, float flexShrink)
  {
    UpdateProperty<&Style::GetFlexShrink, &Style::SetFlexShrink, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, NullableFloat {flexShrink});
  }

  float NodeStyleGetFlexShrink(NodeConstRef node)
  {
    return node->GetStyle().GetFlexShrink().ValueOrDefault(
      node->GetConfig()->UseWebDefaults() ? Style::WebDefaultFlexShrink : Style::DefaultFlexShrink);
  }

  void NodeStyleSetFlexBasis(NodeRef node, float flexBasis)
  {
    UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, StyleSizeLength::Points(flexBasis));
  }

  void NodeStyleSetFlexBasisPercent(NodeRef node, float flexBasis)
  {
    UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, StyleSizeLength::Percent(flexBasis));
  }

  void NodeStyleSetFlexBasisAuto(NodeRef node)
  {
    UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, StyleSizeLength::Auto());
  }

  void NodeStyleSetFlexBasisMaxContent(NodeRef node)
  {
    UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetFlexBasisFitContent(NodeRef node)
  {
    UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, StyleSizeLength::FitContent());
  }

  void NodeStyleSetFlexBasisStretch(NodeRef node)
  {
    UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetFlexBasis(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetFlexBasis());
  }

  void NodeStyleSetPosition(NodeRef node, Edge edge, float position)
  {
    UpdateProperty<&Style::GetPosition, &Style::SetPosition, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Points(position));
  }

  void NodeStyleSetPositionPercent(NodeRef node, Edge edge, float position)
  {
    UpdateProperty<&Style::GetPosition, &Style::SetPosition, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Percent(position));
  }

  UnitValue NodeStyleGetPosition(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetPosition(edge));
  }

  void NodeStyleSetPositionAuto(NodeRef node, Edge edge)
  {
    UpdateProperty<&Style::GetPosition, &Style::SetPosition, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Auto());
  }

  void NodeStyleSetMargin(NodeRef node, Edge edge, float margin)
  {
    UpdateProperty<&Style::GetMargin, &Style::SetMargin, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Points(margin));
  }

  void NodeStyleSetMarginPercent(NodeRef node, Edge edge, float margin)
  {
    UpdateProperty<&Style::GetMargin, &Style::SetMargin, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Percent(margin));
  }

  void NodeStyleSetMarginAuto(NodeRef node, Edge edge)
  {
    UpdateProperty<&Style::GetMargin, &Style::SetMargin, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Auto());
  }

  UnitValue NodeStyleGetMargin(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMargin(edge));
  }

  void NodeStyleSetPadding(NodeRef node, Edge edge, float padding)
  {
    UpdateProperty<&Style::GetPadding, &Style::SetPadding, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Points(padding));
  }

  void NodeStyleSetPaddingPercent(NodeRef node, Edge edge, float padding)
  {
    UpdateProperty<&Style::GetPadding, &Style::SetPadding, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Percent(padding));
  }

  UnitValue NodeStyleGetPadding(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetPadding(edge));
  }

  void NodeStyleSetBorder(NodeRef node, Edge edge, float border)
  {
    UpdateProperty<&Style::GetBorder, &Style::SetBorder, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, edge, StyleLength::Points(border));
  }

  float NodeStyleGetBorder(NodeConstRef node, Edge edge)
  {
    auto border = node->GetStyle().GetBorder(edge);
    if (border.IsUndefined() || border.IsAuto())
    {
      return std::numeric_limits<float>::quiet_NaN();
    }

    return static_cast<UnitValue>(border).Value;
  }

  void NodeStyleSetGap(NodeRef node, Gutter gutter, float gapLength)
  {
    UpdateProperty<&Style::GetGap, &Style::SetGap, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, gutter, StyleLength::Points(gapLength));
  }

  void NodeStyleSetGapPercent(NodeRef node, Gutter gutter, float gapLength)
  {
    UpdateProperty<&Style::GetGap, &Style::SetGap, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, gutter, StyleLength::Percent(gapLength));
  }

  UnitValue NodeStyleGetGap(NodeConstRef node, Gutter gutter)
  {
    return static_cast<UnitValue>(node->GetStyle().GetGap(gutter));
  }

  void NodeStyleSetAspectRatio(NodeRef node, float aspectRatio)
  {
    UpdateProperty<&Style::GetAspectRatio, &Style::SetAspectRatio, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, NullableFloat(aspectRatio));
  }

  float NodeStyleGetAspectRatio(NodeConstRef node)
  {
    return node->GetStyle().GetAspectRatio().Value();
  }

  void NodeStyleSetBoxSizing(NodeRef node, BoxSizing boxSizing)
  {
    UpdateProperty<&Style::GetBoxSizing, &Style::SetBoxSizing, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, boxSizing);
  }

  BoxSizing NodeStyleGetBoxSizing(NodeConstRef node)
  {
    return node->GetStyle().GetBoxSizing();
  }

  void NodeStyleSetWidth(NodeRef node, float width)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Width, StyleSizeLength::Points(width));
  }

  void NodeStyleSetWidthPercent(NodeRef node, float width)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Width, StyleSizeLength::Percent(width));
  }

  void NodeStyleSetWidthAuto(NodeRef node)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Width, StyleSizeLength::Auto());
  }

  void NodeStyleSetWidthMaxContent(NodeRef node)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Width, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetWidthFitContent(NodeRef node)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Width, StyleSizeLength::FitContent());
  }

  void NodeStyleSetWidthStretch(NodeRef node)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Width, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetDimension(Dimension::Width));
  }

  void NodeStyleSetHeight(NodeRef node, float height)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Height, StyleSizeLength::Points(height));
  }

  void NodeStyleSetHeightPercent(NodeRef node, float height)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Height, StyleSizeLength::Percent(height));
  }

  void NodeStyleSetHeightAuto(NodeRef node)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Height, StyleSizeLength::Auto());
  }

  void NodeStyleSetHeightMaxContent(NodeRef node)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Height, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetHeightFitContent(NodeRef node)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Height, StyleSizeLength::FitContent());
  }

  void NodeStyleSetHeightStretch(NodeRef node)
  {
    UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtiesLayout {true}, DirtiesStyle {false}>(
      node, Dimension::Height, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetDimension(Dimension::Height));
  }

  void NodeStyleSetMinWidth(NodeRef node, float minWidth)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::Points(minWidth));
  }

  void NodeStyleSetMinWidthPercent(NodeRef node, float minWidth)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::Percent(minWidth));
  }

  void NodeStyleSetMinWidthMaxContent(NodeRef node)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMinWidthFitContent(NodeRef node)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::FitContent());
  }

  void NodeStyleSetMinWidthStretch(NodeRef node)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMinWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMinDimension(Dimension::Width));
  }

  void NodeStyleSetMinHeight(NodeRef node, float minHeight)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::Points(minHeight));
  }

  void NodeStyleSetMinHeightPercent(NodeRef node, float minHeight)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::Percent(minHeight));
  }

  void NodeStyleSetMinHeightMaxContent(NodeRef node)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMinHeightFitContent(NodeRef node)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::FitContent());
  }

  void NodeStyleSetMinHeightStretch(NodeRef node)
  {
    UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMinHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMinDimension(Dimension::Height));
  }

  void NodeStyleSetMaxWidth(NodeRef node, float maxWidth)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::Points(maxWidth));
  }

  void NodeStyleSetMaxWidthPercent(NodeRef node, float maxWidth)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::Percent(maxWidth));
  }

  void NodeStyleSetMaxWidthMaxContent(NodeRef node)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMaxWidthFitContent(NodeRef node)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::FitContent());
  }

  void NodeStyleSetMaxWidthStretch(NodeRef node)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Width, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMaxWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMaxDimension(Dimension::Width));
  }

  void NodeStyleSetMaxHeight(NodeRef node, float maxHeight)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::Points(maxHeight));
  }

  void NodeStyleSetMaxHeightPercent(NodeRef node, float maxHeight)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::Percent(maxHeight));
  }

  void NodeStyleSetMaxHeightMaxContent(NodeRef node)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMaxHeightFitContent(NodeRef node)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::FitContent());
  }

  void NodeStyleSetMaxHeightStretch(NodeRef node)
  {
    UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtiesLayout {true},
                   DirtiesStyle {false}>(node, Dimension::Height, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMaxHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMaxDimension(Dimension::Height));
  }

  void NodeStyleSetBackgroundColour(NodeRef node, const Gfx::Colour &colour)
  {
    UpdateProperty<&Style::GetBackgroundColour, &Style::SetBackgroundColour, DirtiesLayout {false},
                   DirtiesStyle {true}>(node, colour);
  }

  Gfx::Colour NodeStyleGetBackgroundColour(NodeConstRef node)
  {
    return node->GetStyle().GetBackgroundColour();
  }

  void NodeStyleSetBorderColour(NodeRef node, const Gfx::Colour &colour)
  {
    UpdateProperty<&Style::GetBorderColour, &Style::SetBorderColour, DirtiesLayout {false},
                   DirtiesStyle {true}>(node, colour);
  }

  Gfx::Colour NodeStyleGetBorderColour(NodeConstRef node)
  {
    return node->GetStyle().GetBorderColour();
  }

  void NodeStyleSetBorderRadius(NodeRef node, float radius)
  {
    UpdateProperty<&Style::GetBorderRadius, &Style::SetBorderRadius, DirtiesLayout {false},
                   DirtiesStyle {true}>(node, radius);
  }

  float NodeStyleGetBorderRadius(NodeConstRef node)
  {
    return node->GetStyle().GetBorderRadius();
  }

  void NodeStyleSetBorderWidth(NodeRef node, float radius)
  {
    UpdateProperty<&Style::GetBorderWidth, &Style::SetBorderWidth, DirtiesLayout {true},
                   DirtiesStyle {true}>(node, radius);
  }

  float NodeStyleGetBorderWidth(NodeConstRef node)
  {
    return node->GetStyle().GetBorderWidth();
  }

  void NodeStyleSetTextColour(NodeRef node, const Gfx::Colour &colour)
  {
    UpdateProperty<&Style::GetTextColour, &Style::SetTextColour, DirtiesLayout {false}, DirtiesStyle {true}>(
      node, colour);
  }

  Gfx::Colour NodeStyleGetTextColour(NodeConstRef node)
  {
    return node->GetStyle().GetTextColour();
  }

  void NodeStyleSetOpacity(NodeRef node, float opacity)
  {
    UpdateProperty<&Style::GetOpacity, &Style::SetOpacity, DirtiesLayout {false}, DirtiesStyle {true}>(
      node, opacity);
  }

  float NodeStyleGetOpacity(NodeConstRef node)
  {
    return node->GetStyle().GetOpacity();
  }

  void NodeStyleSetFontFamily(NodeRef node, Gfx::FontFamilyHandle family)
  {
    UpdateProperty<&Style::GetFontFamily, &Style::SetFontFamily, DirtiesLayout {true}, DirtiesStyle {true}>(
      node, family);
  }

  Gfx::FontFamilyHandle NodeStyleGetFontFamily(NodeConstRef node)
  {
    return node->GetStyle().GetFontFamily();
  }

  void NodeStyleSetFontSize(NodeRef node, float fontSize)
  {
    UpdateProperty<&Style::GetFontSize, &Style::SetFontSize, DirtiesLayout {true}, DirtiesStyle {true}>(
      node, fontSize);
  }

  float NodeStyleGetFontSize(NodeConstRef node)
  {
    return node->GetStyle().GetFontSize();
  }

  void NodeStyleSetTextAlign(NodeRef node, TextAlign textAlign)
  {
    UpdateProperty<&Style::GetTextAlign, &Style::SetTextAlign, DirtiesLayout {true}, DirtiesStyle {true}>(
      node, textAlign);
  }

  TextAlign NodeStyleGetTextAlign(NodeConstRef node)
  {
    return node->GetStyle().GetTextAlign();
  }
}