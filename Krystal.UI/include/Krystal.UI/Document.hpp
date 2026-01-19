#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Stack.hpp"
#include "Krystal.UI/Elements/Element.hpp"
#include "Krystal.UI/Layout/Algorithm/MeasureText.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <cassert>

namespace Krys::UI
{
  class Document : NonCopyMovable<Document>
  {
    using ElementManager = Gfx::ResourceManager<Element, ElementHandle>;

  private:
    Gfx::IContext &_context;
    ConfigRef _layoutConfig;
    ElementManager _elements;
    ElementHandle _body;

  public:
    Document(Gfx::IContext &context) : _context(context), _layoutConfig(ConfigCreate())
    {
      _body = Create<Element>();
      ConfigSetContext(_layoutConfig, &_context);
    }

    ~Document()
    {
      ConfigDestroy(_layoutConfig);
    }

    ElementHandle Body() const noexcept
    {
      return _body;
    }

    /// @brief Create a new element.
    template <DerivedFrom<Element> TElement, typename... Args>
    KRYS_NODISCARD ElementHandle Create(Args &&...args)
    {
      ElementHandle handle = _elements.NextHandle();
      _elements.Set(handle, TElement(handle, _layoutConfig, std::forward<Args>(args)...));
      auto &element = _elements.Get(handle);
      NodeSetContext(element.LayoutNode, this);
      NodeSetContext(element.TextContent.LayoutNode, &element);
      return handle;
    }

    KRYS_NODISCARD Element &Get(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid handle");
      return _elements.Get(element);
    }

    /// @brief Detaches the element from it's parent but does not destroy it. Child elements remain intact.
    void Remove(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");

      auto &element = _elements.Get(handle);
      if (!element.Parent.IsValid())
      {
        return;
      }

      auto &parentElement = _elements.Get(element.Parent);
      auto it = std::find(parentElement.Children.begin(), parentElement.Children.end(), handle);
      if (it != parentElement.Children.end())
      {
        parentElement.Children.erase(it);
        NodeRemoveChild(parentElement.LayoutNode, element.LayoutNode);
      }
      element.Parent = ElementHandle {};
    }

    /// @brief Appends a child element under the given parent element.
    void AppendChild(ElementHandle parent, ElementHandle child)
    {
      assert(parent.IsValid() && "Invalid parent handle");
      assert(child.IsValid() && "Invalid child handle");

      auto &parentElement = _elements.Get(parent);
      parentElement.Children.push_back(child);

      auto &childElement = _elements.Get(child);
      childElement.Parent = parent;

      NodeInsertChild(parentElement.LayoutNode, childElement.LayoutNode, parentElement.Children.size() - 1);
    }

    /// @brief Inserts a child element at the specified index under the given parent element.
    void InsertChild(ElementHandle parent, ElementHandle child, size_t index)
    {
      assert(parent.IsValid() && "Invalid parent handle");
      assert(child.IsValid() && "Invalid child handle");

      auto &parentElement = _elements.Get(parent);
      parentElement.Children.insert(parentElement.Children.begin() + index, child);

      auto &childElement = _elements.Get(child);
      childElement.Parent = parent;

      NodeInsertChild(parentElement.LayoutNode, childElement.LayoutNode, index);
    }

    /// @brief Removes the child element at the specified index from the given parent element.
    void RemoveChildAt(ElementHandle parent, size_t index)
    {
      assert(parent.IsValid() && "Invalid parent handle");

      auto &parentElement = _elements.Get(parent);
      assert(index < parentElement.Children.size() && "Index out of bounds");

      ElementHandle childHandle = parentElement.Children[index];
      parentElement.Children.erase(parentElement.Children.begin() + index);

      auto &childElement = _elements.Get(childHandle);
      childElement.Parent = ElementHandle {};

      NodeRemoveChild(parentElement.LayoutNode, childElement.LayoutNode);
    }

    bool ElementRequiresLayer(ElementHandle handle) const
    {
      assert(handle.IsValid() && "Invalid handle");
      return NodeStyleGetOpacity(_elements.Get(handle).LayoutNode) < 1.f;
    }

    /// @brief Destroys the element and removes it from the document. Child elements are also destroyed
    /// recursively.
    void Destroy(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");

      auto &element = _elements.Get(handle);
      for (const auto &childHandle : element.Children)
      {
        Destroy(childHandle);
      }

      if (element.Parent.IsValid())
      {
        auto &parentElement = _elements.Get(element.Parent);
        auto it = std::find(parentElement.Children.begin(), parentElement.Children.end(), handle);
        if (it != parentElement.Children.end())
        {
          NodeRemoveChild(parentElement.LayoutNode, element.LayoutNode);
          parentElement.Children.erase(it);
        }
      }

      _elements.Remove(handle);
    }

#pragma region Styles

