#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Layout/Api/Forward.hpp"

namespace Krys::UI
{
  class Element
  {
    NO_COPY_MOVE(Element)

  protected:
    NodeRef LayoutNode {nullptr};

  public:
    Element() noexcept = default;

    virtual ~Element() = default;

  };
}