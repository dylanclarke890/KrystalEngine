#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class Node;

  class NodeList : public RefCounted<NodeList>
  {
  public:
    virtual ~NodeList() noexcept = default;

    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<Node> Item(size_t index) const noexcept = 0;
  };
}