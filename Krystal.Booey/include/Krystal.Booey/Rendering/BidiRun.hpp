#pragma once

#include "Krystal.Booey/Core/BidiResolver.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"

namespace krys::boo
{
  class LegacyInlineBox;
  class RenderObject;

  /// @brief A bidi run of a renderer's content on a line, and the inline box made for it.
  struct BidiRun : BidiCharacterRun
  {
  private:
    CheckedRef<RenderObject> _renderer;
    LegacyInlineBox *_box {nullptr};

  public:
    BidiRun(uint32 start, uint32 stop, RenderObject &, BidiContext *, UCharDirection) noexcept;
    ~BidiRun() noexcept override;

    KRYS_NODISCARD BidiRun *Next() const noexcept
    {
      return static_cast<BidiRun *>(BidiCharacterRun::Next());
    }
    KRYS_NODISCARD UniquePtr<BidiRun> TakeNext() noexcept;
    void SetNext(UniquePtr<BidiRun> &&next) noexcept
    {
      BidiCharacterRun::SetNext(UniquePtr<BidiCharacterRun>(next.release()));
    }
    KRYS_NODISCARD RenderObject &Renderer() const noexcept
    {
      return *_renderer;
    }
    KRYS_NODISCARD LegacyInlineBox *Box() const noexcept
    {
      return _box;
    }
    void SetBox(LegacyInlineBox *box) noexcept
    {
      _box = box;
    }
  };
}
