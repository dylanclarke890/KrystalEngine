#pragma once

#include "Krystal.Gfx.Lib/Handle.hpp"
#include "Krystal.Gfx/Commands/CommandArena.hpp"
#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Element.hpp"

namespace Krys::UI
{
  struct LayerHandle : public Gfx::Handle<LayerHandle>
  {
  };

  struct Layer
  {
    ElementHandle RootElement {};
    Gfx::CommandArena CommandArena {};
    List<Gfx::CommandList> CommandLists {};
  };
}