#include "Krystal.UI/Layout/Api/NodeStyle.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include "Krystal.UI/Styles/Style.hpp"

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
    node->UpdateProperty<&Style::GetDirection, &Style::SetDirection, DirtyLayout(true), DirtyStyle(false)>(
      direction);
  }

  Direction NodeStyleGetDirection(NodeConstRef node)
  {
    return node->GetStyle().GetDirection();
  }

  void NodeStyleSetFlexDirection(NodeRef node, FlexDirection flexDirection)
  {
    node->UpdateProperty<&Style::GetFlexDirection, &Style::SetFlexDirection, DirtyLayout(true),
                         DirtyStyle(false)>(flexDirection);
  }

  FlexDirection NodeStyleGetFlexDirection(NodeConstRef node)
  {
    return node->GetStyle().GetFlexDirection();
  }

  void NodeStyleSetJustifyContent(NodeRef node, Justify justifyContent)
  {
    node->UpdateProperty<&Style::GetJustifyContent, &Style::SetJustifyContent, DirtyLayout(true),
                         DirtyStyle(false)>(justifyContent);
  }

  Justify NodeStyleGetJustifyContent(NodeConstRef node)
  {
    return node->GetStyle().GetJustifyContent();
  }

  void NodeStyleSetAlignContent(NodeRef node, Align alignContent)
  {
    node->UpdateProperty<&Style::GetAlignContent, &Style::SetAlignContent, DirtyLayout(true),
                         DirtyStyle(false)>(alignContent);
  }

  Align NodeStyleGetAlignContent(NodeConstRef node)
  {
    return node->GetStyle().GetAlignContent();
  }

  void NodeStyleSetAlignItems(NodeRef node, Align alignItems)
  {
    node->UpdateProperty<&Style::GetAlignItems, &Style::SetAlignItems, DirtyLayout(true), DirtyStyle(false)>(
      alignItems);
  }

  Align NodeStyleGetAlignItems(NodeConstRef node)
  {
    return node->GetStyle().GetAlignItems();
  }

  void NodeStyleSetAlignSelf(NodeRef node, Align alignSelf)
  {
    node->UpdateProperty<&Style::GetAlignSelf, &Style::SetAlignSelf, DirtyLayout(true), DirtyStyle(false)>(
      alignSelf);
  }

  Align NodeStyleGetAlignSelf(NodeConstRef node)
  {
    return node->GetStyle().GetAlignSelf();
  }

  void NodeStyleSetPositionType(NodeRef node, PositionType positionType)
  {
    node->UpdateProperty<&Style::GetPositionType, &Style::SetPositionType, DirtyLayout(true),
                         DirtyStyle(false)>(positionType);
  }

  PositionType NodeStyleGetPositionType(NodeConstRef node)
  {
    return node->GetStyle().GetPositionType();
  }

  void NodeStyleSetFlexWrap(NodeRef node, Wrap flexWrap)
  {
    node->UpdateProperty<&Style::GetFlexWrap, &Style::SetFlexWrap, DirtyLayout(true), DirtyStyle(false)>(
      flexWrap);
  }

  Wrap NodeStyleGetFlexWrap(NodeConstRef node)
  {
    return node->GetStyle().GetFlexWrap();
  }

  void NodeStyleSetOverflow(NodeRef node, Overflow overflow)
  {
    node->UpdateProperty<&Style::GetOverflow, &Style::SetOverflow, DirtyLayout(true), DirtyStyle(false)>(
      overflow);
  }

  Overflow NodeStyleGetOverflow(NodeConstRef node)
  {
    return node->GetStyle().GetOverflow();
  }

  void NodeStyleSetDisplay(NodeRef node, Display display)
  {
    node->UpdateProperty<&Style::GetDisplay, &Style::SetDisplay, DirtyLayout(true), DirtyStyle(false)>(
      display);
  }

  Display NodeStyleGetDisplay(NodeConstRef node)
  {
    return node->GetStyle().GetDisplay();
  }

  void NodeStyleSetFlex(NodeRef node, float flex)
  {
    node->UpdateProperty<&Style::GetFlex, &Style::SetFlex, DirtyLayout(true), DirtyStyle(false)>(
      NullableFloat {flex});
  }

  float NodeStyleGetFlex(NodeConstRef node)
  {
    return node->GetStyle().GetFlex().Value();
  }

  void NodeStyleSetFlexGrow(NodeRef node, float flexGrow)
  {
    node->UpdateProperty<&Style::GetFlexGrow, &Style::SetFlexGrow, DirtyLayout(true), DirtyStyle(false)>(
      NullableFloat {flexGrow});
  }

  float NodeStyleGetFlexGrow(NodeConstRef node)
  {
    return node->GetStyle().GetFlexGrow().ValueOrDefault(Style::DefaultFlexGrow);
  }

  void NodeStyleSetFlexShrink(NodeRef node, float flexShrink)
  {
    node->UpdateProperty<&Style::GetFlexShrink, &Style::SetFlexShrink, DirtyLayout(true), DirtyStyle(false)>(
      NullableFloat {flexShrink});
  }

  float NodeStyleGetFlexShrink(NodeConstRef node)
  {
    return node->GetStyle().GetFlexShrink().ValueOrDefault(
      node->GetConfig()->UseWebDefaults() ? Style::WebDefaultFlexShrink : Style::DefaultFlexShrink);
  }

  void NodeStyleSetFlexBasis(NodeRef node, float flexBasis)
  {
    node->UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtyLayout(true), DirtyStyle(false)>(
      StyleSizeLength::Points(flexBasis));
  }

  void NodeStyleSetFlexBasisPercent(NodeRef node, float flexBasis)
  {
    node->UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtyLayout(true), DirtyStyle(false)>(
      StyleSizeLength::Percent(flexBasis));
  }

  void NodeStyleSetFlexBasisAuto(NodeRef node)
  {
    node->UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtyLayout(true), DirtyStyle(false)>(
      StyleSizeLength::Auto());
  }

  void NodeStyleSetFlexBasisMaxContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtyLayout(true), DirtyStyle(false)>(
      StyleSizeLength::MaxContent());
  }

  void NodeStyleSetFlexBasisFitContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtyLayout(true), DirtyStyle(false)>(
      StyleSizeLength::FitContent());
  }

  void NodeStyleSetFlexBasisStretch(NodeRef node)
  {
    node->UpdateProperty<&Style::GetFlexBasis, &Style::SetFlexBasis, DirtyLayout(true), DirtyStyle(false)>(
      StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetFlexBasis(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetFlexBasis());
  }

  void NodeStyleSetPosition(NodeRef node, Edge edge, float position)
  {
    node->UpdateProperty<&Style::GetPosition, &Style::SetPosition, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Points(position));
  }

  void NodeStyleSetPositionPercent(NodeRef node, Edge edge, float position)
  {
    node->UpdateProperty<&Style::GetPosition, &Style::SetPosition, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Percent(position));
  }

  UnitValue NodeStyleGetPosition(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetPosition(edge));
  }

  void NodeStyleSetPositionAuto(NodeRef node, Edge edge)
  {
    node->UpdateProperty<&Style::GetPosition, &Style::SetPosition, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Auto());
  }

  void NodeStyleSetMargin(NodeRef node, Edge edge, float margin)
  {
    node->UpdateProperty<&Style::GetMargin, &Style::SetMargin, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Points(margin));
  }

  void NodeStyleSetMarginPercent(NodeRef node, Edge edge, float margin)
  {
    node->UpdateProperty<&Style::GetMargin, &Style::SetMargin, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Percent(margin));
  }

  void NodeStyleSetMarginAuto(NodeRef node, Edge edge)
  {
    node->UpdateProperty<&Style::GetMargin, &Style::SetMargin, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Auto());
  }

  UnitValue NodeStyleGetMargin(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMargin(edge));
  }

  void NodeStyleSetPadding(NodeRef node, Edge edge, float padding)
  {
    node->UpdateProperty<&Style::GetPadding, &Style::SetPadding, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Points(padding));
  }

  void NodeStyleSetPaddingPercent(NodeRef node, Edge edge, float padding)
  {
    node->UpdateProperty<&Style::GetPadding, &Style::SetPadding, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Percent(padding));
  }

  UnitValue NodeStyleGetPadding(NodeConstRef node, Edge edge)
  {
    return static_cast<UnitValue>(node->GetStyle().GetPadding(edge));
  }

  void NodeStyleSetBorder(NodeRef node, Edge edge, float border)
  {
    node->UpdateProperty<&Style::GetBorder, &Style::SetBorder, DirtyLayout(true), DirtyStyle(false)>(
      edge, StyleLength::Points(border));
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
    node->UpdateProperty<&Style::GetGap, &Style::SetGap, DirtyLayout(true), DirtyStyle(false)>(
      gutter, StyleLength::Points(gapLength));
  }

  void NodeStyleSetGapPercent(NodeRef node, Gutter gutter, float gapLength)
  {
    node->UpdateProperty<&Style::GetGap, &Style::SetGap, DirtyLayout(true), DirtyStyle(false)>(
      gutter, StyleLength::Percent(gapLength));
  }

  UnitValue NodeStyleGetGap(NodeConstRef node, Gutter gutter)
  {
    return static_cast<UnitValue>(node->GetStyle().GetGap(gutter));
  }

  void NodeStyleSetAspectRatio(NodeRef node, float aspectRatio)
  {
    node
      ->UpdateProperty<&Style::GetAspectRatio, &Style::SetAspectRatio, DirtyLayout(true), DirtyStyle(false)>(
        NullableFloat(aspectRatio));
  }

  float NodeStyleGetAspectRatio(NodeConstRef node)
  {
    return node->GetStyle().GetAspectRatio().Value();
  }

  void NodeStyleSetBoxSizing(NodeRef node, BoxSizing boxSizing)
  {
    node->UpdateProperty<&Style::GetBoxSizing, &Style::SetBoxSizing, DirtyLayout(true), DirtyStyle(false)>(
      boxSizing);
  }

  BoxSizing NodeStyleGetBoxSizing(NodeConstRef node)
  {
    return node->GetStyle().GetBoxSizing();
  }

  void NodeStyleSetWidth(NodeRef node, float width)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Width, StyleSizeLength::Points(width));
  }

  void NodeStyleSetWidthPercent(NodeRef node, float width)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Width, StyleSizeLength::Percent(width));
  }

  void NodeStyleSetWidthAuto(NodeRef node)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Width, StyleSizeLength::Auto());
  }

  void NodeStyleSetWidthMaxContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Width, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetWidthFitContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Width, StyleSizeLength::FitContent());
  }

  void NodeStyleSetWidthStretch(NodeRef node)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Width, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetDimension(Dimension::Width));
  }

  void NodeStyleSetHeight(NodeRef node, float height)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Height, StyleSizeLength::Points(height));
  }

  void NodeStyleSetHeightPercent(NodeRef node, float height)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Height, StyleSizeLength::Percent(height));
  }

  void NodeStyleSetHeightAuto(NodeRef node)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Height, StyleSizeLength::Auto());
  }

  void NodeStyleSetHeightMaxContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Height, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetHeightFitContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Height, StyleSizeLength::FitContent());
  }

  void NodeStyleSetHeightStretch(NodeRef node)
  {
    node->UpdateProperty<&Style::GetDimension, &Style::SetDimension, DirtyLayout(true), DirtyStyle(false)>(
      Dimension::Height, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetDimension(Dimension::Height));
  }

  void NodeStyleSetMinWidth(NodeRef node, float minWidth)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::Points(minWidth));
  }

  void NodeStyleSetMinWidthPercent(NodeRef node, float minWidth)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::Percent(minWidth));
  }

  void NodeStyleSetMinWidthMaxContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMinWidthFitContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::FitContent());
  }

  void NodeStyleSetMinWidthStretch(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMinWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMinDimension(Dimension::Width));
  }

  void NodeStyleSetMinHeight(NodeRef node, float minHeight)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::Points(minHeight));
  }

  void NodeStyleSetMinHeightPercent(NodeRef node, float minHeight)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::Percent(minHeight));
  }

  void NodeStyleSetMinHeightMaxContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMinHeightFitContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::FitContent());
  }

  void NodeStyleSetMinHeightStretch(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMinDimension, &Style::SetMinDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMinHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMinDimension(Dimension::Height));
  }

  void NodeStyleSetMaxWidth(NodeRef node, float maxWidth)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::Points(maxWidth));
  }

  void NodeStyleSetMaxWidthPercent(NodeRef node, float maxWidth)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::Percent(maxWidth));
  }

  void NodeStyleSetMaxWidthMaxContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMaxWidthFitContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::FitContent());
  }

  void NodeStyleSetMaxWidthStretch(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Width, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMaxWidth(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMaxDimension(Dimension::Width));
  }

  void NodeStyleSetMaxHeight(NodeRef node, float maxHeight)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::Points(maxHeight));
  }

  void NodeStyleSetMaxHeightPercent(NodeRef node, float maxHeight)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::Percent(maxHeight));
  }

  void NodeStyleSetMaxHeightMaxContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::MaxContent());
  }

  void NodeStyleSetMaxHeightFitContent(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::FitContent());
  }

  void NodeStyleSetMaxHeightStretch(NodeRef node)
  {
    node->UpdateProperty<&Style::GetMaxDimension, &Style::SetMaxDimension, DirtyLayout(true),
                         DirtyStyle(false)>(Dimension::Height, StyleSizeLength::Stretch());
  }

  UnitValue NodeStyleGetMaxHeight(NodeConstRef node)
  {
    return static_cast<UnitValue>(node->GetStyle().GetMaxDimension(Dimension::Height));
  }

  void NodeStyleSetBackgroundColour(NodeRef node, const Gfx::ColourbPremultiplied &colour)
  {
    node->UpdateProperty<&Style::GetBackgroundColour, &Style::SetBackgroundColour, DirtyLayout(false),
                         DirtyStyle(true)>(colour);
  }

  Gfx::ColourbPremultiplied NodeStyleGetBackgroundColour(NodeConstRef node)
  {
    return node->GetStyle().GetBackgroundColour();
  }

  void NodeStyleSetBorderColours(NodeRef node, const Array<Gfx::ColourbPremultiplied, 4> &colours)
  {
    node->UpdateProperty<&Style::GetBorderColours, &Style::SetBorderColours, DirtyLayout(false),
                         DirtyStyle(true)>(colours);
  }

  const Array<Gfx::ColourbPremultiplied, 4> &NodeStyleGetBorderColours(NodeConstRef node)
  {
    return node->GetStyle().GetBorderColours();
  }

  void NodeStyleSetBorderRadii(NodeRef node, const Array<float, 4> &radii)
  {
    node
      ->UpdateProperty<&Style::GetBorderRadii, &Style::SetBorderRadii, DirtyLayout(false), DirtyStyle(true)>(
        radii);
  }

  const Array<float, 4> &NodeStyleGetBorderRadii(NodeConstRef node)
  {
    return node->GetStyle().GetBorderRadii();
  }

  void NodeStyleSetBorderWidths(NodeRef node, const Array<float, 4> &widths)
  {
    node
      ->UpdateProperty<&Style::GetBorderWidths, &Style::SetBorderWidths, DirtyLayout(true), DirtyStyle(true)>(
        widths);
  }

  const Array<float, 4> &NodeStyleGetBorderWidths(NodeConstRef node)
  {
    return node->GetStyle().GetBorderWidths();
  }

  void NodeStyleSetTextColour(NodeRef node, const Gfx::ColourbPremultiplied &colour)
  {
    node->UpdateProperty<&Style::GetTextColour, &Style::SetTextColour, DirtyLayout(false), DirtyStyle(true)>(
      colour);
  }

  Gfx::ColourbPremultiplied NodeStyleGetTextColour(NodeConstRef node)
  {
    return node->GetStyle().GetTextColour();
  }

  void NodeStyleSetOpacity(NodeRef node, float opacity)
  {
    node->UpdateProperty<&Style::GetOpacity, &Style::SetOpacity, DirtyLayout(false), DirtyStyle(true)>(
      opacity);
  }

  float NodeStyleGetOpacity(NodeConstRef node)
  {
    return node->GetStyle().GetOpacity();
  }

  void NodeStyleSetFontFamily(NodeRef node, Gfx::FontFamilyHandle family)
  {
    node->UpdateProperty<&Style::GetFontFamily, &Style::SetFontFamily, DirtyLayout(true), DirtyStyle(true)>(
      family);
  }

  Gfx::FontFamilyHandle NodeStyleGetFontFamily(NodeConstRef node)
  {
    return node->GetStyle().GetFontFamily();
  }

  void NodeStyleSetFontSize(NodeRef node, float fontSize)
  {
    node->UpdateProperty<&Style::GetFontSize, &Style::SetFontSize, DirtyLayout(true), DirtyStyle(true)>(
      fontSize);
  }

  float NodeStyleGetFontSize(NodeConstRef node)
  {
    return node->GetStyle().GetFontSize();
  }

  void NodeStyleSetTextAlign(NodeRef node, TextAlign textAlign)
  {
    node->UpdateProperty<&Style::GetTextAlign, &Style::SetTextAlign, DirtyLayout(true), DirtyStyle(true)>(
      textAlign);
  }

  TextAlign NodeStyleGetTextAlign(NodeConstRef node)
  {
    return node->GetStyle().GetTextAlign();
  }
}