#pragma once

#include "Krystal.UI/Layout/Api/Config.hpp"
#include "Krystal.UI/Layout/Api/Forward.hpp"
#include "Krystal.UI/Layout/Api/Node.hpp"
#include "Krystal.UI/Layout/Api/NodeLayout.hpp"
#include "Krystal.UI/Layout/Api/NodeStyle.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include <cassert>

namespace Krys::UI
{
  class LayoutEngine : NonCopyMovable<LayoutEngine>
  {
  public:
    static void Reflow(class Element &element, Maths::Vec2 containingBlock);
  };
}