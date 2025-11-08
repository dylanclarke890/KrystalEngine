#pragma once

#include "Krystal.Gfx.Lib/Handle.hpp"
#include "Krystal.Lib/Allocators/StringRef.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"

namespace Krys::UI
{
  struct ElementHandle : public Gfx::Handle<ElementHandle>
  {
  };

  Size TextMeasureFunc(NodeConstRef node, float width, MeasureMode widthMode, float height,
                       MeasureMode heightMode)
  {
    // auto *text = static_cast<TextNode *>(NodeGetContext(node));
    // auto &font = context.Fonts().Get(text->Font);

    // Maths::Vec2 size = font.MeasureText(text->Text, widthMode == MeasureMode::Undefined ? FLT_MAX : width);
    // return {size.x, size.y};

    return {0, 0};
  }

  struct TextNode
  {
    NodeRef LayoutNode {nullptr};
    StringRef Text;
  };

  class Element
  {
    NO_COPY(Element)

  public:
    ElementHandle Handle;
    ElementHandle Parent;
    List<ElementHandle> Children;
    NodeRef LayoutNode {nullptr};
    ConfigRef LayoutConfig {nullptr};
    TextNode TextContent;

    Element(ElementHandle handle, ConfigRef config) noexcept : Handle(handle), LayoutConfig(config)
    {
      LayoutNode = NodeCreate(LayoutConfig);
    }

    ~Element()
    {
      if (TextContent.LayoutNode != nullptr)
      {
        NodeDestroy(TextContent.LayoutNode);
      }
      if (LayoutNode != nullptr)
      {
        NodeDestroy(LayoutNode);
      }
    }

    MOVE_SWAP(Element)

    void SetText(StringRef text) noexcept
    {
      TextContent.Text = text;
      // TODO: we only need to do this because the layout engine treats empty text nodes as part of the flex
      // layout
      if (TextContent.Text.IsValid() && TextContent.LayoutNode == nullptr)
      {
        TextContent.LayoutNode = NodeCreate(LayoutConfig);
        NodeSetNodeType(TextContent.LayoutNode, NodeType::Text);
        NodeSetMeasureFunc(TextContent.LayoutNode, &TextMeasureFunc);
        NodeSetContext(TextContent.LayoutNode, this);
        NodeInsertChild(LayoutNode, TextContent.LayoutNode, 0);
      }
      else if (!TextContent.Text.IsValid() && TextContent.LayoutNode != nullptr)
      {
        NodeRemoveChild(LayoutNode, TextContent.LayoutNode);
        NodeDestroy(TextContent.LayoutNode);
        TextContent.LayoutNode = nullptr;
      }
    }

  private:
    void Swap(Element &other) noexcept
    {
      std::swap(Handle, other.Handle);
      std::swap(Parent, other.Parent);
      std::swap(Children, other.Children);
      std::swap(LayoutNode, other.LayoutNode);
      std::swap(LayoutConfig, other.LayoutConfig);
      std::swap(TextContent, other.TextContent);
    }
  };
}