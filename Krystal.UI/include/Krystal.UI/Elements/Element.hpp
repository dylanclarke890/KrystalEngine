#pragma once

#include "Krystal.Gfx.Lib/Handle.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/String/StringRef.hpp"
#include "Krystal.UI/Layout/Algorithm/MeasureText.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <type_traits>

namespace Krys::UI
{
  struct ElementHandle : public Gfx::Handle<ElementHandle>
  {
  };

  struct TextNode
  {
    NodeRef LayoutNode {nullptr};
    StringRef Text;
  };

  struct Geometry
  {
    using Vertex = Gfx::Vertex::Position2D_ColourbPremultiplied_UV;

    Gfx::MeshHandle Mesh;
    Maths::Vec2 Translation;
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
    List<Geometry> Geometries;

    Element(ElementHandle handle, ConfigRef config) noexcept : Handle(handle), LayoutConfig(config)
    {
      LayoutNode = NodeCreate(LayoutConfig);

      TextContent.LayoutNode = NodeCreate(LayoutConfig);
      NodeSetNodeType(TextContent.LayoutNode, NodeType::Text);
      NodeSetMeasureFunc(TextContent.LayoutNode, &TextMeasureFunc);
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
      if (text.IsValid() && !TextContent.Text.IsValid())
      {
        NodeInsertChild(LayoutNode, TextContent.LayoutNode, 0);
      }
      else if (!text.IsValid() && TextContent.Text.IsValid())
      {
        NodeRemoveChild(LayoutNode, TextContent.LayoutNode);
      }

      TextContent.Text = text;
    }

    NO_DISCARD StringRef GetText() const noexcept
    {
      return TextContent.Text;
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
      std::swap(Geometries, other.Geometries);
    }
  };
}

namespace std
{
  template <>
  struct hash<Krys::UI::ElementHandle>
  {
    NO_DISCARD size_t operator()(const Krys::UI::ElementHandle &handle) const noexcept
    {
      return std::hash<Krys::uint32>()(handle.Id);
    }
  };
}