#pragma once

#include "Krystal.UI.Layout/Api/Node.hpp"
#include "Krystal.UI.Styles/Enums/Align.hpp"
#include "Krystal.UI.Styles/Enums/BoxSizing.hpp"
#include "Krystal.UI.Styles/Enums/Direction.hpp"
#include "Krystal.UI.Styles/Enums/Display.hpp"
#include "Krystal.UI.Styles/Enums/Edge.hpp"
#include "Krystal.UI.Styles/Enums/FlexDirection.hpp"
#include "Krystal.UI.Styles/Enums/Gutter.hpp"
#include "Krystal.UI.Styles/Enums/Justify.hpp"
#include "Krystal.UI.Styles/Enums/Overflow.hpp"
#include "Krystal.UI.Styles/Enums/Position.hpp"
#include "Krystal.UI.Styles/Enums/Wrap.hpp"
#include "Krystal.UI.Styles/Values/UnitValue.hpp"

namespace Krys::UI::Layout
{
  void NodeCopyStyle(NodeRef dstNode, NodeConstRef srcNode);

  void NodeStyleSetDirection(NodeRef node, Styles::Direction direction);
  Styles::Direction NodeStyleGetDirection(NodeConstRef node);

  void NodeStyleSetFlexDirection(NodeRef node, Styles::FlexDirection flexDirection);
  Styles::FlexDirection NodeStyleGetFlexDirection(NodeConstRef node);

  void NodeStyleSetJustifyContent(NodeRef node, Styles::Justify justifyContent);
  Styles::Justify NodeStyleGetJustifyContent(NodeConstRef node);

  void NodeStyleSetAlignContent(NodeRef node, Styles::Align alignContent);
  Styles::Align NodeStyleGetAlignContent(NodeConstRef node);

  void NodeStyleSetAlignItems(NodeRef node, Styles::Align alignItems);
  Styles::Align NodeStyleGetAlignItems(NodeConstRef node);

  void NodeStyleSetAlignSelf(NodeRef node, Styles::Align alignSelf);
  Styles::Align NodeStyleGetAlignSelf(NodeConstRef node);

  void NodeStyleSetPositionType(NodeRef node, Styles::PositionType positionType);
  Styles::PositionType NodeStyleGetPositionType(NodeConstRef node);

  void NodeStyleSetFlexWrap(NodeRef node, Styles::Wrap flexWrap);
  Styles::Wrap NodeStyleGetFlexWrap(NodeConstRef node);

  void NodeStyleSetOverflow(NodeRef node, Styles::Overflow overflow);
  Styles::Overflow NodeStyleGetOverflow(NodeConstRef node);

  void NodeStyleSetDisplay(NodeRef node, Styles::Display display);
  Styles::Display NodeStyleGetDisplay(NodeConstRef node);

  void NodeStyleSetFlex(NodeRef node, float flex);
  float NodeStyleGetFlex(NodeConstRef node);

  void NodeStyleSetFlexGrow(NodeRef node, float flexGrow);
  float NodeStyleGetFlexGrow(NodeConstRef node);

  void NodeStyleSetFlexShrink(NodeRef node, float flexShrink);
  float NodeStyleGetFlexShrink(NodeConstRef node);

  void NodeStyleSetFlexBasis(NodeRef node, float flexBasis);
  void NodeStyleSetFlexBasisPercent(NodeRef node, float flexBasis);
  void NodeStyleSetFlexBasisAuto(NodeRef node);
  void NodeStyleSetFlexBasisMaxContent(NodeRef node);
  void NodeStyleSetFlexBasisFitContent(NodeRef node);
  void NodeStyleSetFlexBasisStretch(NodeRef node);
  Styles::UnitValue NodeStyleGetFlexBasis(NodeConstRef node);

  void NodeStyleSetPosition(NodeRef node, Styles::Edge edge, float position);
  void NodeStyleSetPositionPercent(NodeRef node, Styles::Edge edge, float position);
  Styles::UnitValue NodeStyleGetPosition(NodeConstRef node, Styles::Edge edge);
  void NodeStyleSetPositionAuto(NodeRef node, Styles::Edge edge);

