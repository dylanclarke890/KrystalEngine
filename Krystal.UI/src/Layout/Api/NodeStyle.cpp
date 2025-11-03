#include "Krystal.UI/Layout/Api/NodeStyle.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include "Krystal.UI/Styles/Style.hpp"

namespace
{
  using namespace Krys::UI;

  template <auto GetterT, auto SetterT, typename ValueT>
  void UpdateStyle(NodeRef node, ValueT value)
  {
    auto &style = node->GetStyle();
    if ((style.*GetterT)() != value)
    {
      (style.*SetterT)(value);
      node->MarkDirtyAndPropagate();
    }
  }

  template <auto GetterT, auto SetterT, typename IdxT, typename ValueT>
  void UpdateStyle(NodeRef node, IdxT idx, ValueT value)
  {
    auto &style = node->GetStyle();
    if ((style.*GetterT)(idx) != value)
    {
      (style.*SetterT)(idx, value);
      node->MarkDirtyAndPropagate();
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
      dstNode->MarkDirtyAndPropagate();
    }
  }

  void NodeStyleSetDirection(NodeRef node, Direction direction)
  {
    UpdateStyle<&Style::GetDirection, &Style::SetDirection>(node, direction);
  }

  Direction NodeStyleGetDirection(NodeConstRef node)
  {
    return node->GetStyle().GetDirection();
  }

  void NodeStyleSetFlexDirection(NodeRef node, FlexDirection flexDirection)
  {
    UpdateStyle<&Style::GetFlexDirection, &Style::SetFlexDirection>(node, flexDirection);
  }

  FlexDirection NodeStyleGetFlexDirection(NodeConstRef node)
  {
    return node->GetStyle().GetFlexDirection();
  }

  void NodeStyleSetJustifyContent(NodeRef node, Justify justifyContent)
  {
    UpdateStyle<&Style::GetJustifyContent, &Style::SetJustifyContent>(node, justifyContent);
  }

  Justify NodeStyleGetJustifyContent(NodeConstRef node)
  {
    return node->GetStyle().GetJustifyContent();
  }

  void NodeStyleSetAlignContent(NodeRef node, Align alignContent)
  {
    UpdateStyle<&Style::GetAlignContent, &Style::SetAlignContent>(node, alignContent);
  }

  Align NodeStyleGetAlignContent(NodeConstRef node)
  {
    return node->GetStyle().GetAlignContent();
  }

  void NodeStyleSetAlignItems(NodeRef node, Align alignItems)
  {
    UpdateStyle<&Style::GetAlignItems, &Style::SetAlignItems>(node, alignItems);
  }

  Align NodeStyleGetAlignItems(NodeConstRef node)
  {
    return node->GetStyle().GetAlignItems();
  }

  void NodeStyleSetAlignSelf(NodeRef node, Align alignSelf)
  {
    UpdateStyle<&Style::GetAlignSelf, &Style::SetAlignSelf>(node, alignSelf);
  }

  Align NodeStyleGetAlignSelf(NodeConstRef node)
  {
    return node->GetStyle().GetAlignSelf();
  }

  void NodeStyleSetPositionType(NodeRef node, PositionType positionType)
  {
    UpdateStyle<&Style::GetPositionType, &Style::SetPositionType>(node, positionType);
  }

  PositionType NodeStyleGetPositionType(NodeConstRef node)
  {
    return node->GetStyle().GetPositionType();
  }

  void NodeStyleSetFlexWrap(NodeRef node, Wrap flexWrap)
  {
    UpdateStyle<&Style::GetFlexWrap, &Style::SetFlexWrap>(node, flexWrap);
  }

  Wrap NodeStyleGetFlexWrap(NodeConstRef node)
  {
    return node->GetStyle().GetFlexWrap();
  }

  void NodeStyleSetOverflow(NodeRef node, Overflow overflow)
  {
    UpdateStyle<&Style::GetOverflow, &Style::SetOverflow>(node, overflow);
  }

