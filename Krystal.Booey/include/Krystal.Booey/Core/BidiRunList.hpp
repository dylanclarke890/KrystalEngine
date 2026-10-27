#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo
{
  /// @brief The runs a bidi resolver makes for a line, in a singly linked list.
  template <class Run>
  class BidiRunList
  {
    KRYS_NON_COPYABLE(BidiRunList);

  private:
    // The runs form a singly-linked-list, where the links (Run::_next) imply ownership (and are of type
    // UniquePtr). The raw pointers below point into the singly-linked-list.
    UniquePtr<Run> _firstRun; // The head of the list
    Run *_lastRun {nullptr};
    Run *_logicallyLastRun {nullptr};
    uint32 _runCount {0};

  public:
    BidiRunList() noexcept = default;

    // FIXME: Once BidiResolver no longer owns the BidiRunList,
    // then ~BidiRunList should call deleteRuns() automatically.

    KRYS_NODISCARD Run *FirstRun() const noexcept
    {
      return _firstRun.get();
    }
    KRYS_NODISCARD Run *LastRun() const noexcept
    {
      return _lastRun;
    }
    KRYS_NODISCARD Run *LogicallyLastRun() const noexcept
    {
      return _logicallyLastRun;
    }
    KRYS_NODISCARD uint32 RunCount() const noexcept
    {
      return _runCount;
    }

    void AppendRun(UniquePtr<Run> &&run) noexcept
    {
      if (!_firstRun)
      {
        _firstRun = krys::move(run);
        _lastRun = _firstRun.get();
      }
      else
      {
        _lastRun->SetNext(krys::move(run));
        _lastRun = _lastRun->Next();
      }
      _runCount++;
    }

    void PrependRun(UniquePtr<Run> &&run) noexcept
    {
      krys_debug_assert(!run->Next());

      if (!_lastRun)
      {
        _lastRun = run.get();
      }
      else
      {
        run->SetNext(krys::move(_firstRun));
      }
      _firstRun = krys::move(run);
      _runCount++;
    }

    void MoveRunToEnd(Run *run) noexcept
    {
      krys_debug_assert(_firstRun);
      krys_debug_assert(_lastRun);
      krys_debug_assert(run->Next());

      Run *previous = nullptr;
      Run *current = _firstRun.get();
      while (current != run)
      {
        previous = current;
        current = previous->Next();
      }

      if (!previous)
      {
        krys_debug_assert(_firstRun.get() == run);
        UniquePtr<Run> originalFirstRun = krys::move(_firstRun);
        _firstRun = originalFirstRun->TakeNext();
        _lastRun->SetNext(krys::move(originalFirstRun));
      }
      else
      {
        UniquePtr<Run> target = previous->TakeNext();
        previous->SetNext(current->TakeNext());
        _lastRun->SetNext(krys::move(target));
      }
    }

    void MoveRunToBeginning(Run *run) noexcept
    {
      krys_debug_assert(_firstRun);
      krys_debug_assert(_lastRun);
      krys_debug_assert(run != _firstRun.get());

      Run *previous = _firstRun.get();
      Run *current = previous->Next();
      while (current != run)
      {
        previous = current;
        current = previous->Next();
      }

      UniquePtr<Run> target = previous->TakeNext();
      previous->SetNext(run->TakeNext());
      if (run == _lastRun)
      {
        _lastRun = previous;
      }

      target->SetNext(krys::move(_firstRun));
      _firstRun = krys::move(target);
    }

    void Clear() noexcept
    {
      _firstRun = nullptr;
      _lastRun = nullptr;
      _logicallyLastRun = nullptr;
      _runCount = 0;
    }

    void ReverseRuns(uint32 start, uint32 end) noexcept
    {
      if (start >= end)
      {
        return;
      }

      krys_assert(end < _runCount);

      // Get the item before the start of the runs to reverse and put it in
      // |beforeStart|. |curr| should point to the first run to reverse.
      Run *curr = _firstRun.get();
      Run *beforeStart = nullptr;
      uint32 i = 0;
      for (; i < start; ++i)
      {
        beforeStart = curr;
        curr = curr->Next();
      }
      Run *startRun = curr;

      for (; i < end; ++i)
      {
        curr = curr->Next();
      }

      if (!curr->Next())
      {
        _lastRun = startRun;
      }

      // Standard "sliding window" of 3 pointers
      UniquePtr<Run> previous = curr->TakeNext();
      UniquePtr<Run> current = beforeStart ? beforeStart->TakeNext() : krys::move(_firstRun);
      while (current)
      {
        UniquePtr<Run> next = current->TakeNext();
        current->SetNext(krys::move(previous));
        previous = krys::move(current);
        current = krys::move(next);
      }

      if (beforeStart)
      {
        beforeStart->SetNext(krys::move(previous));
      }
      else
      {
        _firstRun = krys::move(previous);
      }
    }

    void SetLogicallyLastRun(Run *run) noexcept
    {
      _logicallyLastRun = run;
    }

    void ReplaceRunWithRuns(Run *toReplace, BidiRunList<Run> &newRuns) noexcept
    {
      krys_debug_assert(newRuns.RunCount());
      krys_debug_assert(_firstRun);
      krys_debug_assert(toReplace);

      _runCount += newRuns.RunCount() - 1; // We are adding the new runs and removing toReplace.

      // Fix up any pointers which may end up stale.
      if (_lastRun == toReplace)
      {
        _lastRun = newRuns.LastRun();
      }
      if (_logicallyLastRun == toReplace)
      {
        _logicallyLastRun = newRuns.LogicallyLastRun();
      }

      if (_firstRun.get() == toReplace)
      {
        newRuns._lastRun->SetNext(_firstRun->TakeNext());
        _firstRun = krys::move(newRuns._firstRun);
      }
      else
      {
        // Find the run just before "toReplace" in the list of runs.
        Run *previousRun = _firstRun.get();
        while (previousRun->Next() && previousRun->Next() != toReplace)
        {
          previousRun = previousRun->Next();
        }
        krys_debug_assert(previousRun);

        UniquePtr<Run> target = previousRun->TakeNext();
        previousRun->SetNext(krys::move(newRuns._firstRun));
        newRuns._lastRun->SetNext(target->TakeNext());
      }

      newRuns.Clear();
    }
  };
}
