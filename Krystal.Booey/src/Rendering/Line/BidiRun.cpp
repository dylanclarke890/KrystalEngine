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

#include "Krystal.Booey/Rendering/Line/BidiRun.hpp"

#include "Krystal.Booey/Rendering/Line/LegacyInlineBox.hpp"
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