  Overflow NodeStyleGetOverflow(NodeConstRef node)
  {
    return node->GetStyle().GetOverflow();
  }

  void NodeStyleSetDisplay(NodeRef node, Display display)
  {
    UpdateStyle<&Style::GetDisplay, &Style::SetDisplay>(node, display);
  }

  Display NodeStyleGetDisplay(NodeConstRef node)
  {
    return node->GetStyle().GetDisplay();
  }

  void NodeStyleSetFlex(NodeRef node, float flex)
  {
    UpdateStyle<&Style::GetFlex, &Style::SetFlex>(node, NullableFloat {flex});
  }

  float NodeStyleGetFlex(NodeConstRef node)
  {
    return node->GetStyle().GetFlex().Value();
  }

  void NodeStyleSetFlexGrow(NodeRef node, float flexGrow)
  {
    UpdateStyle<&Style::GetFlexGrow, &Style::SetFlexGrow>(node, NullableFloat {flexGrow});
  }

  float NodeStyleGetFlexGrow(NodeConstRef node)
  {
    return node->GetStyle().GetFlexGrow().ValueOrDefault(Style::DefaultFlexGrow);
  }

  void NodeStyleSetFlexShrink(NodeRef node, float flexShrink)
  {
    UpdateStyle<&Style::GetFlexShrink, &Style::SetFlexShrink>(node, NullableFloat {flexShrink});
  }

  float NodeStyleGetFlexShrink(NodeConstRef node)
  {
    return node->GetStyle().GetFlexShrink().ValueOrDefault(
      node->GetConfig()->UseWebDefaults() ? Style::WebDefaultFlexShrink : Style::DefaultFlexShrink);
  }

  void NodeStyleSetFlexBasis(NodeRef node, float flexBasis)
  {
    UpdateStyle<&Style::GetFlexBasis, &Style::SetFlexBasis>(node, StyleSizeLength::Points(flexBasis));
  }

  void NodeStyleSetFlexBasisPercent(NodeRef node, float flexBasis)
  {
    UpdateStyle<&Style::GetFlexBasis, &Style::SetFlexBasis>(node, StyleSizeLength::Percent(flexBasis));
  }

  void NodeStyleSetFlexBasisAuto(NodeRef node)
  {
    UpdateStyle<&Style::GetFlexBasis, &Style::SetFlexBasis>(node, StyleSizeLength::Auto());
  }

  void NodeStyleSetFlexBasisMaxContent(NodeRef node)
  {
    UpdateStyle<&Style::GetFlexBasis, &Style::SetFlexBasis>(node, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetFlexBasisFitContent(NodeRef node)
  {
    UpdateStyle<&Style::GetFlexBasis, &Style::SetFlexBasis>(node, StyleSizeLength::FitContent());
  }

  void NodeStyleSetFlexBasisStretch(NodeRef node)
  {
    UpdateStyle<&Style::GetFlexBasis, &Style::SetFlexBasis>(node, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetFlexBasis(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetFlexBasis());
  }

  void NodeStyleSetPosition(NodeRef node, Edge edge, float position)
  {
    UpdateStyle<&Style::GetPosition, &Style::SetPosition>(node, edge, StyleLength::Points(position));
  }

  void NodeStyleSetPositionPercent(NodeRef node, Edge edge, float position)
  {
    UpdateStyle<&Style::GetPosition, &Style::SetPosition>(node, edge, StyleLength::Percent(position));
  }

  UnitValue NodeStyleGetPosition(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetPosition(edge));
  }

  void NodeStyleSetPositionAuto(NodeRef node, Edge edge)
  {
    UpdateStyle<&Style::GetPosition, &Style::SetPosition>(node, edge, StyleLength::Auto());
  }

  void NodeStyleSetMargin(NodeRef node, Edge edge, float margin)
  {
    UpdateStyle<&Style::GetMargin, &Style::SetMargin>(node, edge, StyleLength::Points(margin));
  }

  void NodeStyleSetMarginPercent(NodeRef node, Edge edge, float margin)
  {
    UpdateStyle<&Style::GetMargin, &Style::SetMargin>(node, edge, StyleLength::Percent(margin));
  }

  void NodeStyleSetMarginAuto(NodeRef node, Edge edge)
  {
    UpdateStyle<&Style::GetMargin, &Style::SetMargin>(node, edge, StyleLength::Auto());
  }

  UnitValue NodeStyleGetMargin(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMargin(edge));
  }

  void NodeStyleSetPadding(NodeRef node, Edge edge, float padding)
  {
    UpdateStyle<&Style::GetPadding, &Style::SetPadding>(node, edge, StyleLength::Points(padding));
  }

  void NodeStyleSetPaddingPercent(NodeRef node, Edge edge, float padding)
  {
    UpdateStyle<&Style::GetPadding, &Style::SetPadding>(node, edge, StyleLength::Percent(padding));
  }

  UnitValue NodeStyleGetPadding(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetPadding(edge));
  }

