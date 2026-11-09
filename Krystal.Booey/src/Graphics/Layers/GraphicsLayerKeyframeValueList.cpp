/*
 * Copyright (C) 2009-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Layers/GraphicsLayerKeyframeValueList.hpp"

namespace krys::boo
{
  void GraphicsLayerKeyframeValueList::Insert(UniquePtr<const GraphicsLayerAnimationValue> value) noexcept
  {
    for (size_t i = 0; i < _values.size(); ++i)
    {
      const auto *curValue = _values[i].get();
      if (curValue->KeyTime() == value->KeyTime())
      {
        krys_debug_assert(false);
        // insert after
        _values.insert(_values.begin() + static_cast<ptrdiff_t>(i + 1), krys::move(value));
        return;
      }
      if (curValue->KeyTime() > value->KeyTime())
      {
        // insert before
        _values.insert(_values.begin() + static_cast<ptrdiff_t>(i), krys::move(value));
        return;
      }
    }

    _values.push_back(krys::move(value));
  }
}
