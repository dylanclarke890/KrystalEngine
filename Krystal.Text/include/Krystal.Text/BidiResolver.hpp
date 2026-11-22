/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Text/Bidi.hpp"
#include "Krystal.Text/BidiContext.hpp"
#include "Krystal.Text/BidiRunList.hpp"
#include "Krystal.Text/TextDirection.hpp"
#include <algorithm>

namespace krys::text
{
  template <typename Iterator>
  class WhitespaceCollapsingState
  {
  private:
    List<Iterator> _transitions;
    size_t _currentTransition {0};

  public:
    void Reset() noexcept
    {
      _transitions.clear();
      _currentTransition = 0;
    }

    void StartIgnoringSpaces(const Iterator &transition) noexcept
    {
      krys_debug_assert(!(_transitions.size() % 2));
      _transitions.push_back(transition);
    }

    void StopIgnoringSpaces(const Iterator &transition) noexcept
    {
      krys_debug_assert(_transitions.size() % 2);
      _transitions.push_back(transition);
    }

    /// @brief When ignoring spaces, this needs to be called for objects that need line boxes such as
    /// inlines or hard line breaks to ensure that they're not ignored.
    template <typename TObject>
    void EnsureLineBoxInsideIgnoredSpaces(TObject &object) noexcept
    {
      Iterator transition(nullptr, &object, 0);
      StopIgnoringSpaces(transition);
      StartIgnoringSpaces(transition);
    }

    void DecrementTransitionAt(size_t index) noexcept
    {
      _transitions[index].FastDecrement();
    }

    KRYS_NODISCARD const List<Iterator> &Transitions() const noexcept
    {
      return _transitions;
    }

    KRYS_NODISCARD size_t NumTransitions() const noexcept
    {
      return _transitions.size();
    }

    KRYS_NODISCARD size_t CurrentTransition() const noexcept
    {
      return _currentTransition;
    }

    void SetCurrentTransition(size_t currentTransition) noexcept
    {
      _currentTransition = currentTransition;
    }

    void IncrementCurrentTransition() noexcept
    {
      ++_currentTransition;
    }

    void DecrementNumTransitions() noexcept
    {
      _transitions.pop_back();
    }

    KRYS_NODISCARD bool BetweenTransitions() const noexcept
    {
      return _currentTransition % 2;
    }
  };

  /// @brief The BidiStatus at a given position (typically the end of a line) can be cached and then used to
  /// restart bidi resolution at that position.
  struct BidiStatus
  {
    BidiClass Eor {BidiClass::OtherNeutral};
    BidiClass LastStrong {BidiClass::OtherNeutral};
    BidiClass Last {BidiClass::OtherNeutral};
    RefPtr<BidiContext> Context;

    BidiStatus() noexcept = default;

    /// @brief Creates a BidiStatus representing a new paragraph root with a default direction.
    /// Uses TextDirection as it only has two possibilities instead of BidiClass which has at least 19.
    BidiStatus(TextDirection direction, bool isOverride) noexcept
        : Eor(direction == TextDirection::LTR ? BidiClass::LeftToRight : BidiClass::RightToLeft),
          LastStrong(Eor), Last(Eor),
          Context(BidiContext::Create(direction == TextDirection::LTR ? 0 : 1, Eor, isOverride))
    {
    }

    BidiStatus(BidiClass eor, BidiClass lastStrong, BidiClass last, RefPtr<BidiContext> &&context) noexcept
        : Eor(eor), LastStrong(lastStrong), Last(last), Context(krys::move(context))
    {
    }
  };

  struct BidiEmbedding
  {
    BidiClass Direction;
    BidiEmbeddingSource Source;
  };

  KRYS_NODISCARD inline bool operator==(const BidiStatus &status1, const BidiStatus &status2) noexcept
  {
    return status1.Eor == status2.Eor && status1.Last == status2.Last
           && status1.LastStrong == status2.LastStrong && *(status1.Context) == *(status2.Context);
  }

  struct BidiCharacterRun
  {
  private:
    UniquePtr<BidiCharacterRun> _next;

    uint32 _start;
    uint32 _stop;
    uint8 _level;
    bool _override : 1;

  public:
    BidiCharacterRun(uint32 start, uint32 stop, BidiContext *context, BidiClass direction) noexcept
        : _start(start), _stop(stop), _override(context->Override())
    {
      if (direction == BidiClass::OtherNeutral)
      {
        direction = context->Dir();
      }

      _level = context->Level();

      // add level of run (cases I1 & I2)
      if (_level % 2)
      {
        if (direction == BidiClass::LeftToRight || direction == BidiClass::ArabicNumber
            || direction == BidiClass::EuropeanNumber)
        {
          _level++;
        }
      }
      else
      {
        if (direction == BidiClass::RightToLeft)
        {
          _level++;
        }
        else if (direction == BidiClass::ArabicNumber || direction == BidiClass::EuropeanNumber)
        {
          _level += 2;
        }
      }
    }

    BidiCharacterRun(const BidiCharacterRun &) = delete;

