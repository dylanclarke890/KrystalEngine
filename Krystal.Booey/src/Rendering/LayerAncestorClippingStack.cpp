#include "Krystal.Booey/Rendering/LayerAncestorClippingStack.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
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
