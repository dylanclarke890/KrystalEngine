#include "Krystal.Booey/Rendering/LegacyInlineIterator.hpp"

#include "Krystal.Booey/Rendering/LegacyLineLayout.hpp"
#include <limits>

namespace krys::boo
{
  RenderObject *FirstInlineRendererSkippingEmpty(RenderElement &root, InlineBidiResolver *resolver) noexcept
  {
    RenderObject *renderer = root.FirstChild();
    if (!renderer)
    {
      return nullptr;
    }

    if (auto *renderInline = DynamicDowncast<RenderInline>(*renderer))
    {
      NotifyObserverEnteredObject(resolver, renderer);
      if (!IsEmptyInline(*renderInline))
      {
        renderer = NextInlineRendererSkippingEmpty(root, renderer, resolver);
      }
      else
      {
        // Never skip empty inlines.
        if (resolver)
        {
          resolver->CommitExplicitEmbedding();
        }
        return renderer;
      }
    }

    // FIXME: Unify this with the next call above.
    if (renderer && !IsIteratorTarget(renderer))
    {
      renderer = NextInlineRendererSkippingEmpty(root, renderer, resolver);
    }

    if (resolver)
    {
      resolver->CommitExplicitEmbedding();
    }
    return renderer;
  }

  void IsolateTracker::AddFakeRunIfNecessary(RenderObject &object, uint32 position, uint32 end,
                                             RenderElement &root, InlineBidiResolver &resolver) noexcept
  {
    // We only need to add a fake run for a given isolated span once during each call to
    // createBidiRunsForLine. We'll be called for every span inside the isolated span so we just ignore
    // subsequent calls. We also avoid creating a fake run until we hit a child that warrants one, e.g. we
    // skip floats.
    if (LegacyLineLayout::ShouldSkipCreatingRunsForObject(object))
    {
      return;
    }
    if (!_haveAddedFakeRunForRootIsolate)
    {
      // object and position together denote a single position in the inline, from which the parsing of the
      // isolate will start. We don't need to mark the end of the run because this is implicit: it is either
      // endOfLine or the end of the isolate, when we call createBidiRunsForLine it will stop at whichever
      // comes first.
      AddPlaceholderRunForIsolatedInline(resolver, object, position, root);
    }
    _haveAddedFakeRunForRootIsolate = true;
    LegacyLineLayout::AppendRunsForObject(nullptr, static_cast<int32>(position), static_cast<int32>(end),
                                          object, resolver);
  }

  template <>
  void InlineBidiResolver::AppendRunInternal() noexcept
  {
    if (!_emptyRun && !_eor.AtEnd() && !_reachedEndOfLine)
    {
      // Keep track of when we enter/leave "unicode-bidi: isolate" inlines.
      // Initialize our state depending on if we're starting in the middle of such an inline.
      // FIXME: Could this initialize from this->inIsolate() instead of walking up the render tree?
      IsolateTracker isolateTracker(NumberOfIsolateAncestors(_sor));
      auto start = static_cast<int32>(_sor.Offset());
      RenderObject *obj = _sor.Renderer();
      while (obj && obj != _eor.Renderer() && obj != _endOfLine.Renderer())
      {
        size_t objectLength = 1;
        if (auto *renderText = DynamicDowncast<RenderText>(obj))
        {
          objectLength = renderText->Length();
        }

        if (isolateTracker.InIsolate())
        {
          isolateTracker.AddFakeRunIfNecessary(*obj, static_cast<uint32>(start),
                                               static_cast<uint32>(objectLength), *_sor.Root(), *this);
        }
        else
        {
          LegacyLineLayout::AppendRunsForObject(&_runs, start, static_cast<int32>(objectLength), *obj, *this);
        }
        // FIXME: start/obj should be an LegacyInlineIterator instead of two separate variables.
        start = 0;
        obj = NextInlineRendererSkippingEmpty(*_sor.Root(), obj, &isolateTracker);
      }

      if (obj)
      {
        // The end is just past the code point the run ends at, or the whole renderer.
        Maybe<uint32> stop;
        if (obj == _eor.Renderer())
        {
          stop = _eor.NextOffset();
        }
        if (obj == _endOfLine.Renderer() && (!stop || _endOfLine.Offset() <= _eor.Offset()))
        {
          _reachedEndOfLine = true;
          stop = _endOfLine.NextOffset();
        }
        size_t objectLength = 1;
        if (auto *renderText = DynamicDowncast<RenderText>(obj))
        {
          objectLength = renderText->Length();
        }

        // It's OK to add runs for zero-length RenderObjects, just don't make the run larger than it should be
        int32 end = objectLength ? static_cast<int32>(stop.value_or(0)) : 0;
        if (isolateTracker.InIsolate())
        {
          isolateTracker.AddFakeRunIfNecessary(*obj, static_cast<uint32>(start),
                                               static_cast<uint32>(objectLength), *_sor.Root(), *this);
        }
        else
        {
          LegacyLineLayout::AppendRunsForObject(&_runs, start, end, *obj, *this);
        }
      }

      _eor.Increment();
      _sor = _eor;
    }

    _direction = U_OTHER_NEUTRAL;
    _status.Eor = U_OTHER_NEUTRAL;
  }
}