    BidiCharacterRun &operator=(const BidiCharacterRun &) = delete;

    virtual ~BidiCharacterRun() noexcept
    {
      // Delete the linked list in a loop to prevent destructor recursion.
      auto next = krys::move(_next);
      while (next)
      {
        next = krys::move(next->_next);
      }
    }

    KRYS_NODISCARD uint32 Start() const noexcept
    {
      return _start;
    }

    KRYS_NODISCARD uint32 Stop() const noexcept
    {
      return _stop;
    }

    KRYS_NODISCARD uint8 Level() const noexcept
    {
      return _level;
    }

    KRYS_NODISCARD bool Reversed(bool visuallyOrdered) const noexcept
    {
      return _level % 2 && !visuallyOrdered;
    }

    KRYS_NODISCARD bool DirOverride(bool visuallyOrdered) const noexcept
    {
      return _override || visuallyOrdered;
    }

    KRYS_NODISCARD BidiCharacterRun *Next() const noexcept
    {
      return _next.get();
    }

    KRYS_NODISCARD UniquePtr<BidiCharacterRun> TakeNext() noexcept
    {
      return krys::move(_next);
    }

    void SetNext(UniquePtr<BidiCharacterRun> &&next) noexcept
    {
      _next = krys::move(next);
    }
  };

  /// @brief An implementation of the Unicode Bidi Algorithm (http://unicode.org/reports/tr9).
  /// @note The iterator steps by code point. Its NextOffset() is the offset just past the code point it's at,
  /// which ends a run.
  template <typename Iterator, typename Run, typename DerivedClass>
  class BidiResolverBase
  {
    KRYS_NON_COPYABLE(BidiResolverBase);

  protected:
    Iterator _current;
    // sor and eor are "start of run" and "end of run" respectively and correpond
    // to abreviations used in UBA spec: http://unicode.org/reports/tr9/#BD7
    Iterator _sor; // Points to the first character in the current run.
    Iterator _eor; // Points to the last character in the current run.
    Iterator _last;
    BidiStatus _status;
    BidiClass _direction {BidiClass::OtherNeutral};
    Iterator _endOfLine;
    bool _reachedEndOfLine {false};
    Iterator _lastBeforeET; // Before a BidiClass::EuropeanNumberTerminator
    bool _emptyRun {true};

    // FIXME: This should not belong to the resolver, but rather be passed
    // into createBidiRunsForLine by the caller.
    BidiRunList<Run> _runs;

    WhitespaceCollapsingState<Iterator> _whitespaceCollapsingState;

    uint32 _nestedIsolateCount {0};
    HashMap<Run *, uint32> _whitespaceCollapsingTransitionForIsolatedRun;

  private:
    SmallList<BidiEmbedding, 8> _currentExplicitEmbeddingSequence;

  public:
    KRYS_NODISCARD const Iterator &Position() const noexcept
    {
      return _current;
    }

    void SetPositionIgnoringNestedIsolates(const Iterator &position) noexcept
    {
      _current = position;
    }

    void SetPosition(const Iterator &position, uint32 nestedIsolatedCount) noexcept
    {
      _current = position;
      _nestedIsolateCount = nestedIsolatedCount;
    }

    void Increment() noexcept
    {
      static_cast<DerivedClass &>(*this).IncrementInternal();
    }

    KRYS_NODISCARD BidiContext *Context() const noexcept
    {
      return _status.Context.get();
    }

    void SetContext(RefPtr<BidiContext> &&context) noexcept
    {
      _status.Context = krys::move(context);
    }

    void SetLastDir(BidiClass lastDir) noexcept
    {
      _status.Last = lastDir;
    }

    void SetLastStrongDir(BidiClass lastStrongDir) noexcept
    {
      _status.LastStrong = lastStrongDir;
    }

    void SetEorDir(BidiClass eorDir) noexcept
    {
      _status.Eor = eorDir;
    }

    KRYS_NODISCARD BidiClass Dir() const noexcept
    {
      return _direction;
    }

    void SetDir(BidiClass direction) noexcept
    {
      _direction = direction;
    }

    KRYS_NODISCARD const BidiStatus &Status() const noexcept
    {
      return _status;
    }

    void SetStatus(BidiStatus status) noexcept
    {
      _status = krys::move(status);
    }

    KRYS_NODISCARD WhitespaceCollapsingState<Iterator> &GetWhitespaceCollapsingState() noexcept
    {
      return _whitespaceCollapsingState;
    }

    // The current algorithm handles nested isolates one layer of nesting at a time.
    // But when we layout each isolated span, we will walk into (and ignore) all
    // child isolated spans.
    void EnterIsolate() noexcept
    {
      _nestedIsolateCount++;
    }

    void ExitIsolate() noexcept
    {
      krys_debug_assert(_nestedIsolateCount >= 1);
      _nestedIsolateCount--;
    }

    KRYS_NODISCARD bool InIsolate() const noexcept
    {
      return _nestedIsolateCount;
    }

