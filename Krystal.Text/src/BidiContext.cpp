/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All right reserved.
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

#include "Krystal.Text/BidiContext.hpp"

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Text/Bidi.hpp"

namespace krys::text
{
  BidiContext::BidiContext(uint8 level, BidiClass direction, bool override, BidiEmbeddingSource source,
                           BidiContext *parent) noexcept
      : _level(level), _direction(static_cast<uint32>(direction)), _override(override),
        _source(static_cast<uint32>(source)), _parent(ShareRefPtr(parent))
  {
  }

  Ref<BidiContext> BidiContext::CreateUncached(uint8 level, BidiClass direction, bool override,
                                               BidiEmbeddingSource source, BidiContext *parent) noexcept
  {
    return AdoptRef(*new BidiContext(level, direction, override, source, parent));
  }

  Ref<BidiContext> BidiContext::Create(uint8 level, BidiClass direction, bool override,
                                       BidiEmbeddingSource source, BidiContext *parent) noexcept
  {
    krys_debug_assert(direction == (level % 2 ? BidiClass::RightToLeft : BidiClass::LeftToRight));

    if (parent)
    {
      return CreateUncached(level, direction, override, source, parent);
    }

    krys_debug_assert(level <= 1);
    if (!level)
    {
      if (!override)
      {
        static NeverDestroyed<RefPtr<BidiContext>> ltrContext =
          CreateUncached(0, BidiClass::LeftToRight, false, BidiEmbeddingSource::FromStyleOrDOM, nullptr);
        return *ltrContext;
      }

      static NeverDestroyed<RefPtr<BidiContext>> ltrOverrideContext =
        CreateUncached(0, BidiClass::LeftToRight, true, BidiEmbeddingSource::FromStyleOrDOM, nullptr);
      return *ltrOverrideContext;
    }

    if (!override)
    {
      static NeverDestroyed<RefPtr<BidiContext>> rtlContext =
        CreateUncached(1, BidiClass::RightToLeft, false, BidiEmbeddingSource::FromStyleOrDOM, nullptr);
      return *rtlContext;
    }

    static NeverDestroyed<RefPtr<BidiContext>> rtlOverrideContext =
      CreateUncached(1, BidiClass::RightToLeft, true, BidiEmbeddingSource::FromStyleOrDOM, nullptr);
    return *rtlOverrideContext;
  }

  namespace
  {
    Ref<BidiContext> CopyContextAndRebaselineLevel(BidiContext &context, BidiContext *parent) noexcept
    {
      auto newLevel = parent ? parent->Level() : uint8 {0};
      if (context.Dir() == BidiClass::RightToLeft)
      {
        newLevel = NextGreaterOddLevel(newLevel);
      }
      else if (parent)
      {
        newLevel = NextGreaterEvenLevel(newLevel);
      }
      return BidiContext::Create(newLevel, context.Dir(), context.Override(), context.Source(), parent);
    }
  }

  // The BidiContext stack must be immutable -- they're re-used for re-layout after
  // DOM modification/editing -- so we copy all the non-Unicode contexts, and
  // recalculate their levels.
  Ref<BidiContext> BidiContext::CopyStackRemovingUnicodeEmbeddingContexts() noexcept
  {
    List<BidiContext *> contexts;
    for (auto *ancestor = this; ancestor; ancestor = ancestor->Parent())
    {
      if (ancestor->Source() != BidiEmbeddingSource::FromUnicode)
      {
        contexts.push_back(ancestor);
      }
    }
    krys_debug_assert(!contexts.empty());
    auto topContext = CopyContextAndRebaselineLevel(*contexts.back(), nullptr);
    for (auto i = contexts.size() - 1; i; --i)
    {
      topContext = CopyContextAndRebaselineLevel(*contexts[i - 1], topContext.get());
    }
    return topContext;
  }

  bool operator==(const BidiContext &c1, const BidiContext &c2) noexcept
  {
    if (&c1 == &c2)
    {
      return true;
    }
    if (c1.Level() != c2.Level() || c1.Override() != c2.Override() || c1.Dir() != c2.Dir()
        || c1.Source() != c2.Source())
    {
      return false;
    }
    if (!c1.Parent())
    {
      return !c2.Parent();
    }
    return c2.Parent() && *c1.Parent() == *c2.Parent();
  }
}
