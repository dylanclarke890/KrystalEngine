#pragma once

#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class HTMLCollection;

  class ParentNodeRareData
  {
  private:
    WeakPtr<HTMLCollection> _children;

  public:
    KRYS_NODISCARD Ref<HTMLCollection> Children(ContainerNode &parent) noexcept;
  };
}