    void Embed(BidiClass dir, BidiEmbeddingSource source) noexcept
    {
      // Isolated spans compute base directionality during their own UBA run.
      // Do not insert fake embed characters once we enter an isolated span.
      krys_debug_assert(!InIsolate());

      krys_debug_assert(dir == BidiClass::PopDirectionalFormat || dir == BidiClass::LeftToRightEmbedding
                        || dir == BidiClass::LeftToRightOverride || dir == BidiClass::RightToLeftEmbedding
                        || dir == BidiClass::RightToLeftOverride);
      _currentExplicitEmbeddingSequence.push_back(BidiEmbedding {dir, source});
    }

    bool CommitExplicitEmbedding() noexcept;

    void CreateBidiRunsForLine(const Iterator &end, bool hardLineBreak = false) noexcept;

    KRYS_NODISCARD BidiRunList<Run> &Runs() noexcept
    {
      return _runs;
    }

    // FIXME: This used to be part of deleteRuns() but was a layering violation.
    // It's unclear if this is still needed.
    void MarkCurrentRunEmpty() noexcept
    {
      _emptyRun = true;
    }

    void SetWhitespaceCollapsingTransitionForIsolatedRun(Run &run, size_t transition) noexcept
    {
      krys_debug_assert(!_whitespaceCollapsingTransitionForIsolatedRun.contains(&run));
      _whitespaceCollapsingTransitionForIsolatedRun.emplace(&run, static_cast<uint32>(transition));
    }

    KRYS_NODISCARD uint32 WhitespaceCollapsingTransitionForIsolatedRun(Run &run) noexcept
    {
      auto it = _whitespaceCollapsingTransitionForIsolatedRun.find(&run);
      if (it == _whitespaceCollapsingTransitionForIsolatedRun.end())
      {
        return 0;
      }
      auto transition = it->second;
      _whitespaceCollapsingTransitionForIsolatedRun.erase(it);
      return transition;
    }

  protected:
    BidiResolverBase() noexcept = default;

    // FIXME: Instead of InlineBidiResolvers subclassing this method, we should
    // pass in some sort of Traits object which knows how to create runs for appending.
    void AppendRun() noexcept
    {
      static_cast<DerivedClass &>(*this).AppendRunInternal();
    }

    KRYS_NODISCARD bool NeedsContinuePastEnd() const noexcept
    {
      return static_cast<const DerivedClass &>(*this).NeedsContinuePastEndInternal();
    }

    void IncrementInternal() noexcept
    {
      _current.Increment();
    }

    void AppendRunInternal() noexcept;

    KRYS_NODISCARD bool NeedsContinuePastEndInternal() const noexcept
    {
      return true;
    }

  private:
    void RaiseExplicitEmbeddingLevel(BidiClass from, BidiClass to) noexcept;

    void LowerExplicitEmbeddingLevel(BidiClass from) noexcept;

    void CheckDirectionInLowerRaiseEmbeddingLevel() noexcept;

    void UpdateStatusLastFromCurrentDirection(BidiClass) noexcept;

    void ReorderRunsFromLevels() noexcept;
  };

  template <typename Iterator, typename Run>
  class BidiResolver : public BidiResolverBase<Iterator, Run, BidiResolver<Iterator, Run>>
  {
  };

  template <typename Iterator, typename Run, typename IsolateRun>
  class BidiResolverWithIsolate
      : public BidiResolverBase<Iterator, Run, BidiResolverWithIsolate<Iterator, Run, IsolateRun>>
  {
    friend class BidiResolverBase<Iterator, Run, BidiResolverWithIsolate<Iterator, Run, IsolateRun>>;

  private:
    List<IsolateRun> _isolatedRuns;

  public:
    BidiResolverWithIsolate() noexcept = default;

    ~BidiResolverWithIsolate() noexcept
    {
      // The owner of this resolver should have handled the isolated runs.
      krys_debug_assert(_isolatedRuns.empty());
    }

    KRYS_NODISCARD List<IsolateRun> &IsolatedRuns() noexcept
    {
      return _isolatedRuns;
    }

  protected:
    void IncrementInternal() noexcept;

    void AppendRunInternal() noexcept;

    KRYS_NODISCARD bool NeedsContinuePastEndInternal() const noexcept;
  };

