#pragma once

#include "Krystal.Lib/Pointers/WeakPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class HTMLCollection;

  class ParentNodeRareData
  {
  private:
    WeakPtr<HTMLCollection> _children;

  public:
    KRYS_NODISCARD Ref<HTMLCollection> Children(const ContainerNode &parent) noexcept;

    void InvalidateChildren() noexcept;
  };
}