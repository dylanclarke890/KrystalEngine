#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Element.hpp"
#include "Krystal.Gfx/Commands/CommandArena.hpp"
#include "Krystal.Gfx/Commands/CommandList.hpp"

namespace Krys::UI
{
  struct Layer
  {
    ElementHandle RootElement {};
    float Width {0.f};
    float Height {0.f};
    Gfx::CommandArena CommandArena {};
    List<Gfx::CommandList> CommandLists {};
  };
}