  void NodeStyleSetBorder(NodeRef node, Edge edge, float border)
  {
    UpdateStyle<&Style::GetBorder, &Style::SetBorder>(node, edge, StyleLength::Points(border));
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
    UpdateStyle<&Style::GetGap, &Style::SetGap>(node, gutter, StyleLength::Points(gapLength));
  }

  void NodeStyleSetGapPercent(NodeRef node, Gutter gutter, float gapLength)
  {
    UpdateStyle<&Style::GetGap, &Style::SetGap>(node, gutter, StyleLength::Percent(gapLength));
  }

  UnitValue NodeStyleGetGap(NodeConstRef node, Gutter gutter)
  {
    return static_cast<UnitValue>(node->GetStyle().GetGap(gutter));
  }

  void NodeStyleSetAspectRatio(NodeRef node, float aspectRatio)
  {
    UpdateStyle<&Style::GetAspectRatio, &Style::SetAspectRatio>(node, NullableFloat(aspectRatio));
  }

  float NodeStyleGetAspectRatio(NodeConstRef node)
  {
    return node->GetStyle().GetAspectRatio().Value();
  }

  void NodeStyleSetBoxSizing(NodeRef node, BoxSizing boxSizing)
  {
    UpdateStyle<&Style::GetBoxSizing, &Style::SetBoxSizing>(node, boxSizing);
  }

  BoxSizing NodeStyleGetBoxSizing(NodeConstRef node)
  {
    return node->GetStyle().GetBoxSizing();
  }

