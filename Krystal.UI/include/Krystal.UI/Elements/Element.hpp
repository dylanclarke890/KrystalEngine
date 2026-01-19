#pragma once

#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Handle.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Map.hpp"
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
    utf8_string Text;
  };

  struct Geometry
  {
    using Vertex = Gfx::Vertex::Position2D_ColourbPremultiplied_UV;

    Gfx::MeshHandle Mesh;
    Maths::Vec2 Translation;
  };

  class Element : NonCopyable<Element>
  {
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

    Element(Element &&other) noexcept
        : Handle(std::exchange(other.Handle, ElementHandle {})),
          Parent(std::exchange(other.Parent, ElementHandle {})), Children(std::move(other.Children)),
          LayoutNode(std::exchange(other.LayoutNode, nullptr)),
          LayoutConfig(std::exchange(other.LayoutConfig, nullptr)),
          TextContent(std::exchange(other.TextContent, TextNode {})), Geometries(std::move(other.Geometries))
    {
    }

    Element &operator=(Element &&other) noexcept
    {
      if (this != &other)
      {
        Handle = std::exchange(other.Handle, ElementHandle {});
        Parent = std::exchange(other.Parent, ElementHandle {});
        Children = std::move(other.Children);
        LayoutNode = std::exchange(other.LayoutNode, nullptr);
        LayoutConfig = std::exchange(other.LayoutConfig, nullptr);
        TextContent = std::exchange(other.TextContent, TextNode {});
        Geometries = std::move(other.Geometries);
      }
      return *this;
    }

    void SetText(const utf8_string& text) noexcept
    {
      if (!text.empty() && TextContent.Text.empty())
      {
        NodeInsertChild(LayoutNode, TextContent.LayoutNode, 0);
      }
      else if (text.empty() && !TextContent.Text.empty())
      {
        NodeRemoveChild(LayoutNode, TextContent.LayoutNode);
      }

      TextContent.Text = text;
    }

    KRYS_NODISCARD utf8_stringview GetText() const noexcept
    {
      return TextContent.Text;
    }
  };
}

namespace std
{
  template <>
  struct hash<Krys::UI::ElementHandle>
  {
    KRYS_NODISCARD size_t operator()(const Krys::UI::ElementHandle &handle) const noexcept
    {
      return std::hash<Krys::uint32>()(handle.Id);
    }
  };
}