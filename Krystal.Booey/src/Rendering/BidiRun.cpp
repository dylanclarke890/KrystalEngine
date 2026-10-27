#include "Krystal.Booey/Rendering/BidiRun.hpp"

#include "Krystal.Booey/Rendering/LegacyInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"

namespace krys::boo
{
  BidiRun::BidiRun(uint32 start, uint32 stop, RenderObject &renderer, BidiContext *context,
                   UCharDirection dir) noexcept
      : BidiCharacterRun(start, stop, context, dir), _renderer(ShareCheckedRef(renderer))
  {
    krys_debug_assert(!Is<RenderText>(renderer) || stop <= Downcast<RenderText>(renderer).Text().length());
  }

  BidiRun::~BidiRun() noexcept = default;

  UniquePtr<BidiRun> BidiRun::TakeNext() noexcept
  {
    UniquePtr<BidiCharacterRun> next = BidiCharacterRun::TakeNext();
    BidiCharacterRun *raw = next.release();
    return UniquePtr<BidiRun>(static_cast<BidiRun *>(raw));
  }
}