  void NodeStyleSetWidth(NodeRef node, float width)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Width,
                                                            StyleSizeLength::Points(width));
  }

  void NodeStyleSetWidthPercent(NodeRef node, float width)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Width,
                                                            StyleSizeLength::Percent(width));
  }

  void NodeStyleSetWidthAuto(NodeRef node)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Width, StyleSizeLength::Auto());
  }

  void NodeStyleSetWidthMaxContent(NodeRef node)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Width,
                                                            StyleSizeLength::MaxContent());
  }

  void NodeStyleSetWidthFitContent(NodeRef node)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Width,
                                                            StyleSizeLength::FitContent());
  }

  void NodeStyleSetWidthStretch(NodeRef node)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Width,
                                                            StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetDimension(Dimension::Width));
  }

  void NodeStyleSetHeight(NodeRef node, float height)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Height,
                                                            StyleSizeLength::Points(height));
  }

  void NodeStyleSetHeightPercent(NodeRef node, float height)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Height,
                                                            StyleSizeLength::Percent(height));
  }

  void NodeStyleSetHeightAuto(NodeRef node)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Height, StyleSizeLength::Auto());
  }

  void NodeStyleSetHeightMaxContent(NodeRef node)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Height,
                                                            StyleSizeLength::MaxContent());
  }

  void NodeStyleSetHeightFitContent(NodeRef node)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Height,
                                                            StyleSizeLength::FitContent());
  }

  void NodeStyleSetHeightStretch(NodeRef node)
  {
    UpdateStyle<&Style::GetDimension, &Style::SetDimension>(node, Dimension::Height,
                                                            StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetDimension(Dimension::Height));
  }

  void NodeStyleSetMinWidth(NodeRef node, float minWidth)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::Points(minWidth));
  }

  void NodeStyleSetMinWidthPercent(NodeRef node, float minWidth)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::Percent(minWidth));
  }

  void NodeStyleSetMinWidthMaxContent(NodeRef node)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMinWidthFitContent(NodeRef node)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::FitContent());
  }

  void NodeStyleSetMinWidthStretch(NodeRef node)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMinWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMinDimension(Dimension::Width));
  }

  void NodeStyleSetMinHeight(NodeRef node, float minHeight)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::Points(minHeight));
  }

  void NodeStyleSetMinHeightPercent(NodeRef node, float minHeight)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::Percent(minHeight));
  }

  void NodeStyleSetMinHeightMaxContent(NodeRef node)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMinHeightFitContent(NodeRef node)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::FitContent());
  }

  void NodeStyleSetMinHeightStretch(NodeRef node)
  {
    UpdateStyle<&Style::GetMinDimension, &Style::SetMinDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMinHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMinDimension(Dimension::Height));
  }

  void NodeStyleSetMaxWidth(NodeRef node, float maxWidth)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::Points(maxWidth));
  }

  void NodeStyleSetMaxWidthPercent(NodeRef node, float maxWidth)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::Percent(maxWidth));
  }

  void NodeStyleSetMaxWidthMaxContent(NodeRef node)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMaxWidthFitContent(NodeRef node)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::FitContent());
  }

  void NodeStyleSetMaxWidthStretch(NodeRef node)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Width,
                                                                  StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMaxWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMaxDimension(Dimension::Width));
  }

  void NodeStyleSetMaxHeight(NodeRef node, float maxHeight)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::Points(maxHeight));
  }

  void NodeStyleSetMaxHeightPercent(NodeRef node, float maxHeight)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::Percent(maxHeight));
  }

  void NodeStyleSetMaxHeightMaxContent(NodeRef node)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMaxHeightFitContent(NodeRef node)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::FitContent());
  }

  void NodeStyleSetMaxHeightStretch(NodeRef node)
  {
    UpdateStyle<&Style::GetMaxDimension, &Style::SetMaxDimension>(node, Dimension::Height,
                                                                  StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMaxHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMaxDimension(Dimension::Height));
  }

  void NodeStyleSetBackgroundColour(NodeRef node, const Gfx::Colour &colour)
  {
    node->GetStyle().SetBackgroundColour(colour);
  }

  Gfx::Colour NodeStyleGetBackgroundColour(NodeConstRef node)
  {
    return node->GetStyle().GetBackgroundColour();
  }

  void NodeStyleSetBorderColour(NodeRef node, const Gfx::Colour &colour)
  {
    node->GetStyle().SetBorderColour(colour);
  }

  Gfx::Colour NodeStyleGetBorderColour(NodeConstRef node)
  {
    return node->GetStyle().GetBorderColour();
  }

  void NodeStyleSetTextColour(NodeRef node, const Gfx::Colour &colour)
  {
    node->GetStyle().SetTextColour(colour);
  }

  Gfx::Colour NodeStyleGetTextColour(NodeConstRef node)
  {
    return node->GetStyle().GetTextColour();
  }

  void NodeStyleSetOpacity(NodeRef node, float opacity)
  {
    node->GetStyle().SetOpacity(opacity);
  }

  float NodeStyleGetOpacity(NodeConstRef node)
  {
    return node->GetStyle().GetOpacity();
  }
}