    void ElementCopyStyle(ElementHandle dstElement, ElementHandle srcElement)
    {
      assert(dstElement.IsValid() && "Invalid destination element handle");
      assert(srcElement.IsValid() && "Invalid source element handle");
      auto &dstElem = _elements.Get(dstElement);
      auto &srcElem = _elements.Get(srcElement);
      NodeCopyStyle(dstElem.LayoutNode, srcElem.LayoutNode);
    }

    void ElementStyleSetDirection(ElementHandle element, Direction direction)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetDirection(_elements.Get(element).LayoutNode, direction);
    }
    Direction ElementStyleGetDirection(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetDirection(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetFlexDirection(ElementHandle element, FlexDirection flexDirection)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexDirection(_elements.Get(element).LayoutNode, flexDirection);
    }
    FlexDirection ElementStyleGetFlexDirection(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetFlexDirection(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetJustifyContent(ElementHandle element, Justify justifyContent)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetJustifyContent(_elements.Get(element).LayoutNode, justifyContent);
    }
    Justify ElementStyleGetJustifyContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetJustifyContent(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetAlignContent(ElementHandle element, Align alignContent)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetAlignContent(_elements.Get(element).LayoutNode, alignContent);
    }
    Align ElementStyleGetAlignContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetAlignContent(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetAlignItems(ElementHandle element, Align alignItems)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetAlignItems(_elements.Get(element).LayoutNode, alignItems);
    }
    Align ElementStyleGetAlignItems(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetAlignItems(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetAlignSelf(ElementHandle element, Align alignSelf)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetAlignSelf(_elements.Get(element).LayoutNode, alignSelf);
    }
    Align ElementStyleGetAlignSelf(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetAlignSelf(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetPositionType(ElementHandle element, PositionType positionType)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetPositionType(_elements.Get(element).LayoutNode, positionType);
    }
    PositionType ElementStyleGetPositionType(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetPositionType(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetFlexWrap(ElementHandle element, Wrap flexWrap)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexWrap(_elements.Get(element).LayoutNode, flexWrap);
    }
    Wrap ElementStyleGetFlexWrap(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetFlexWrap(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetOverflow(ElementHandle element, Overflow overflow)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetOverflow(_elements.Get(element).LayoutNode, overflow);
    }
    Overflow ElementStyleGetOverflow(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetOverflow(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetDisplay(ElementHandle element, Display display)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetDisplay(_elements.Get(element).LayoutNode, display);
    }
    Display ElementStyleGetDisplay(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetDisplay(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetFlex(ElementHandle element, float flex)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlex(_elements.Get(element).LayoutNode, flex);
    }
    float ElementStyleGetFlex(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetFlex(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetFlexGrow(ElementHandle element, float flexGrow)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexGrow(_elements.Get(element).LayoutNode, flexGrow);
    }
    float ElementStyleGetFlexGrow(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetFlexGrow(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetFlexShrink(ElementHandle element, float flexShrink)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexShrink(_elements.Get(element).LayoutNode, flexShrink);
    }
    float ElementStyleGetFlexShrink(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetFlexShrink(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetFlexBasis(ElementHandle element, float flexBasis)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexBasis(_elements.Get(element).LayoutNode, flexBasis);
    }
    void ElementStyleSetFlexBasisPercent(ElementHandle element, float flexBasis)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexBasisPercent(_elements.Get(element).LayoutNode, flexBasis);
    }
    void ElementStyleSetFlexBasisAuto(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexBasisAuto(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetFlexBasisMaxContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexBasisMaxContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetFlexBasisFitContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexBasisFitContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetFlexBasisStretch(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFlexBasisStretch(_elements.Get(element).LayoutNode);
    }
    UnitValue ElementStyleGetFlexBasis(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetFlexBasis(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetPosition(ElementHandle element, Edge edge, float position)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetPosition(_elements.Get(element).LayoutNode, edge, position);
    }
    void ElementStyleSetPositionPercent(ElementHandle element, Edge edge, float position)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetPositionPercent(_elements.Get(element).LayoutNode, edge, position);
    }
    UnitValue ElementStyleGetPosition(ElementHandle element, Edge edge)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetPosition(_elements.Get(element).LayoutNode, edge);
    }
    void ElementStyleSetPositionAuto(ElementHandle element, Edge edge)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetPositionAuto(_elements.Get(element).LayoutNode, edge);
    }

    void ElementStyleSetMargin(ElementHandle element, Edge edge, float margin)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMargin(_elements.Get(element).LayoutNode, edge, margin);
    }
    void ElementStyleSetMarginPercent(ElementHandle element, Edge edge, float margin)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMarginPercent(_elements.Get(element).LayoutNode, edge, margin);
    }
    void ElementStyleSetMarginAuto(ElementHandle element, Edge edge)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMarginAuto(_elements.Get(element).LayoutNode, edge);
    }
    UnitValue ElementStyleGetMargin(ElementHandle element, Edge edge)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetMargin(_elements.Get(element).LayoutNode, edge);
    }

    void ElementStyleSetPadding(ElementHandle element, Edge edge, float padding)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetPadding(_elements.Get(element).LayoutNode, edge, padding);
    }
    void ElementStyleSetPaddingPercent(ElementHandle element, Edge edge, float padding)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetPaddingPercent(_elements.Get(element).LayoutNode, edge, padding);
    }
    UnitValue ElementStyleGetPadding(ElementHandle element, Edge edge)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetPadding(_elements.Get(element).LayoutNode, edge);
    }

    void ElementStyleSetBorder(ElementHandle element, Edge edge, float border)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetBorder(_elements.Get(element).LayoutNode, edge, border);
    }
    float ElementStyleGetBorder(ElementHandle element, Edge edge)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetBorder(_elements.Get(element).LayoutNode, edge);
    }

    void ElementStyleSetGap(ElementHandle element, Gutter gutter, float gapLength)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetGap(_elements.Get(element).LayoutNode, gutter, gapLength);
    }
    void ElementStyleSetGapPercent(ElementHandle element, Gutter gutter, float gapLength)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetGapPercent(_elements.Get(element).LayoutNode, gutter, gapLength);
    }
    UnitValue ElementStyleGetGap(ElementHandle element, Gutter gutter)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetGap(_elements.Get(element).LayoutNode, gutter);
    }

    void ElementStyleSetBoxSizing(ElementHandle element, BoxSizing boxSizing)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetBoxSizing(_elements.Get(element).LayoutNode, boxSizing);
    }
    BoxSizing ElementStyleGetBoxSizing(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetBoxSizing(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetWidth(ElementHandle element, float width)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetWidth(_elements.Get(element).LayoutNode, width);
    }
    void ElementStyleSetWidthPercent(ElementHandle element, float width)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetWidthPercent(_elements.Get(element).LayoutNode, width);
    }
    void ElementStyleSetWidthAuto(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetWidthAuto(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetWidthMaxContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetWidthMaxContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetWidthFitContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetWidthFitContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetWidthStretch(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetWidthStretch(_elements.Get(element).LayoutNode);
    }
    UnitValue ElementStyleGetWidth(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetWidth(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetHeight(ElementHandle element, float height)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetHeight(_elements.Get(element).LayoutNode, height);
    }
    void ElementStyleSetHeightPercent(ElementHandle element, float height)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetHeightPercent(_elements.Get(element).LayoutNode, height);
    }
    void ElementStyleSetHeightAuto(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetHeightAuto(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetHeightMaxContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetHeightMaxContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetHeightFitContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetHeightFitContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetHeightStretch(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetHeightStretch(_elements.Get(element).LayoutNode);
    }
    UnitValue ElementStyleGetHeight(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetHeight(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetMinWidth(ElementHandle element, float minWidth)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinWidth(_elements.Get(element).LayoutNode, minWidth);
    }
    void ElementStyleSetMinWidthPercent(ElementHandle element, float minWidth)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinWidthPercent(_elements.Get(element).LayoutNode, minWidth);
    }
    void ElementStyleSetMinWidthMaxContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinWidthMaxContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetMinWidthFitContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinWidthFitContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetMinWidthStretch(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinWidthStretch(_elements.Get(element).LayoutNode);
    }
    UnitValue ElementStyleGetMinWidth(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetMinWidth(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetMinHeight(ElementHandle element, float minHeight)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinHeight(_elements.Get(element).LayoutNode, minHeight);
    }
    void ElementStyleSetMinHeightPercent(ElementHandle element, float minHeight)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinHeightPercent(_elements.Get(element).LayoutNode, minHeight);
    }
    void ElementStyleSetMinHeightMaxContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinHeightMaxContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetMinHeightFitContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinHeightFitContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetMinHeightStretch(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMinHeightStretch(_elements.Get(element).LayoutNode);
    }
    UnitValue ElementStyleGetMinHeight(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetMinHeight(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetMaxWidth(ElementHandle element, float maxWidth)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxWidth(_elements.Get(element).LayoutNode, maxWidth);
    }
    void ElementStyleSetMaxWidthPercent(ElementHandle element, float maxWidth)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxWidthPercent(_elements.Get(element).LayoutNode, maxWidth);
    }
    void ElementStyleSetMaxWidthMaxContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxWidthMaxContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetMaxWidthFitContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxWidthFitContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetMaxWidthStretch(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxWidthStretch(_elements.Get(element).LayoutNode);
    }
    UnitValue ElementStyleGetMaxWidth(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetMaxWidth(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetMaxHeight(ElementHandle element, float maxHeight)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxHeight(_elements.Get(element).LayoutNode, maxHeight);
    }
    void ElementStyleSetMaxHeightPercent(ElementHandle element, float maxHeight)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxHeightPercent(_elements.Get(element).LayoutNode, maxHeight);
    }
    void ElementStyleSetMaxHeightMaxContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxHeightMaxContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetMaxHeightFitContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxHeightFitContent(_elements.Get(element).LayoutNode);
    }
    void ElementStyleSetMaxHeightStretch(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetMaxHeightStretch(_elements.Get(element).LayoutNode);
    }
    UnitValue ElementStyleGetMaxHeight(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetMaxHeight(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetAspectRatio(ElementHandle element, float aspectRatio)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetAspectRatio(_elements.Get(element).LayoutNode, aspectRatio);
    }
    float ElementStyleGetAspectRatio(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetAspectRatio(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetBackgroundColour(ElementHandle element, const Gfx::ColourbPremultiplied &colour)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetBackgroundColour(_elements.Get(element).LayoutNode, colour);
    }
    Gfx::ColourbPremultiplied ElementStyleGetBackgroundColour(ElementHandle element,
                                                              const Gfx::ColourbPremultiplied &colour)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetBackgroundColour(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetBorderColours(ElementHandle element,
                                      const Array<Gfx::ColourbPremultiplied, 4> &colours)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetBorderColours(_elements.Get(element).LayoutNode, colours);
    }
    const Array<Gfx::ColourbPremultiplied, 4> &ElementStyleGetBorderColours(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetBorderColours(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetBorderRadii(ElementHandle element, const Array<float, 4> &radii)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetBorderRadii(_elements.Get(element).LayoutNode, radii);
    }
    const Array<float, 4> &ElementStyleGetBorderRadii(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetBorderRadii(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetBorderWidths(ElementHandle element, const Array<float, 4> &widths)
    {
      assert(element.IsValid() && "Invalid element handle");
      auto node = _elements.Get(element).LayoutNode;
      NodeStyleSetBorder(node, Edge::Top, widths[0]);
      NodeStyleSetBorder(node, Edge::Right, widths[1]);
      NodeStyleSetBorder(node, Edge::Bottom, widths[2]);
      NodeStyleSetBorder(node, Edge::Left, widths[3]);
    }
    const Array<float, 4> &ElementStyleGetBorderWidths(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetBorderWidths(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetTextColour(ElementHandle element, const Gfx::ColourbPremultiplied &color)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetTextColour(_elements.Get(element).LayoutNode, color);
    }
    Gfx::ColourbPremultiplied ElementStyleGetTextColour(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetTextColour(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetOpacity(ElementHandle element, float opacity)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetOpacity(_elements.Get(element).LayoutNode, opacity);
    }

    float ElementStyleGetOpacity(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetOpacity(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetFontFamily(ElementHandle element, Gfx::FontFamilyHandle family)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFontFamily(_elements.Get(element).LayoutNode, family);
    }

    Gfx::FontFamilyHandle ElementStyleGetFontFamily(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return NodeStyleGetFontFamily(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetFontSize(ElementHandle element, float fontSize)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetFontSize(_elements.Get(element).LayoutNode, fontSize);
    }

    float ElementStyleGetFontSize(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleGetFontSize(_elements.Get(element).LayoutNode);
    }

    void ElementStyleSetTextAlign(ElementHandle element, TextAlign textAlign)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleSetTextAlign(_elements.Get(element).LayoutNode, textAlign);
    }

    TextAlign ElementStyleGetTextAlign(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      NodeStyleGetTextAlign(_elements.Get(element).LayoutNode);
    }

    Gfx::FontDesc ElementGetFontDesc(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");

      Gfx::FontDesc desc;
      desc.Family = NodeStyleGetFontFamily(_elements.Get(element).LayoutNode);
      if (!desc.Family.IsValid())
      {
        desc.Family = _context.Fonts().GetDefaultFontFamily();
      }

      desc.Size = NodeStyleGetFontSize(_elements.Get(element).LayoutNode);
      desc.Type = Gfx::FontType::Bitmap;

      return desc;
    }

#pragma endregion

    void ElementSetTextContent(ElementHandle element, const utf8_string &text)
    {
      assert(element.IsValid() && "Invalid element handle");
      _elements.Get(element).SetText(text);
    }

    utf8_stringview ElementGetTextContent(ElementHandle element)
    {
      assert(element.IsValid() && "Invalid element handle");
      return _elements.Get(element).GetText();
    }
  };
}