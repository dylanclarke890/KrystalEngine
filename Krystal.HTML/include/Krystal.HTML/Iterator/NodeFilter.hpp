#pragma once

#include "Krystal.HTML/Iterator/FilterResult.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class Node;

  class NodeFilter : public RefCounted<NodeFilter>
  {
  public:
    virtual ~NodeFilter() noexcept = default;

    KRYS_NODISCARD virtual ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept = 0;
  };
}