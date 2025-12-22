#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.UI/Layout/Api/Node.hpp"
#include "Krystal.UI/Styles/Enums/Align.hpp"
#include "Krystal.UI/Styles/Enums/BoxSizing.hpp"
#include "Krystal.UI/Styles/Enums/Direction.hpp"
#include "Krystal.UI/Styles/Enums/Display.hpp"
#include "Krystal.UI/Styles/Enums/Edge.hpp"
#include "Krystal.UI/Styles/Enums/FlexDirection.hpp"
#include "Krystal.UI/Styles/Enums/Gutter.hpp"
#include "Krystal.UI/Styles/Enums/Justify.hpp"
#include "Krystal.UI/Styles/Enums/Overflow.hpp"
#include "Krystal.UI/Styles/Enums/Position.hpp"
#include "Krystal.UI/Styles/Enums/TextAlign.hpp"
#include "Krystal.UI/Styles/Enums/Wrap.hpp"
#include "Krystal.UI/Styles/Values/UnitValue.hpp"

namespace Krys::UI
{
  void NodeCopyStyle(NodeRef dstNode, NodeConstRef srcNode);

  void NodeStyleSetDirection(NodeRef node, Direction direction);
  Direction NodeStyleGetDirection(NodeConstRef node);

  void NodeStyleSetFlexDirection(NodeRef node, FlexDirection flexDirection);
  FlexDirection NodeStyleGetFlexDirection(NodeConstRef node);

  void NodeStyleSetJustifyContent(NodeRef node, Justify justifyContent);
  Justify NodeStyleGetJustifyContent(NodeConstRef node);

  void NodeStyleSetAlignContent(NodeRef node, Align alignContent);
  Align NodeStyleGetAlignContent(NodeConstRef node);

  void NodeStyleSetAlignItems(NodeRef node, Align alignItems);
  Align NodeStyleGetAlignItems(NodeConstRef node);

  void NodeStyleSetAlignSelf(NodeRef node, Align alignSelf);
  Align NodeStyleGetAlignSelf(NodeConstRef node);

  void NodeStyleSetPositionType(NodeRef node, PositionType positionType);
  PositionType NodeStyleGetPositionType(NodeConstRef node);

  void NodeStyleSetFlexWrap(NodeRef node, Wrap flexWrap);
  Wrap NodeStyleGetFlexWrap(NodeConstRef node);

  void NodeStyleSetOverflow(NodeRef node, Overflow overflow);
  Overflow NodeStyleGetOverflow(NodeConstRef node);

  void NodeStyleSetDisplay(NodeRef node, Display display);
  Display NodeStyleGetDisplay(NodeConstRef node);

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
  UnitValue NodeStyleGetFlexBasis(NodeConstRef node);

  void NodeStyleSetPosition(NodeRef node, Edge edge, float position);
  void NodeStyleSetPositionPercent(NodeRef node, Edge edge, float position);
  UnitValue NodeStyleGetPosition(NodeConstRef node, Edge edge);
  void NodeStyleSetPositionAuto(NodeRef node, Edge edge);

  void NodeStyleSetMargin(NodeRef node, Edge edge, float margin);
  void NodeStyleSetMarginPercent(NodeRef node, Edge edge, float margin);
  void NodeStyleSetMarginAuto(NodeRef node, Edge edge);
  UnitValue NodeStyleGetMargin(NodeConstRef node, Edge edge);

  void NodeStyleSetPadding(NodeRef node, Edge edge, float padding);
  void NodeStyleSetPaddingPercent(NodeRef node, Edge edge, float padding);
  UnitValue NodeStyleGetPadding(NodeConstRef node, Edge edge);

  void NodeStyleSetBorder(NodeRef node, Edge edge, float border);
  float NodeStyleGetBorder(NodeConstRef node, Edge edge);

  void NodeStyleSetGap(NodeRef node, Gutter gutter, float gapLength);
  void NodeStyleSetGapPercent(NodeRef node, Gutter gutter, float gapLength);
  UnitValue NodeStyleGetGap(NodeConstRef node, Gutter gutter);

  void NodeStyleSetBoxSizing(NodeRef node, BoxSizing boxSizing);
  BoxSizing NodeStyleGetBoxSizing(NodeConstRef node);

