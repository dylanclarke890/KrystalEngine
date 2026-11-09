/**
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Andrew Wellington (proton@wiretapped.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Text/BidiResolver.hpp"
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