  template <typename Iterator, typename Run, typename DerivedClass>
  void BidiResolverBase<Iterator, Run, DerivedClass>::AppendRunInternal() noexcept
  {
    if (!_emptyRun && !_eor.AtEnd())
    {
      auto startOffset = _sor.Offset();
      auto endOffset = _eor.Offset();
      auto stopOffset = _eor.NextOffset();

      if (!_endOfLine.AtEnd() && endOffset >= _endOfLine.Offset())
      {
        _reachedEndOfLine = true;
        endOffset = _endOfLine.Offset();
        stopOffset = _endOfLine.NextOffset();
      }

      if (endOffset >= startOffset)
      {
        _runs.AppendRun(CreateUnique<Run>(startOffset, stopOffset, Context(), _direction));
      }

      _eor.Increment();
      _sor = _eor;
    }

    _direction = BidiClass::OtherNeutral;
    _status.Eor = BidiClass::OtherNeutral;
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  void BidiResolverBase<Iterator, Run, DerivedClass>::CheckDirectionInLowerRaiseEmbeddingLevel() noexcept
  {
    krys_debug_assert(_status.Eor != BidiClass::OtherNeutral || _eor.AtEnd());
    krys_debug_assert(
      _status.Last != BidiClass::NonSpacingMark && _status.Last != BidiClass::BoundaryNeutral
      && _status.Last != BidiClass::RightToLeftEmbedding && _status.Last != BidiClass::LeftToRightEmbedding
      && _status.Last != BidiClass::RightToLeftOverride && _status.Last != BidiClass::LeftToRightOverride
      && _status.Last != BidiClass::PopDirectionalFormat);
    if (_direction == BidiClass::OtherNeutral)
    {
      _direction =
        _status.LastStrong == BidiClass::LeftToRight ? BidiClass::LeftToRight : BidiClass::RightToLeft;
    }
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  void BidiResolverBase<Iterator, Run, DerivedClass>::LowerExplicitEmbeddingLevel(BidiClass from) noexcept
  {
    if (!_emptyRun && _eor != _last)
    {
      CheckDirectionInLowerRaiseEmbeddingLevel();
      // bidi.sor ... bidi.eor ... bidi.last eor; need to append the bidi.sor-bidi.eor run or extend it
      // through bidi.last
      if (from == BidiClass::LeftToRight)
      {
        // bidi.sor ... bidi.eor ... bidi.last L
        if (_status.Eor == BidiClass::EuropeanNumber)
        {
          if (_status.LastStrong != BidiClass::LeftToRight)
          {
            _direction = BidiClass::EuropeanNumber;
            AppendRun();
          }
        }
        else if (_status.Eor == BidiClass::ArabicNumber)
        {
          _direction = BidiClass::ArabicNumber;
          AppendRun();
        }
        else if (_status.LastStrong != BidiClass::LeftToRight)
        {
          AppendRun();
          _direction = BidiClass::LeftToRight;
        }
      }
      else if (_status.Eor == BidiClass::EuropeanNumber || _status.Eor == BidiClass::ArabicNumber
               || _status.LastStrong == BidiClass::LeftToRight)
      {
        AppendRun();
        _direction = BidiClass::RightToLeft;
      }
      _eor = _last;
    }

    AppendRun();
    _emptyRun = true;

    // sor for the new run is determined by the higher level (rule X10)
    SetLastDir(from);
    SetLastStrongDir(from);
    _eor = Iterator();
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  void BidiResolverBase<Iterator, Run, DerivedClass>::RaiseExplicitEmbeddingLevel(BidiClass from,
                                                                                  BidiClass to) noexcept
  {
    if (!_emptyRun && _eor != _last)
    {
      CheckDirectionInLowerRaiseEmbeddingLevel();
      // bidi.sor ... bidi.eor ... bidi.last eor; need to append the bidi.sor-bidi.eor run or extend it
      // through bidi.last
      if (to == BidiClass::LeftToRight)
      {
        // bidi.sor ... bidi.eor ... bidi.last L
        if (_status.Eor == BidiClass::EuropeanNumber)
        {
          if (_status.LastStrong != BidiClass::LeftToRight)
          {
            _direction = BidiClass::EuropeanNumber;
            AppendRun();
          }
        }
        else if (_status.Eor == BidiClass::ArabicNumber)
        {
          _direction = BidiClass::ArabicNumber;
          AppendRun();
        }
        else if (_status.LastStrong != BidiClass::LeftToRight && from == BidiClass::LeftToRight)
        {
          AppendRun();
          _direction = BidiClass::LeftToRight;
        }
      }
      else if (_status.Eor == BidiClass::ArabicNumber
               || (_status.Eor == BidiClass::EuropeanNumber
                   && (_status.LastStrong != BidiClass::LeftToRight || from == BidiClass::RightToLeft))
               || (_status.Eor != BidiClass::EuropeanNumber && _status.LastStrong == BidiClass::LeftToRight
                   && from == BidiClass::RightToLeft))
      {
        AppendRun();
        _direction = BidiClass::RightToLeft;
      }
      _eor = _last;
    }

    AppendRun();
    _emptyRun = true;

    SetLastDir(to);
    SetLastStrongDir(to);
    _eor = Iterator();
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  bool BidiResolverBase<Iterator, Run, DerivedClass>::CommitExplicitEmbedding() noexcept
  {
    // When we're "inIsolate()" we're resolving the parent context which
    // ignores (skips over) the isolated content, including embedding levels.
    // We should never accrue embedding levels while skipping over isolated content.
    krys_debug_assert(!InIsolate() || _currentExplicitEmbeddingSequence.empty());

    auto fromLevel = Context()->Level();
    RefPtr<BidiContext> toContext = _status.Context;

    for (auto &embedding : _currentExplicitEmbeddingSequence)
    {
      if (embedding.Direction == BidiClass::PopDirectionalFormat)
      {
        if (auto *parentContext = toContext->Parent())
        {
          toContext = ShareRefPtr(parentContext);
        }
      }
      else
      {
        BidiClass direction = (embedding.Direction == BidiClass::RightToLeftEmbedding
                               || embedding.Direction == BidiClass::RightToLeftOverride)
                                ? BidiClass::RightToLeft
                                : BidiClass::LeftToRight;
        bool override = embedding.Direction == BidiClass::LeftToRightOverride
                        || embedding.Direction == BidiClass::RightToLeftOverride;
        uint8 level = toContext->Level();
        if (direction == BidiClass::RightToLeft)
        {
          level = NextGreaterOddLevel(level);
        }
        else
        {
          level = NextGreaterEvenLevel(level);
        }
        if (level < 61)
        {
          toContext = BidiContext::Create(level, direction, override, embedding.Source, toContext.get());
        }
      }
    }

    auto toLevel = toContext->Level();

    if (toLevel > fromLevel)
    {
      RaiseExplicitEmbeddingLevel(fromLevel % 2 ? BidiClass::RightToLeft : BidiClass::LeftToRight,
                                  toLevel % 2 ? BidiClass::RightToLeft : BidiClass::LeftToRight);
    }
    else if (toLevel < fromLevel)
    {
      LowerExplicitEmbeddingLevel(fromLevel % 2 ? BidiClass::RightToLeft : BidiClass::LeftToRight);
    }

    SetContext(krys::move(toContext));

    _currentExplicitEmbeddingSequence.clear();

    return fromLevel != toLevel;
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  inline void BidiResolverBase<Iterator, Run, DerivedClass>::UpdateStatusLastFromCurrentDirection(
    BidiClass dirCurrent) noexcept
  {
    switch (dirCurrent)
    {
      case BidiClass::EuropeanNumberTerminator:
        if (_status.Last != BidiClass::EuropeanNumber)
        {
          _status.Last = BidiClass::EuropeanNumberTerminator;
        }
        break;
      case BidiClass::EuropeanNumberSeparator:
      case BidiClass::CommonNumberSeparator:
      case BidiClass::SegmentSeparator:
      case BidiClass::WhiteSpaceNeutral:
      case BidiClass::OtherNeutral:
        switch (_status.Last)
        {
          case BidiClass::LeftToRight:
          case BidiClass::RightToLeft:
          case BidiClass::RightToLeftArabic:
          case BidiClass::EuropeanNumber:
          case BidiClass::ArabicNumber:      _status.Last = dirCurrent; break;
          default:                           _status.Last = BidiClass::OtherNeutral;
        }
        break;
      case BidiClass::NonSpacingMark:
      case BidiClass::BoundaryNeutral:
      case BidiClass::RightToLeftEmbedding:
      case BidiClass::LeftToRightEmbedding:
      case BidiClass::RightToLeftOverride:
      case BidiClass::LeftToRightOverride:
      case BidiClass::PopDirectionalFormat:
        // ignore these
        break;
      case BidiClass::EuropeanNumber: [[fallthrough]];
      default:                        _status.Last = dirCurrent;
    }
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  inline void BidiResolverBase<Iterator, Run, DerivedClass>::ReorderRunsFromLevels() noexcept
  {
    uint8 levelLow = 128;
    uint8 levelHigh = 0;
    for (Run *run = _runs.FirstRun(); run; run = run->Next())
    {
      levelHigh = std::max(run->Level(), levelHigh);
      levelLow = std::min(run->Level(), levelLow);
    }

    // This implements reordering of the line (L2 according to Bidi spec):
    // http://unicode.org/reports/tr9/#L2
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // Reversing is only done up to the lowest odd level.
    if (!(levelLow % 2))
    {
      levelLow++;
    }

    uint32 count = _runs.RunCount() - 1;

    while (levelHigh >= levelLow)
    {
      uint32 i = 0;
      Run *run = _runs.FirstRun();
      while (i < count)
      {
        for (; i < count && run && run->Level() < levelHigh; i++)
        {
          run = run->Next();
        }
        uint32 start = i;
        for (; i <= count && run && run->Level() >= levelHigh; i++)
        {
          run = run->Next();
        }
        uint32 end = i - 1;
        _runs.ReverseRuns(start, end);
      }
      levelHigh--;
    }
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  void BidiResolverBase<Iterator, Run, DerivedClass>::CreateBidiRunsForLine(const Iterator &end,
                                                                            bool hardLineBreak) noexcept
  {
    krys_debug_assert(_direction == BidiClass::OtherNeutral);

    _emptyRun = true;

    _eor = Iterator();

    _last = _current;
    bool pastEnd = false;
    BidiResolverBase<Iterator, Run, DerivedClass> stateAtEnd;

    while (true)
    {
      BidiClass dirCurrent;
      if (pastEnd && (hardLineBreak || _current.AtEnd()))
      {
        BidiContext *c = Context();
        if (hardLineBreak)
        {
          // A deviation from the Unicode Bidi Algorithm in order to match
          // WinIE and user expectations: hard line breaks reset bidi state
          // coming from unicode bidi control characters, but not those from
          // DOM nodes with specified directionality
          stateAtEnd.SetContext(c->CopyStackRemovingUnicodeEmbeddingContexts());

          dirCurrent = stateAtEnd.Context()->Dir();
          stateAtEnd.SetEorDir(dirCurrent);
          stateAtEnd.SetLastDir(dirCurrent);
          stateAtEnd.SetLastStrongDir(dirCurrent);
        }
        else
        {
          while (c->Parent())
          {
            c = c->Parent();
          }
          dirCurrent = c->Dir();
        }
      }
      else
      {
        dirCurrent = _current.Direction();
        if (Context()->Override() && dirCurrent != BidiClass::RightToLeftEmbedding
            && dirCurrent != BidiClass::LeftToRightEmbedding && dirCurrent != BidiClass::RightToLeftOverride
            && dirCurrent != BidiClass::LeftToRightOverride && dirCurrent != BidiClass::PopDirectionalFormat)
        {
          dirCurrent = Context()->Dir();
        }
        else if (dirCurrent == BidiClass::NonSpacingMark)
        {
          dirCurrent = _status.Last;
        }
      }

      // We ignore all character directionality while in unicode-bidi: isolate spans.
      // We'll handle ordering the isolated characters in a second pass.
      if (InIsolate() || dirCurrent == BidiClass::FirstStrongIsolate
          || dirCurrent == BidiClass::LeftToRightIsolate || dirCurrent == BidiClass::RightToLeftIsolate
          || dirCurrent == BidiClass::PopDirectionalIsolate)
      {
        dirCurrent = BidiClass::OtherNeutral;
      }

      krys_debug_assert(_status.Eor != BidiClass::OtherNeutral || _eor.AtEnd());
      switch (dirCurrent)
      {
          // embedding and overrides (X1-X9 in the Bidi specs)
        case BidiClass::RightToLeftEmbedding:
        case BidiClass::LeftToRightEmbedding:
        case BidiClass::RightToLeftOverride:
        case BidiClass::LeftToRightOverride:
        case BidiClass::PopDirectionalFormat:
          Embed(dirCurrent, BidiEmbeddingSource::FromUnicode);
          CommitExplicitEmbedding();
          break;

          // strong types
        case BidiClass::LeftToRight:
          switch (_status.Last)
          {
            case BidiClass::RightToLeft:
            case BidiClass::RightToLeftArabic:
            case BidiClass::EuropeanNumber:
            case BidiClass::ArabicNumber:
              if (_status.Last != BidiClass::EuropeanNumber || _status.LastStrong != BidiClass::LeftToRight)
              {
                AppendRun();
              }
              break;
            case BidiClass::LeftToRight: break;
            case BidiClass::EuropeanNumberSeparator:
            case BidiClass::EuropeanNumberTerminator:
            case BidiClass::CommonNumberSeparator:
            case BidiClass::BoundaryNeutral:
            case BidiClass::BlockSeparator:
            case BidiClass::SegmentSeparator:
            case BidiClass::WhiteSpaceNeutral:
            case BidiClass::OtherNeutral:
              if (_status.Eor == BidiClass::EuropeanNumber)
              {
                if (_status.LastStrong != BidiClass::LeftToRight)
                {
                  // the numbers need to be on a higher embedding level, so let's close that run
                  _direction = BidiClass::EuropeanNumber;
                  AppendRun();
                  if (Context()->Dir() != BidiClass::LeftToRight)
                  {
                    // the neutrals take the embedding direction, which is R
                    _eor = _last;
                    _direction = BidiClass::RightToLeft;
                    AppendRun();
                  }
                }
              }
              else if (_status.Eor == BidiClass::ArabicNumber)
              {
                // Arabic numbers are always on a higher embedding level, so let's close that run
                _direction = BidiClass::ArabicNumber;
                AppendRun();
                if (Context()->Dir() != BidiClass::LeftToRight)
                {
                  // the neutrals take the embedding direction, which is R
                  _eor = _last;
                  _direction = BidiClass::RightToLeft;
                  AppendRun();
                }
              }
              else if (_status.LastStrong != BidiClass::LeftToRight)
              {
                // last stuff takes embedding dir
                if (Context()->Dir() == BidiClass::RightToLeft)
                {
                  _eor = _last;
                  _direction = BidiClass::RightToLeft;
                }
                AppendRun();
              }
              break;
            default: break;
          }
          _eor = _current;
          _status.Eor = BidiClass::LeftToRight;
          _status.LastStrong = BidiClass::LeftToRight;
          _direction = BidiClass::LeftToRight;
          break;
        case BidiClass::RightToLeftArabic:
        case BidiClass::RightToLeft:
          switch (_status.Last)
          {
            case BidiClass::LeftToRight:
            case BidiClass::EuropeanNumber:
            case BidiClass::ArabicNumber:      AppendRun(); [[fallthrough]];
            case BidiClass::RightToLeft:
            case BidiClass::RightToLeftArabic: break;
            case BidiClass::EuropeanNumberSeparator:
            case BidiClass::EuropeanNumberTerminator:
            case BidiClass::CommonNumberSeparator:
            case BidiClass::BoundaryNeutral:
            case BidiClass::BlockSeparator:
            case BidiClass::SegmentSeparator:
            case BidiClass::WhiteSpaceNeutral:
            case BidiClass::OtherNeutral:
              if (_status.Eor == BidiClass::EuropeanNumber)
              {
                if (_status.LastStrong == BidiClass::LeftToRight
                    && Context()->Dir() == BidiClass::LeftToRight)
                {
                  _eor = _last;
                }
                AppendRun();
              }
              else if (_status.Eor == BidiClass::ArabicNumber)
              {
                AppendRun();
              }
              else if (_status.LastStrong == BidiClass::LeftToRight)
              {
                if (Context()->Dir() == BidiClass::LeftToRight)
                {
                  _eor = _last;
                }
                AppendRun();
              }
              break;
            default: break;
          }
          _eor = _current;
          _status.Eor = BidiClass::RightToLeft;
          _status.LastStrong = dirCurrent;
          _direction = BidiClass::RightToLeft;
          break;

          // weak types:

        case BidiClass::EuropeanNumber:
          if (_status.LastStrong != BidiClass::RightToLeftArabic)
          {
            // if last strong was AL change EN to AN
            switch (_status.Last)
            {
              case BidiClass::EuropeanNumber:
              case BidiClass::LeftToRight:    break;
              case BidiClass::RightToLeft:
              case BidiClass::RightToLeftArabic:
              case BidiClass::ArabicNumber:
                _eor = _last;
                AppendRun();
                _direction = BidiClass::EuropeanNumber;
                break;
              case BidiClass::EuropeanNumberSeparator:
              case BidiClass::CommonNumberSeparator:
                if (_status.Eor == BidiClass::EuropeanNumber)
                {
                  break;
                }
                [[fallthrough]];
              case BidiClass::EuropeanNumberTerminator:
              case BidiClass::BoundaryNeutral:
              case BidiClass::BlockSeparator:
              case BidiClass::SegmentSeparator:
              case BidiClass::WhiteSpaceNeutral:
              case BidiClass::OtherNeutral:
                if (_status.Eor == BidiClass::EuropeanNumber)
                {
                  if (_status.LastStrong == BidiClass::RightToLeft)
                  {
                    // ENs on both sides behave like Rs, so the neutrals should be R.
                    // Terminate the EN run.
                    AppendRun();
                    // Make an R run.
                    _eor = _status.Last == BidiClass::EuropeanNumberTerminator ? _lastBeforeET : _last;
                    _direction = BidiClass::RightToLeft;
                    AppendRun();
                    // Begin a new EN run.
                    _direction = BidiClass::EuropeanNumber;
                  }
                }
                else if (_status.Eor == BidiClass::ArabicNumber)
                {
                  // Terminate the AN run.
                  AppendRun();
                  if (_status.LastStrong == BidiClass::RightToLeft
                      || Context()->Dir() == BidiClass::RightToLeft)
                  {
                    // Make an R run.
                    _eor = _status.Last == BidiClass::EuropeanNumberTerminator ? _lastBeforeET : _last;
                    _direction = BidiClass::RightToLeft;
                    AppendRun();
                    // Begin a new EN run.
                    _direction = BidiClass::EuropeanNumber;
                  }
                }
                else if (_status.LastStrong == BidiClass::RightToLeft)
                {
                  // Extend the R run to include the neutrals.
                  _eor = _status.Last == BidiClass::EuropeanNumberTerminator ? _lastBeforeET : _last;
                  _direction = BidiClass::RightToLeft;
                  AppendRun();
                  // Begin a new EN run.
                  _direction = BidiClass::EuropeanNumber;
                }
                break;
              default: break;
            }
            _eor = _current;
            _status.Eor = BidiClass::EuropeanNumber;
            if (_direction == BidiClass::OtherNeutral)
            {
              _direction = BidiClass::LeftToRight;
            }
            break;
          }
          [[fallthrough]];
        case BidiClass::ArabicNumber:
          dirCurrent = BidiClass::ArabicNumber;
          switch (_status.Last)
          {
            case BidiClass::LeftToRight:
              if (Context()->Dir() == BidiClass::LeftToRight)
              {
                AppendRun();
              }
              break;
            case BidiClass::ArabicNumber: break;
            case BidiClass::RightToLeft:
            case BidiClass::RightToLeftArabic:
            case BidiClass::EuropeanNumber:
              _eor = _last;
              AppendRun();
              break;
            case BidiClass::CommonNumberSeparator:
              if (_status.Eor == BidiClass::ArabicNumber)
              {
                break;
              }
              [[fallthrough]];
            case BidiClass::EuropeanNumberSeparator:
            case BidiClass::EuropeanNumberTerminator:
            case BidiClass::BoundaryNeutral:
            case BidiClass::BlockSeparator:
            case BidiClass::SegmentSeparator:
            case BidiClass::WhiteSpaceNeutral:
            case BidiClass::OtherNeutral:
              if (_status.Eor == BidiClass::ArabicNumber
                  || (_status.Eor == BidiClass::EuropeanNumber
                      && (_status.LastStrong == BidiClass::RightToLeft
                          || Context()->Dir() == BidiClass::RightToLeft))
                  || (_status.Eor != BidiClass::EuropeanNumber && _status.LastStrong == BidiClass::LeftToRight
                      && Context()->Dir() == BidiClass::RightToLeft))
              {
                // Terminate the run before the neutrals.
                AppendRun();
                // Begin an R run for the neutrals.
                _direction = BidiClass::RightToLeft;
              }
              else if (_direction == BidiClass::OtherNeutral)
              {
                _direction = _status.LastStrong == BidiClass::LeftToRight ? BidiClass::LeftToRight
                                                                          : BidiClass::RightToLeft;
              }
              _eor = _last;
              AppendRun();
              break;
            default: break;
          }
          _eor = _current;
          _status.Eor = BidiClass::ArabicNumber;
          if (_direction == BidiClass::OtherNeutral)
          {
            _direction = BidiClass::ArabicNumber;
          }
          break;
        case BidiClass::EuropeanNumberSeparator:
        case BidiClass::CommonNumberSeparator:   break;
        case BidiClass::EuropeanNumberTerminator:
          if (_status.Last == BidiClass::EuropeanNumber)
          {
            dirCurrent = BidiClass::EuropeanNumber;
            _eor = _current;
            _status.Eor = dirCurrent;
          }
          else if (_status.Last != BidiClass::EuropeanNumberTerminator)
          {
            _lastBeforeET = _emptyRun ? _eor : _last;
          }
          break;

          // boundary neutrals should be ignored
        case BidiClass::BoundaryNeutral:
          if (_eor == _last)
          {
            _eor = _current;
          }
          break;
          // neutrals
        // NOLINTNEXTLINE(bugprone-branch-clone): each has its own FIXME.
        case BidiClass::BlockSeparator:
          // FIXME: What do we do with newline and paragraph separators that come to here?
          break;
        case BidiClass::SegmentSeparator:
          // FIXME: Implement rule L1.
          break;
        case BidiClass::WhiteSpaceNeutral: break;
        case BidiClass::OtherNeutral:      break;
        default:                           break;
      }

      if (pastEnd && (_eor == _current || !NeedsContinuePastEnd()))
      {
        if (!_reachedEndOfLine)
        {
          _eor = _endOfLine;
          switch (_status.Eor)
          {
            case BidiClass::LeftToRight:
            case BidiClass::RightToLeft:
            case BidiClass::ArabicNumber: _direction = _status.Eor; break;
            case BidiClass::EuropeanNumber:
              _direction = _status.LastStrong == BidiClass::LeftToRight ? BidiClass::LeftToRight
                                                                        : BidiClass::EuropeanNumber;
              break;
            default:
              // FIXME: handle neutrals in this case. See https://bugs.webkit.org/show_bug.cgi?id=204817
              break;
          }
          AppendRun();
        }
        _current = end;
        _status = stateAtEnd._status;
        _sor = stateAtEnd._sor;
        _eor = stateAtEnd._eor;
        _last = stateAtEnd._last;
        _reachedEndOfLine = stateAtEnd._reachedEndOfLine;
        _lastBeforeET = stateAtEnd._lastBeforeET;
        _emptyRun = stateAtEnd._emptyRun;
        _direction = BidiClass::OtherNeutral;
        break;
      }

      UpdateStatusLastFromCurrentDirection(dirCurrent);
      _last = _current;

      if (_emptyRun)
      {
        _sor = _current;
        _emptyRun = false;
      }

      Increment();
      if (!_currentExplicitEmbeddingSequence.empty())
      {
        bool committed = CommitExplicitEmbedding();
        if (committed && pastEnd)
        {
          _current = end;
          _status = stateAtEnd._status;
          _sor = stateAtEnd._sor;
          _eor = stateAtEnd._eor;
          _last = stateAtEnd._last;
          _reachedEndOfLine = stateAtEnd._reachedEndOfLine;
          _lastBeforeET = stateAtEnd._lastBeforeET;
          _emptyRun = stateAtEnd._emptyRun;
          _direction = BidiClass::OtherNeutral;
          break;
        }
      }

      if (!pastEnd && (_current == end || _current.AtEnd()))
      {
        if (_emptyRun)
        {
          break;
        }
        stateAtEnd._status = _status;
        stateAtEnd._sor = _sor;
        stateAtEnd._eor = _eor;
        stateAtEnd._last = _last;
        stateAtEnd._reachedEndOfLine = _reachedEndOfLine;
        stateAtEnd._lastBeforeET = _lastBeforeET;
        stateAtEnd._emptyRun = _emptyRun;
        _endOfLine = _last;
        pastEnd = true;
      }
    }

    _runs.SetLogicallyLastRun(_runs.LastRun());
    ReorderRunsFromLevels();
    _endOfLine = Iterator();
  }
}