  void NodeStyleSetWidth(NodeRef node, float width);
  void NodeStyleSetWidthPercent(NodeRef node, float width);
  void NodeStyleSetWidthAuto(NodeRef node);
  void NodeStyleSetWidthMaxContent(NodeRef node);
  void NodeStyleSetWidthFitContent(NodeRef node);
  void NodeStyleSetWidthStretch(NodeRef node);
  UnitValue NodeStyleGetWidth(NodeConstRef node);

  void NodeStyleSetHeight(NodeRef node, float height);
  void NodeStyleSetHeightPercent(NodeRef node, float height);
  void NodeStyleSetHeightAuto(NodeRef node);
  void NodeStyleSetHeightMaxContent(NodeRef node);
  void NodeStyleSetHeightFitContent(NodeRef node);
  void NodeStyleSetHeightStretch(NodeRef node);
  UnitValue NodeStyleGetHeight(NodeConstRef node);

  void NodeStyleSetMinWidth(NodeRef node, float minWidth);
  void NodeStyleSetMinWidthPercent(NodeRef node, float minWidth);
  void NodeStyleSetMinWidthMaxContent(NodeRef node);
  void NodeStyleSetMinWidthFitContent(NodeRef node);
  void NodeStyleSetMinWidthStretch(NodeRef node);
  UnitValue NodeStyleGetMinWidth(NodeConstRef node);

  void NodeStyleSetMinHeight(NodeRef node, float minHeight);
  void NodeStyleSetMinHeightPercent(NodeRef node, float minHeight);
  void NodeStyleSetMinHeightMaxContent(NodeRef node);
  void NodeStyleSetMinHeightFitContent(NodeRef node);
  void NodeStyleSetMinHeightStretch(NodeRef node);
  UnitValue NodeStyleGetMinHeight(NodeConstRef node);

  void NodeStyleSetMaxWidth(NodeRef node, float maxWidth);
  void NodeStyleSetMaxWidthPercent(NodeRef node, float maxWidth);
  void NodeStyleSetMaxWidthMaxContent(NodeRef node);
  void NodeStyleSetMaxWidthFitContent(NodeRef node);
  void NodeStyleSetMaxWidthStretch(NodeRef node);
  UnitValue NodeStyleGetMaxWidth(NodeConstRef node);

  void NodeStyleSetMaxHeight(NodeRef node, float maxHeight);
  void NodeStyleSetMaxHeightPercent(NodeRef node, float maxHeight);
  void NodeStyleSetMaxHeightMaxContent(NodeRef node);
  void NodeStyleSetMaxHeightFitContent(NodeRef node);
  void NodeStyleSetMaxHeightStretch(NodeRef node);
  UnitValue NodeStyleGetMaxHeight(NodeConstRef node);

  void NodeStyleSetAspectRatio(NodeRef node, float aspectRatio);
  float NodeStyleGetAspectRatio(NodeConstRef node);

  void NodeStyleSetBackgroundColour(NodeRef node, const Gfx::ColourbPremultiplied &colour);
  Gfx::ColourbPremultiplied NodeStyleGetBackgroundColour(NodeConstRef node);

  void NodeStyleSetBorderColours(NodeRef node, const Array<Gfx::ColourbPremultiplied, 4> &colour);
  const Array<Gfx::ColourbPremultiplied, 4> &NodeStyleGetBorderColours(NodeConstRef node);

  void NodeStyleSetBorderRadii(NodeRef node, const Array<float, 4> &radii);
  const Array<float, 4> &NodeStyleGetBorderRadii(NodeConstRef node);

  void NodeStyleSetBorderWidths(NodeRef node, const Array<float, 4> &widths);
  const Array<float, 4> &NodeStyleGetBorderWidths(NodeConstRef node);

  void NodeStyleSetTextColour(NodeRef node, const Gfx::ColourbPremultiplied &colour);
  Gfx::ColourbPremultiplied NodeStyleGetTextColour(NodeConstRef node);

  void NodeStyleSetOpacity(NodeRef node, float opacity);
  float NodeStyleGetOpacity(NodeConstRef node);

  void NodeStyleSetFontFamily(NodeRef node, Gfx::FontFamilyHandle family);
  Gfx::FontFamilyHandle NodeStyleGetFontFamily(NodeConstRef node);

  void NodeStyleSetFontSize(NodeRef node, float fontSize);
  float NodeStyleGetFontSize(NodeConstRef node);

  void NodeStyleSetTextAlign(NodeRef node, TextAlign textAlign);
  TextAlign NodeStyleGetTextAlign(NodeConstRef node);
}