  void NodeStyleSetMargin(NodeRef node, Styles::Edge edge, float margin);
  void NodeStyleSetMarginPercent(NodeRef node, Styles::Edge edge, float margin);
  void NodeStyleSetMarginAuto(NodeRef node, Styles::Edge edge);
  Styles::UnitValue NodeStyleGetMargin(NodeConstRef node, Styles::Edge edge);

  void NodeStyleSetPadding(NodeRef node, Styles::Edge edge, float padding);
  void NodeStyleSetPaddingPercent(NodeRef node, Styles::Edge edge, float padding);
  Styles::UnitValue NodeStyleGetPadding(NodeConstRef node, Styles::Edge edge);

  void NodeStyleSetBorder(NodeRef node, Styles::Edge edge, float border);
  float NodeStyleGetBorder(NodeConstRef node, Styles::Edge edge);

  void NodeStyleSetGap(NodeRef node, Styles::Gutter gutter, float gapLength);
  void NodeStyleSetGapPercent(NodeRef node, Styles::Gutter gutter, float gapLength);
  Styles::UnitValue NodeStyleGetGap(NodeConstRef node, Styles::Gutter gutter);

  void NodeStyleSetBoxSizing(NodeRef node, Styles::BoxSizing boxSizing);
  Styles::BoxSizing NodeStyleGetBoxSizing(NodeConstRef node);

  void NodeStyleSetWidth(NodeRef node, float width);
  void NodeStyleSetWidthPercent(NodeRef node, float width);
  void NodeStyleSetWidthAuto(NodeRef node);
  void NodeStyleSetWidthMaxContent(NodeRef node);
  void NodeStyleSetWidthFitContent(NodeRef node);
  void NodeStyleSetWidthStretch(NodeRef node);
  Styles::UnitValue NodeStyleGetWidth(NodeConstRef node);

  void NodeStyleSetHeight(NodeRef node, float height);
  void NodeStyleSetHeightPercent(NodeRef node, float height);
  void NodeStyleSetHeightAuto(NodeRef node);
  void NodeStyleSetHeightMaxContent(NodeRef node);
  void NodeStyleSetHeightFitContent(NodeRef node);
  void NodeStyleSetHeightStretch(NodeRef node);
  Styles::UnitValue NodeStyleGetHeight(NodeConstRef node);

  void NodeStyleSetMinWidth(NodeRef node, float minWidth);
  void NodeStyleSetMinWidthPercent(NodeRef node, float minWidth);
  void NodeStyleSetMinWidthMaxContent(NodeRef node);
  void NodeStyleSetMinWidthFitContent(NodeRef node);
  void NodeStyleSetMinWidthStretch(NodeRef node);
  Styles::UnitValue NodeStyleGetMinWidth(NodeConstRef node);

  void NodeStyleSetMinHeight(NodeRef node, float minHeight);
  void NodeStyleSetMinHeightPercent(NodeRef node, float minHeight);
  void NodeStyleSetMinHeightMaxContent(NodeRef node);
  void NodeStyleSetMinHeightFitContent(NodeRef node);
  void NodeStyleSetMinHeightStretch(NodeRef node);
  Styles::UnitValue NodeStyleGetMinHeight(NodeConstRef node);

  void NodeStyleSetMaxWidth(NodeRef node, float maxWidth);
  void NodeStyleSetMaxWidthPercent(NodeRef node, float maxWidth);
  void NodeStyleSetMaxWidthMaxContent(NodeRef node);
  void NodeStyleSetMaxWidthFitContent(NodeRef node);
  void NodeStyleSetMaxWidthStretch(NodeRef node);
  Styles::UnitValue NodeStyleGetMaxWidth(NodeConstRef node);

  void NodeStyleSetMaxHeight(NodeRef node, float maxHeight);
  void NodeStyleSetMaxHeightPercent(NodeRef node, float maxHeight);
  void NodeStyleSetMaxHeightMaxContent(NodeRef node);
  void NodeStyleSetMaxHeightFitContent(NodeRef node);
  void NodeStyleSetMaxHeightStretch(NodeRef node);
  Styles::UnitValue NodeStyleGetMaxHeight(NodeConstRef node);

  void NodeStyleSetAspectRatio(NodeRef node, float aspectRatio);
  float NodeStyleGetAspectRatio(NodeConstRef node);

}