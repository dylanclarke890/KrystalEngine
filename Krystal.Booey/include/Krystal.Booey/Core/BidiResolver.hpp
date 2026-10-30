#pragma once

#include "Krystal.Booey/Core/BidiContext.hpp"
#include "Krystal.Booey/Core/BidiRunList.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <algorithm>

namespace krys::boo
{
  class RenderObject;

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
    /// RenderInlines or hard line breaks to ensure that they're not ignored.
    void EnsureLineBoxInsideIgnoredSpaces(RenderObject &renderer) noexcept
    {
      Iterator transition(nullptr, &renderer, 0);
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
    UCharDirection Eor {U_OTHER_NEUTRAL};
    UCharDirection LastStrong {U_OTHER_NEUTRAL};
    UCharDirection Last {U_OTHER_NEUTRAL};
    RefPtr<BidiContext> Context;

    BidiStatus() noexcept = default;

    /// @brief Creates a BidiStatus representing a new paragraph root with a default direction.
    /// Uses TextDirection as it only has two possibilities instead of UCharDirection which has at least 19.
    BidiStatus(TextDirection direction, bool isOverride) noexcept
        : Eor(direction == TextDirection::LTR ? U_LEFT_TO_RIGHT : U_RIGHT_TO_LEFT), LastStrong(Eor),
          Last(Eor), Context(BidiContext::Create(direction == TextDirection::LTR ? 0 : 1, Eor, isOverride))
    {
    }

    BidiStatus(UCharDirection eor, UCharDirection lastStrong, UCharDirection last,
               RefPtr<BidiContext> &&context) noexcept
        : Eor(eor), LastStrong(lastStrong), Last(last), Context(krys::move(context))
    {
    }
  };

