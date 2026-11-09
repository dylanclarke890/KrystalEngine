/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Layers/LayerAncestorClippingStack.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  CompositedClipData::CompositedClipData(RenderLayer *layer, const LayoutRoundedRect &roundedRect,
                                         bool isOverflowScrollEntry) noexcept
      : ClippingLayer(CreateWeakPtr(layer)), ClipRect(roundedRect), IsOverflowScroll(isOverflowScrollEntry)
  {
  }

  LayerAncestorClippingStack::LayerAncestorClippingStack(List<CompositedClipData> &&clipDataStack) noexcept
  {
    _stack.reserve(clipDataStack.size());
    for (auto &clipDataEntry : clipDataStack)
    {
      _stack.push_back({krys::move(clipDataEntry), nullptr, nullptr});
    }
  }

  bool
    LayerAncestorClippingStack::EqualToClipData(const List<CompositedClipData> &clipDataStack) const noexcept
  {
    if (_stack.size() != clipDataStack.size())
    {
      return false;
    }

    for (size_t i = 0; i < _stack.size(); ++i)
    {
      if (!(_stack[i].ClipData == clipDataStack[i]))
      {
        return false;
      }
    }

    return true;
  }

  bool LayerAncestorClippingStack::HasAnyScrollingLayers() const noexcept
  {
    for (const auto &entry : _stack)
    {
      if (entry.ClipData.IsOverflowScroll)
      {
        return true;
      }
    }

    return false;
  }

  void LayerAncestorClippingStack::Clear() noexcept
  {
    for (auto &entry : _stack)
    {
      GraphicsLayer::UnparentAndClear(entry.ClippingLayer);
      GraphicsLayer::UnparentAndClear(entry.ScrollingLayer);
    }
  }

  GraphicsLayer *LayerAncestorClippingStack::FirstLayer() const noexcept
  {
    return _stack.front().ChildForSuperlayers();
  }

  GraphicsLayer *LayerAncestorClippingStack::LastLayer() const noexcept
  {
    return _stack.back().ParentForSublayers();
  }

  bool LayerAncestorClippingStack::UpdateWithClipData(List<CompositedClipData> &&clipDataStack) noexcept
  {
    bool stackChanged = false;

    size_t clipEntryCount = clipDataStack.size();
    size_t stackEntryCount = _stack.size();

    for (size_t i = 0; i < clipEntryCount; ++i)
    {
      auto &clipDataEntry = clipDataStack[i];

      if (i >= stackEntryCount)
      {
        _stack.push_back({krys::move(clipDataEntry), nullptr, nullptr});
        stackChanged = true;
        continue;
      }

      auto &existingEntry = _stack[i];

      if (!(existingEntry.ClipData == clipDataEntry))
      {
        stackChanged = true;
      }

      existingEntry.ClipData = krys::move(clipDataEntry);
    }

    if (stackEntryCount > clipEntryCount)
    {
      for (size_t i = clipEntryCount; i < stackEntryCount; ++i)
      {
        GraphicsLayer::UnparentAndClear(_stack[i].ClippingLayer);
      }

      _stack.erase(_stack.begin() + static_cast<ptrdiff_t>(clipEntryCount), _stack.end());
      stackChanged = true;
    }
    else
    {
      _stack.shrink_to_fit();
    }

    return stackChanged;
  }

  List<CompositedClipData> LayerAncestorClippingStack::GetCompositedClipData() const noexcept
  {
    List<CompositedClipData> clipData;
    clipData.reserve(_stack.size());
    for (const auto &entry : _stack)
    {
      clipData.push_back(entry.ClipData);
    }
    return clipData;
  }

  namespace
  {
    TextStream &operator<<(TextStream &ts,
                           const LayerAncestorClippingStack::ClippingStackEntry &entry) noexcept
    {
      ts.DumpProperty(u8"clippingLayer", static_cast<const void *>(entry.ClipData.ClippingLayer.get()));
      ts.DumpProperty(u8"clip", entry.ClipData.ClipRect);
      ts.DumpProperty(u8"isOverflowScroll", entry.ClipData.IsOverflowScroll);

      if (entry.ClippingLayer)
      {
        ts.DumpProperty(u8"clippingLayer", entry.ClippingLayer->PrimaryLayerID());
      }

      if (entry.ScrollingLayer)
      {
        ts.DumpProperty(u8"scrollingLayer", entry.ScrollingLayer->PrimaryLayerID());
      }

      return ts;
    }
  }

  TextStream &operator<<(TextStream &ts, const LayerAncestorClippingStack &clipStack) noexcept
  {
    TextStream multilineStream;
    multilineStream.SetIndent(ts.GetIndent() + 2);

    {
      TextStream::GroupScope scope(ts);
      multilineStream << "LayerAncestorClippingStack";

      for (size_t i = 0; i < clipStack.Stack().size(); ++i)
      {
        const auto &entry = clipStack.Stack()[i];

        TextStream::GroupScope entryScope(multilineStream);
        multilineStream << "entry " << static_cast<uint32>(i);
        multilineStream << entry;
      }

      ts << multilineStream.Release();
    }
    return ts;
  }
}
