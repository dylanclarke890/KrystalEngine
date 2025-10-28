#pragma once

#include "Krystal.Gfx.Lib/Handle.hpp"
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

  class Element
  {
    NO_COPY(Element)

  public:
    Element(ElementHandle handle, ConfigRef config) noexcept : Handle(handle), LayoutConfig(config)
    {
      LayoutNode = NodeCreate(LayoutConfig);
    }

    ~Element() = default;

    MOVE_SWAP(Element)

    ElementHandle Handle;
    ElementHandle Parent;
    List<ElementHandle> Children;
    NodeRef LayoutNode;
    ConfigRef LayoutConfig;

  private:
    void Swap(Element &other) noexcept
    {
      std::swap(Handle, other.Handle);
      std::swap(Parent, other.Parent);
      std::swap(Children, other.Children);
      std::swap(LayoutNode, other.LayoutNode);
      std::swap(LayoutConfig, other.LayoutConfig);
    }
  };
}