  struct BidiEmbedding
  {
    UCharDirection Direction;
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
    BidiCharacterRun(uint32 start, uint32 stop, BidiContext *context, UCharDirection direction) noexcept
        : _start(start), _stop(stop), _override(context->Override())
    {
      if (direction == U_OTHER_NEUTRAL)
      {
        direction = context->Dir();
      }

      _level = context->Level();

      // add level of run (cases I1 & I2)
      if (_level % 2)
      {
        if (direction == U_LEFT_TO_RIGHT || direction == U_ARABIC_NUMBER || direction == U_EUROPEAN_NUMBER)
        {
          _level++;
        }
      }
      else
      {
        if (direction == U_RIGHT_TO_LEFT)
        {
          _level++;
        }
        else if (direction == U_ARABIC_NUMBER || direction == U_EUROPEAN_NUMBER)
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

  enum class VisualDirectionOverride : uint8
  {
    NoVisualOverride,
    VisualLeftToRightOverride,
    VisualRightToLeftOverride
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
    UCharDirection _direction {U_OTHER_NEUTRAL};
    Iterator _endOfLine;
    bool _reachedEndOfLine {false};
    Iterator _lastBeforeET; // Before a U_EUROPEAN_NUMBER_TERMINATOR
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

    void SetLastDir(UCharDirection lastDir) noexcept
    {
      _status.Last = lastDir;
    }
    void SetLastStrongDir(UCharDirection lastStrongDir) noexcept
    {
      _status.LastStrong = lastStrongDir;
    }
    void SetEorDir(UCharDirection eorDir) noexcept
    {
      _status.Eor = eorDir;
    }

    KRYS_NODISCARD UCharDirection Dir() const noexcept
    {
      return _direction;
    }
    void SetDir(UCharDirection direction) noexcept
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

    void Embed(UCharDirection dir, BidiEmbeddingSource source) noexcept
    {
      // Isolated spans compute base directionality during their own UBA run.
      // Do not insert fake embed characters once we enter an isolated span.
      krys_debug_assert(!InIsolate());

      krys_debug_assert(dir == U_POP_DIRECTIONAL_FORMAT || dir == U_LEFT_TO_RIGHT_EMBEDDING
                        || dir == U_LEFT_TO_RIGHT_OVERRIDE || dir == U_RIGHT_TO_LEFT_EMBEDDING
                        || dir == U_RIGHT_TO_LEFT_OVERRIDE);
      _currentExplicitEmbeddingSequence.push_back(BidiEmbedding {dir, source});
    }

    bool CommitExplicitEmbedding() noexcept;

    void CreateBidiRunsForLine(const Iterator &end,
                               VisualDirectionOverride = VisualDirectionOverride::NoVisualOverride,
                               bool hardLineBreak = false) noexcept;

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
    void RaiseExplicitEmbeddingLevel(UCharDirection from, UCharDirection to) noexcept;
    void LowerExplicitEmbeddingLevel(UCharDirection from) noexcept;
    void CheckDirectionInLowerRaiseEmbeddingLevel() noexcept;

    void UpdateStatusLastFromCurrentDirection(UCharDirection) noexcept;
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

    _direction = U_OTHER_NEUTRAL;
    _status.Eor = U_OTHER_NEUTRAL;
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  void BidiResolverBase<Iterator, Run, DerivedClass>::CheckDirectionInLowerRaiseEmbeddingLevel() noexcept
  {
    krys_debug_assert(_status.Eor != U_OTHER_NEUTRAL || _eor.AtEnd());
    krys_debug_assert(_status.Last != U_DIR_NON_SPACING_MARK && _status.Last != U_BOUNDARY_NEUTRAL
                      && _status.Last != U_RIGHT_TO_LEFT_EMBEDDING
                      && _status.Last != U_LEFT_TO_RIGHT_EMBEDDING && _status.Last != U_RIGHT_TO_LEFT_OVERRIDE
                      && _status.Last != U_LEFT_TO_RIGHT_OVERRIDE
                      && _status.Last != U_POP_DIRECTIONAL_FORMAT);
    if (_direction == U_OTHER_NEUTRAL)
    {
      _direction = _status.LastStrong == U_LEFT_TO_RIGHT ? U_LEFT_TO_RIGHT : U_RIGHT_TO_LEFT;
    }
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  void
    BidiResolverBase<Iterator, Run, DerivedClass>::LowerExplicitEmbeddingLevel(UCharDirection from) noexcept
  {
    if (!_emptyRun && _eor != _last)
    {
      CheckDirectionInLowerRaiseEmbeddingLevel();
      // bidi.sor ... bidi.eor ... bidi.last eor; need to append the bidi.sor-bidi.eor run or extend it
      // through bidi.last
      if (from == U_LEFT_TO_RIGHT)
      {
        // bidi.sor ... bidi.eor ... bidi.last L
        if (_status.Eor == U_EUROPEAN_NUMBER)
        {
          if (_status.LastStrong != U_LEFT_TO_RIGHT)
          {
            _direction = U_EUROPEAN_NUMBER;
            AppendRun();
          }
        }
        else if (_status.Eor == U_ARABIC_NUMBER)
        {
          _direction = U_ARABIC_NUMBER;
          AppendRun();
        }
        else if (_status.LastStrong != U_LEFT_TO_RIGHT)
        {
          AppendRun();
          _direction = U_LEFT_TO_RIGHT;
        }
      }
      else if (_status.Eor == U_EUROPEAN_NUMBER || _status.Eor == U_ARABIC_NUMBER
               || _status.LastStrong == U_LEFT_TO_RIGHT)
      {
        AppendRun();
        _direction = U_RIGHT_TO_LEFT;
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
  void BidiResolverBase<Iterator, Run, DerivedClass>::RaiseExplicitEmbeddingLevel(UCharDirection from,
                                                                                  UCharDirection to) noexcept
  {
    if (!_emptyRun && _eor != _last)
    {
      CheckDirectionInLowerRaiseEmbeddingLevel();
      // bidi.sor ... bidi.eor ... bidi.last eor; need to append the bidi.sor-bidi.eor run or extend it
      // through bidi.last
      if (to == U_LEFT_TO_RIGHT)
      {
        // bidi.sor ... bidi.eor ... bidi.last L
        if (_status.Eor == U_EUROPEAN_NUMBER)
        {
          if (_status.LastStrong != U_LEFT_TO_RIGHT)
          {
            _direction = U_EUROPEAN_NUMBER;
            AppendRun();
          }
        }
        else if (_status.Eor == U_ARABIC_NUMBER)
        {
          _direction = U_ARABIC_NUMBER;
          AppendRun();
        }
        else if (_status.LastStrong != U_LEFT_TO_RIGHT && from == U_LEFT_TO_RIGHT)
        {
          AppendRun();
          _direction = U_LEFT_TO_RIGHT;
        }
      }
      else if (_status.Eor == U_ARABIC_NUMBER
               || (_status.Eor == U_EUROPEAN_NUMBER
                   && (_status.LastStrong != U_LEFT_TO_RIGHT || from == U_RIGHT_TO_LEFT))
               || (_status.Eor != U_EUROPEAN_NUMBER && _status.LastStrong == U_LEFT_TO_RIGHT
                   && from == U_RIGHT_TO_LEFT))
      {
        AppendRun();
        _direction = U_RIGHT_TO_LEFT;
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
      if (embedding.Direction == U_POP_DIRECTIONAL_FORMAT)
      {
        if (auto *parentContext = toContext->Parent())
        {
          toContext = ShareRefPtr(parentContext);
        }
      }
      else
      {
        UCharDirection direction = (embedding.Direction == U_RIGHT_TO_LEFT_EMBEDDING
                                    || embedding.Direction == U_RIGHT_TO_LEFT_OVERRIDE)
                                     ? U_RIGHT_TO_LEFT
                                     : U_LEFT_TO_RIGHT;
        bool override =
          embedding.Direction == U_LEFT_TO_RIGHT_OVERRIDE || embedding.Direction == U_RIGHT_TO_LEFT_OVERRIDE;
        uint8 level = toContext->Level();
        if (direction == U_RIGHT_TO_LEFT)
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
      RaiseExplicitEmbeddingLevel(fromLevel % 2 ? U_RIGHT_TO_LEFT : U_LEFT_TO_RIGHT,
                                  toLevel % 2 ? U_RIGHT_TO_LEFT : U_LEFT_TO_RIGHT);
    }
    else if (toLevel < fromLevel)
    {
      LowerExplicitEmbeddingLevel(fromLevel % 2 ? U_RIGHT_TO_LEFT : U_LEFT_TO_RIGHT);
    }

    SetContext(krys::move(toContext));

    _currentExplicitEmbeddingSequence.clear();

    return fromLevel != toLevel;
  }

  template <typename Iterator, typename Run, typename DerivedClass>
  inline void BidiResolverBase<Iterator, Run, DerivedClass>::UpdateStatusLastFromCurrentDirection(
    UCharDirection dirCurrent) noexcept
  {
    switch (dirCurrent)
    {
      case U_EUROPEAN_NUMBER_TERMINATOR:
        if (_status.Last != U_EUROPEAN_NUMBER)
        {
          _status.Last = U_EUROPEAN_NUMBER_TERMINATOR;
        }
        break;
      case U_EUROPEAN_NUMBER_SEPARATOR:
      case U_COMMON_NUMBER_SEPARATOR:
      case U_SEGMENT_SEPARATOR:
      case U_WHITE_SPACE_NEUTRAL:
      case U_OTHER_NEUTRAL:
        switch (_status.Last)
        {
          case U_LEFT_TO_RIGHT:
          case U_RIGHT_TO_LEFT:
          case U_RIGHT_TO_LEFT_ARABIC:
          case U_EUROPEAN_NUMBER:
          case U_ARABIC_NUMBER:        _status.Last = dirCurrent; break;
          default:                     _status.Last = U_OTHER_NEUTRAL;
        }
        break;
      case U_DIR_NON_SPACING_MARK:
      case U_BOUNDARY_NEUTRAL:
      case U_RIGHT_TO_LEFT_EMBEDDING:
      case U_LEFT_TO_RIGHT_EMBEDDING:
      case U_RIGHT_TO_LEFT_OVERRIDE:
      case U_LEFT_TO_RIGHT_OVERRIDE:
      case U_POP_DIRECTIONAL_FORMAT:
        // ignore these
        break;
      case U_EUROPEAN_NUMBER: [[fallthrough]];
      default:                _status.Last = dirCurrent;
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
                                                                            VisualDirectionOverride override,
                                                                            bool hardLineBreak) noexcept
  {
    krys_debug_assert(_direction == U_OTHER_NEUTRAL);

    if (override != VisualDirectionOverride::NoVisualOverride)
    {
      _emptyRun = false;
      _sor = _current;
      _eor = Iterator();
      while (_current != end && !_current.AtEnd())
      {
        _eor = _current;
        Increment();
      }
      _direction =
        override == VisualDirectionOverride::VisualLeftToRightOverride ? U_LEFT_TO_RIGHT : U_RIGHT_TO_LEFT;
      AppendRun();
      _runs.SetLogicallyLastRun(_runs.LastRun());
      if (override == VisualDirectionOverride::VisualRightToLeftOverride && _runs.RunCount())
      {
        _runs.ReverseRuns(0, _runs.RunCount() - 1);
      }
      return;
    }

    _emptyRun = true;

    _eor = Iterator();

    _last = _current;
    bool pastEnd = false;
    BidiResolverBase<Iterator, Run, DerivedClass> stateAtEnd;

    while (true)
    {
      UCharDirection dirCurrent;
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
        if (Context()->Override() && dirCurrent != U_RIGHT_TO_LEFT_EMBEDDING
            && dirCurrent != U_LEFT_TO_RIGHT_EMBEDDING && dirCurrent != U_RIGHT_TO_LEFT_OVERRIDE
            && dirCurrent != U_LEFT_TO_RIGHT_OVERRIDE && dirCurrent != U_POP_DIRECTIONAL_FORMAT)
        {
          dirCurrent = Context()->Dir();
        }
        else if (dirCurrent == U_DIR_NON_SPACING_MARK)
        {
          dirCurrent = _status.Last;
        }
      }

      // We ignore all character directionality while in unicode-bidi: isolate spans.
      // We'll handle ordering the isolated characters in a second pass.
      if (InIsolate() || dirCurrent == U_FIRST_STRONG_ISOLATE || dirCurrent == U_LEFT_TO_RIGHT_ISOLATE
          || dirCurrent == U_RIGHT_TO_LEFT_ISOLATE || dirCurrent == U_POP_DIRECTIONAL_ISOLATE)
      {
        dirCurrent = U_OTHER_NEUTRAL;
      }

      krys_debug_assert(_status.Eor != U_OTHER_NEUTRAL || _eor.AtEnd());
      switch (dirCurrent)
      {
          // embedding and overrides (X1-X9 in the Bidi specs)
        case U_RIGHT_TO_LEFT_EMBEDDING:
        case U_LEFT_TO_RIGHT_EMBEDDING:
        case U_RIGHT_TO_LEFT_OVERRIDE:
        case U_LEFT_TO_RIGHT_OVERRIDE:
        case U_POP_DIRECTIONAL_FORMAT:
          Embed(dirCurrent, BidiEmbeddingSource::FromUnicode);
          CommitExplicitEmbedding();
          break;

          // strong types
        case U_LEFT_TO_RIGHT:
          switch (_status.Last)
          {
            case U_RIGHT_TO_LEFT:
            case U_RIGHT_TO_LEFT_ARABIC:
            case U_EUROPEAN_NUMBER:
            case U_ARABIC_NUMBER:
              if (_status.Last != U_EUROPEAN_NUMBER || _status.LastStrong != U_LEFT_TO_RIGHT)
              {
                AppendRun();
              }
              break;
            case U_LEFT_TO_RIGHT: break;
            case U_EUROPEAN_NUMBER_SEPARATOR:
            case U_EUROPEAN_NUMBER_TERMINATOR:
            case U_COMMON_NUMBER_SEPARATOR:
            case U_BOUNDARY_NEUTRAL:
            case U_BLOCK_SEPARATOR:
            case U_SEGMENT_SEPARATOR:
            case U_WHITE_SPACE_NEUTRAL:
            case U_OTHER_NEUTRAL:
              if (_status.Eor == U_EUROPEAN_NUMBER)
              {
                if (_status.LastStrong != U_LEFT_TO_RIGHT)
                {
                  // the numbers need to be on a higher embedding level, so let's close that run
                  _direction = U_EUROPEAN_NUMBER;
                  AppendRun();
                  if (Context()->Dir() != U_LEFT_TO_RIGHT)
                  {
                    // the neutrals take the embedding direction, which is R
                    _eor = _last;
                    _direction = U_RIGHT_TO_LEFT;
                    AppendRun();
                  }
                }
              }
              else if (_status.Eor == U_ARABIC_NUMBER)
              {
                // Arabic numbers are always on a higher embedding level, so let's close that run
                _direction = U_ARABIC_NUMBER;
                AppendRun();
                if (Context()->Dir() != U_LEFT_TO_RIGHT)
                {
                  // the neutrals take the embedding direction, which is R
                  _eor = _last;
                  _direction = U_RIGHT_TO_LEFT;
                  AppendRun();
                }
              }
              else if (_status.LastStrong != U_LEFT_TO_RIGHT)
              {
                // last stuff takes embedding dir
                if (Context()->Dir() == U_RIGHT_TO_LEFT)
                {
                  _eor = _last;
                  _direction = U_RIGHT_TO_LEFT;
                }
                AppendRun();
              }
              break;
            default: break;
          }
          _eor = _current;
          _status.Eor = U_LEFT_TO_RIGHT;
          _status.LastStrong = U_LEFT_TO_RIGHT;
          _direction = U_LEFT_TO_RIGHT;
          break;
        case U_RIGHT_TO_LEFT_ARABIC:
        case U_RIGHT_TO_LEFT:
          switch (_status.Last)
          {
            case U_LEFT_TO_RIGHT:
            case U_EUROPEAN_NUMBER:
            case U_ARABIC_NUMBER:        AppendRun(); [[fallthrough]];
            case U_RIGHT_TO_LEFT:
            case U_RIGHT_TO_LEFT_ARABIC: break;
            case U_EUROPEAN_NUMBER_SEPARATOR:
            case U_EUROPEAN_NUMBER_TERMINATOR:
            case U_COMMON_NUMBER_SEPARATOR:
            case U_BOUNDARY_NEUTRAL:
            case U_BLOCK_SEPARATOR:
            case U_SEGMENT_SEPARATOR:
            case U_WHITE_SPACE_NEUTRAL:
            case U_OTHER_NEUTRAL:
              if (_status.Eor == U_EUROPEAN_NUMBER)
              {
                if (_status.LastStrong == U_LEFT_TO_RIGHT && Context()->Dir() == U_LEFT_TO_RIGHT)
                {
                  _eor = _last;
                }
                AppendRun();
              }
              else if (_status.Eor == U_ARABIC_NUMBER)
              {
                AppendRun();
              }
              else if (_status.LastStrong == U_LEFT_TO_RIGHT)
              {
                if (Context()->Dir() == U_LEFT_TO_RIGHT)
                {
                  _eor = _last;
                }
                AppendRun();
              }
              break;
            default: break;
          }
          _eor = _current;
          _status.Eor = U_RIGHT_TO_LEFT;
          _status.LastStrong = dirCurrent;
          _direction = U_RIGHT_TO_LEFT;
          break;

          // weak types:

        case U_EUROPEAN_NUMBER:
          if (_status.LastStrong != U_RIGHT_TO_LEFT_ARABIC)
          {
            // if last strong was AL change EN to AN
            switch (_status.Last)
            {
              case U_EUROPEAN_NUMBER:
              case U_LEFT_TO_RIGHT:   break;
              case U_RIGHT_TO_LEFT:
              case U_RIGHT_TO_LEFT_ARABIC:
              case U_ARABIC_NUMBER:
                _eor = _last;
                AppendRun();
                _direction = U_EUROPEAN_NUMBER;
                break;
              case U_EUROPEAN_NUMBER_SEPARATOR:
              case U_COMMON_NUMBER_SEPARATOR:
                if (_status.Eor == U_EUROPEAN_NUMBER)
                {
                  break;
                }
                [[fallthrough]];
              case U_EUROPEAN_NUMBER_TERMINATOR:
              case U_BOUNDARY_NEUTRAL:
              case U_BLOCK_SEPARATOR:
              case U_SEGMENT_SEPARATOR:
              case U_WHITE_SPACE_NEUTRAL:
              case U_OTHER_NEUTRAL:
                if (_status.Eor == U_EUROPEAN_NUMBER)
                {
                  if (_status.LastStrong == U_RIGHT_TO_LEFT)
                  {
                    // ENs on both sides behave like Rs, so the neutrals should be R.
                    // Terminate the EN run.
                    AppendRun();
                    // Make an R run.
                    _eor = _status.Last == U_EUROPEAN_NUMBER_TERMINATOR ? _lastBeforeET : _last;
                    _direction = U_RIGHT_TO_LEFT;
                    AppendRun();
                    // Begin a new EN run.
                    _direction = U_EUROPEAN_NUMBER;
                  }
                }
                else if (_status.Eor == U_ARABIC_NUMBER)
                {
                  // Terminate the AN run.
                  AppendRun();
                  if (_status.LastStrong == U_RIGHT_TO_LEFT || Context()->Dir() == U_RIGHT_TO_LEFT)
                  {
                    // Make an R run.
                    _eor = _status.Last == U_EUROPEAN_NUMBER_TERMINATOR ? _lastBeforeET : _last;
                    _direction = U_RIGHT_TO_LEFT;
                    AppendRun();
                    // Begin a new EN run.
                    _direction = U_EUROPEAN_NUMBER;
                  }
                }
                else if (_status.LastStrong == U_RIGHT_TO_LEFT)
                {
                  // Extend the R run to include the neutrals.
                  _eor = _status.Last == U_EUROPEAN_NUMBER_TERMINATOR ? _lastBeforeET : _last;
                  _direction = U_RIGHT_TO_LEFT;
                  AppendRun();
                  // Begin a new EN run.
                  _direction = U_EUROPEAN_NUMBER;
                }
                break;
              default: break;
            }
            _eor = _current;
            _status.Eor = U_EUROPEAN_NUMBER;
            if (_direction == U_OTHER_NEUTRAL)
            {
              _direction = U_LEFT_TO_RIGHT;
            }
            break;
          }
          [[fallthrough]];
        case U_ARABIC_NUMBER:
          dirCurrent = U_ARABIC_NUMBER;
          switch (_status.Last)
          {
            case U_LEFT_TO_RIGHT:
              if (Context()->Dir() == U_LEFT_TO_RIGHT)
              {
                AppendRun();
              }
              break;
            case U_ARABIC_NUMBER: break;
            case U_RIGHT_TO_LEFT:
            case U_RIGHT_TO_LEFT_ARABIC:
            case U_EUROPEAN_NUMBER:
              _eor = _last;
              AppendRun();
              break;
            case U_COMMON_NUMBER_SEPARATOR:
              if (_status.Eor == U_ARABIC_NUMBER)
              {
                break;
              }
              [[fallthrough]];
            case U_EUROPEAN_NUMBER_SEPARATOR:
            case U_EUROPEAN_NUMBER_TERMINATOR:
            case U_BOUNDARY_NEUTRAL:
            case U_BLOCK_SEPARATOR:
            case U_SEGMENT_SEPARATOR:
            case U_WHITE_SPACE_NEUTRAL:
            case U_OTHER_NEUTRAL:
              if (_status.Eor == U_ARABIC_NUMBER
                  || (_status.Eor == U_EUROPEAN_NUMBER
                      && (_status.LastStrong == U_RIGHT_TO_LEFT || Context()->Dir() == U_RIGHT_TO_LEFT))
                  || (_status.Eor != U_EUROPEAN_NUMBER && _status.LastStrong == U_LEFT_TO_RIGHT
                      && Context()->Dir() == U_RIGHT_TO_LEFT))
              {
                // Terminate the run before the neutrals.
                AppendRun();
                // Begin an R run for the neutrals.
                _direction = U_RIGHT_TO_LEFT;
              }
              else if (_direction == U_OTHER_NEUTRAL)
              {
                _direction = _status.LastStrong == U_LEFT_TO_RIGHT ? U_LEFT_TO_RIGHT : U_RIGHT_TO_LEFT;
              }
              _eor = _last;
              AppendRun();
              break;
            default: break;
          }
          _eor = _current;
          _status.Eor = U_ARABIC_NUMBER;
          if (_direction == U_OTHER_NEUTRAL)
          {
            _direction = U_ARABIC_NUMBER;
          }
          break;
        case U_EUROPEAN_NUMBER_SEPARATOR:
        case U_COMMON_NUMBER_SEPARATOR:   break;
        case U_EUROPEAN_NUMBER_TERMINATOR:
          if (_status.Last == U_EUROPEAN_NUMBER)
          {
            dirCurrent = U_EUROPEAN_NUMBER;
            _eor = _current;
            _status.Eor = dirCurrent;
          }
          else if (_status.Last != U_EUROPEAN_NUMBER_TERMINATOR)
          {
            _lastBeforeET = _emptyRun ? _eor : _last;
          }
          break;

          // boundary neutrals should be ignored
        case U_BOUNDARY_NEUTRAL:
          if (_eor == _last)
          {
            _eor = _current;
          }
          break;
          // neutrals
        case U_BLOCK_SEPARATOR:
          // FIXME: What do we do with newline and paragraph separators that come to here?
          break;
        case U_SEGMENT_SEPARATOR:
          // FIXME: Implement rule L1.
          break;
        case U_WHITE_SPACE_NEUTRAL: break;
        case U_OTHER_NEUTRAL:       break;
        default:                    break;
      }

      if (pastEnd && (_eor == _current || !NeedsContinuePastEnd()))
      {
        if (!_reachedEndOfLine)
        {
          _eor = _endOfLine;
          switch (_status.Eor)
          {
            case U_LEFT_TO_RIGHT:
            case U_RIGHT_TO_LEFT:
            case U_ARABIC_NUMBER: _direction = _status.Eor; break;
            case U_EUROPEAN_NUMBER:
              _direction = _status.LastStrong == U_LEFT_TO_RIGHT ? U_LEFT_TO_RIGHT : U_EUROPEAN_NUMBER;
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
        _direction = U_OTHER_NEUTRAL;
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
          _direction = U_OTHER_NEUTRAL;
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
