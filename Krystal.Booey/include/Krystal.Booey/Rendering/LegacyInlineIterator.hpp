#pragma once

#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Rendering/BidiRun.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <limits>

namespace krys::boo
{
  class LegacyInlineIterator;

  struct BidiIsolatedRun
  {
    RenderObject &Object;
    RenderElement &Root;
    BidiRun &RunToReplace;
    uint32 Position;

    BidiIsolatedRun(RenderObject &object, uint32 position, RenderElement &root,
                    BidiRun &runToReplace) noexcept
        : Object(object), Root(root), RunToReplace(runToReplace), Position(position)
    {
    }
  };

  using InlineBidiResolver = BidiResolverWithIsolate<LegacyInlineIterator, BidiRun, BidiIsolatedRun>;
  using LineWhitespaceCollapsingState = WhitespaceCollapsingState<LegacyInlineIterator>;

  /// @brief Walks RenderInline subtrees, stepping by character within the text children. LegacyInlineIterator
  /// will use next to find the next RenderText optionally notifying a BidiResolver every time it steps
  /// into/out of a RenderInline.
  /// @note Offsets are UTF-8 byte offsets, and stepping within text moves by code point.
  class LegacyInlineIterator
  {
  private:
    RenderElement *_root {nullptr};
    RenderObject *_renderer {nullptr};

    Maybe<uint32> _nextBreakablePosition;
    uint32 _pos {0};

    // There are a couple places where we want to decrement an LegacyInlineIterator.
    // Usually this take the form of decrementing _pos; however, _pos might be 0.
    // However, we shouldn't ever need to decrement an LegacyInlineIterator more than
    // once, so rather than implementing a decrement() function which traverses
    // nodes, we can simply keep track of this state and handle it.
    bool _refersToEndOfPreviousNode {false};

  public:
    LegacyInlineIterator() noexcept = default;

    LegacyInlineIterator(RenderElement *root, RenderObject *o, uint32 p) noexcept
        : _root(root), _renderer(o), _pos(p)
    {
    }

    void Clear() noexcept
    {
      SetRenderer(nullptr);
      SetOffset(0);
      SetNextBreakablePosition(std::numeric_limits<uint32>::max());
    }
    void MoveToStartOf(RenderObject &object) noexcept
    {
      MoveTo(object, 0);
    }

    void MoveTo(RenderObject &object, uint32 offset, Maybe<uint32> nextBreak = null) noexcept
    {
      SetRenderer(&object);
      SetOffset(offset);
      SetNextBreakablePosition(nextBreak);
    }

    KRYS_NODISCARD RenderObject *Renderer() const noexcept
    {
      return _renderer;
    }
    void SetRenderer(RenderObject *renderer) noexcept
    {
      _renderer = renderer;
    }
    KRYS_NODISCARD uint32 Offset() const noexcept
    {
      return _pos;
    }
    void SetOffset(uint32 position) noexcept
    {
      krys_debug_assert(position <= std::numeric_limits<uint32>::max() - 10); // Sanity check
      _pos = position;
    }
    KRYS_NODISCARD RenderElement *Root() const noexcept
    {
      return _root;
    }
    KRYS_NODISCARD Maybe<uint32> NextBreakablePosition() const noexcept
    {
      return _nextBreakablePosition;
    }
    void SetNextBreakablePosition(Maybe<uint32> position) noexcept
    {
      _nextBreakablePosition = position;
    }
    KRYS_NODISCARD bool RefersToEndOfPreviousNode() const noexcept
    {
      return _refersToEndOfPreviousNode;
    }
    void SetRefersToEndOfPreviousNode() noexcept
    {
      krys_debug_assert(!_pos);
      krys_debug_assert(!_refersToEndOfPreviousNode);
      _refersToEndOfPreviousNode = true;
    }

    /// @brief Moves to the start of the next code point.
    void FastIncrementInTextNode() noexcept
    {
      krys_debug_assert(_renderer);
      const auto &text = Downcast<RenderText>(*_renderer).Text();
      krys_debug_assert(_pos <= text.length());
      ++_pos;
      while (_pos < text.length() && text::IsTrailingUTF8(text[_pos]))
      {
        ++_pos;
      }
    }
    void IncrementByCodePointInTextNode() noexcept
    {
      krys_debug_assert(_renderer);
      krys_debug_assert(_pos < Downcast<RenderText>(*_renderer).Text().length());
      FastIncrementInTextNode();
    }
    void Increment(InlineBidiResolver * = nullptr) noexcept;
    /// @brief Moves back one byte: a whitespace collapsing transition that starts ignoring spaces at this
    /// offset ends its run just past it.
    void FastDecrement() noexcept
    {
      krys_debug_assert(!RefersToEndOfPreviousNode());
      if (_pos)
      {
        SetOffset(_pos - 1);
      }
      else
      {
        SetRefersToEndOfPreviousNode();
      }
    }
    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_renderer;
    }

    /// @brief The offset just past the code point at this offset, which ends a bidi run.
    KRYS_NODISCARD uint32 NextOffset() const noexcept
    {
      auto *textRenderer = DynamicDowncast<RenderText>(_renderer);
      if (!textRenderer)
      {
        return _pos + 1;
      }
      const auto &text = textRenderer->Text();
      auto next = _pos + 1;
      while (next < text.length() && text::IsTrailingUTF8(text[next]))
      {
        ++next;
      }
      return next;
    }

    KRYS_NODISCARD bool AtTextParagraphSeparator() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool AtParagraphSeparator() const noexcept
    {
      return (_renderer && _renderer->IsBR()) || AtTextParagraphSeparator();
    }

    KRYS_NODISCARD char32 Current() const noexcept
    {
      return CharacterAt(_pos);
    }
    KRYS_NODISCARD char32 PreviousInSameNode() const noexcept
    {
      auto *textRenderer = DynamicDowncast<RenderText>(_renderer);
      if (!textRenderer || !_pos || _pos > textRenderer->Text().length())
      {
        return 0;
      }
      return CharacterAt(static_cast<uint32>(text::PreviousCodePointStart(textRenderer->Text(), _pos)));
    }
    KRYS_NODISCARD UCharDirection Direction() const noexcept
    {
      if (!_renderer) KRYS_UNLIKELY
      {
        return U_OTHER_NEUTRAL;
      }

      if (auto *textRenderer = DynamicDowncast<RenderText>(*_renderer)) KRYS_LIKELY
      {
        return u_charDirection(static_cast<UChar32>(CharacterAt(_pos)));
      }

      if (auto *listMarkerRenderer = DynamicDowncast<RenderListMarker>(*_renderer))
      {
        return listMarkerRenderer->GetWritingMode().IsBidiLTR() ? U_LEFT_TO_RIGHT : U_RIGHT_TO_LEFT;
      }

      return U_OTHER_NEUTRAL;
    }

    KRYS_NODISCARD friend bool operator==(const LegacyInlineIterator &it1,
                                          const LegacyInlineIterator &it2) noexcept
    {
      return it1.Offset() == it2.Offset() && it1.Renderer() == it2.Renderer();
    }

  private:
    KRYS_NODISCARD char32 CharacterAt(uint32 index) const noexcept
    {
      auto *textRenderer = DynamicDowncast<RenderText>(_renderer);
      if (!textRenderer || index >= textRenderer->Text().length())
      {
        return 0;
      }
      return text::CodePointAt(textRenderer->Text(), index);
    }
  };

  template <>
  inline void InlineBidiResolver::IncrementInternal() noexcept
  {
    _current.Increment(this);
  }

  template <>
  void InlineBidiResolver::AppendRunInternal() noexcept;

  template <>
  inline bool InlineBidiResolver::NeedsContinuePastEndInternal() const noexcept
  {
    // We don't collect runs beyond the endOfLine renderer. Stop traversing when the iterator moves to the
    // next renderer to prevent O(n^2).
    return _current.Renderer() == _endOfLine.Renderer();
  }

  KRYS_NODISCARD inline UCharDirection EmbedCharFromDirection(WritingMode writingMode,
                                                              UnicodeBidi unicodeBidi) noexcept
  {
    if (unicodeBidi == UnicodeBidi::Embed)
    {
      return writingMode.IsBidiRTL() ? U_RIGHT_TO_LEFT_EMBEDDING : U_LEFT_TO_RIGHT_EMBEDDING;
    }
    return writingMode.IsBidiRTL() ? U_RIGHT_TO_LEFT_OVERRIDE : U_LEFT_TO_RIGHT_OVERRIDE;
  }

  template <class Observer>
  inline void NotifyObserverEnteredObject(Observer *observer, RenderObject *object) noexcept
  {
    if (!observer || !object || !object->IsRenderInline())
    {
      return;
    }

    auto &style = Downcast<RenderInline>(*object).Style();
    auto unicodeBidi = style.GetUnicodeBidi();
    if (unicodeBidi == UnicodeBidi::Normal)
    {
      // http://dev.w3.org/csswg/css3-writing-modes/#unicode-bidi
      // "The element does not open an additional level of embedding with respect to the bidirectional
      // algorithm." Thus we ignore any possible dir= attribute on the span.
      return;
    }
    if (IsIsolated(unicodeBidi))
    {
      // Make sure that explicit embeddings are committed before we enter the isolated content.
      observer->CommitExplicitEmbedding();
      observer->EnterIsolate();
      // Embedding/Override characters implied by dir= will be handled when
      // we process the isolated span, not when laying out the "parent" run.
      return;
    }

    if (!observer->InIsolate())
    {
      observer->Embed(EmbedCharFromDirection(style.GetWritingMode(), unicodeBidi),
                      BidiEmbeddingSource::FromStyleOrDOM);
    }
  }

  template <class Observer>
  inline void NotifyObserverWillExitObject(Observer *observer, RenderObject *object) noexcept
  {
    if (!observer || !object || !object->IsRenderInline())
    {
      return;
    }

    auto unicodeBidi = Downcast<RenderInline>(*object).Style().GetUnicodeBidi();
    if (unicodeBidi == UnicodeBidi::Normal)
    {
      return; // Nothing to do for unicode-bidi: normal
    }
    if (IsIsolated(unicodeBidi))
    {
      observer->ExitIsolate();
      return;
    }

    // Otherwise we pop any embed/override character we added when we opened this tag.
    if (!observer->InIsolate())
    {
      observer->Embed(U_POP_DIRECTIONAL_FORMAT, BidiEmbeddingSource::FromStyleOrDOM);
    }
  }

  KRYS_NODISCARD inline bool IsIteratorTarget(RenderObject *object) noexcept
  {
    krys_debug_assert(object); // The iterator will of course return 0, but its not an expected argument to
                               // this function.
    return object->IsRenderTextOrLineBreak() || object->IsFloating() || object->IsOutOfFlowPositioned()
           || object->IsBlockLevelReplacedOrAtomicInline();
  }

  template <class Observer>
  KRYS_NODISCARD inline RenderObject *
    NextInlineRendererSkippingEmpty(RenderElement &root, RenderObject *current, Observer *observer) noexcept
  {
    RenderObject *next = nullptr;

    while (current)
    {
      next = nullptr;
      if (!IsIteratorTarget(current))
      {
        next = Downcast<RenderElement>(*current).FirstChild();
        NotifyObserverEnteredObject(observer, next);
      }

      if (!next)
      {
        while (current && current != &root)
        {
          NotifyObserverWillExitObject(observer, current);

          next = current->NextSibling();
          if (next)
          {
            NotifyObserverEnteredObject(observer, next);
            break;
          }

          current = current->Parent();
        }
      }

      if (!next)
      {
        break;
      }

      if (IsIteratorTarget(next))
      {
        break;
      }

      auto *renderInline = DynamicDowncast<RenderInline>(*next);
      if (renderInline && IsEmptyInline(*renderInline))
      {
        break;
      }

      current = next;
    }

    return next;
  }

  /// @brief This makes callers cleaner as they don't have to specify a type for the observer when not
  /// providing one.
  KRYS_NODISCARD inline RenderObject *NextInlineRendererSkippingEmpty(RenderElement &root,
                                                                      RenderObject *current) noexcept
  {
    InlineBidiResolver *observer = nullptr;
    return NextInlineRendererSkippingEmpty(root, current, observer);
  }

  KRYS_NODISCARD RenderObject *
    FirstInlineRendererSkippingEmpty(RenderElement &root, InlineBidiResolver *resolver = nullptr) noexcept;

  inline void LegacyInlineIterator::Increment(InlineBidiResolver *resolver) noexcept
  {
    if (!_renderer)
    {
      return;
    }
    if (auto *textRenderer = DynamicDowncast<RenderText>(*_renderer))
    {
      FastIncrementInTextNode();
      if (_pos < textRenderer->Text().length())
      {
        return;
      }
    }
    // next can return nullptr
    RenderObject *next = NextInlineRendererSkippingEmpty(*_root, _renderer, resolver);
    if (next)
    {
      MoveToStartOf(*next);
    }
    else
    {
      Clear();
    }
  }

  KRYS_NODISCARD inline bool IsIsolatedInline(RenderObject &object) noexcept
  {
    if (auto *inlineBox = DynamicDowncast<RenderInline>(object))
    {
      return IsIsolated(inlineBox->Style().GetUnicodeBidi());
    }
    return false;
  }

  KRYS_NODISCARD inline RenderObject *HighestContainingIsolateWithinRoot(RenderObject &initialObject,
                                                                         RenderObject *root) noexcept
  {
    RenderObject *containingIsolateObject = nullptr;
    for (RenderObject *object = &initialObject; object && object != root; object = object->Parent())
    {
      if (IsIsolatedInline(*object))
      {
        containingIsolateObject = object;
      }
    }
    return containingIsolateObject;
  }

  KRYS_NODISCARD inline uint32 NumberOfIsolateAncestors(const LegacyInlineIterator &iter) noexcept
  {
    uint32 count = 0;
    RenderObject *root = iter.Root();
    for (RenderObject *object = iter.Renderer(); object && object != root; object = object->Parent())
    {
      if (IsIsolatedInline(*object))
      {
        count++;
      }
    }
    return count;
  }

  // FIXME: This belongs on InlineBidiResolver, except it's a template specialization
  // of BidiResolver which knows nothing about RenderObjects.
  inline void AddPlaceholderRunForIsolatedInline(InlineBidiResolver &resolver, RenderObject &obj, uint32 pos,
                                                 RenderElement &root) noexcept
  {
    auto isolatedRun = CreateUnique<BidiRun>(pos, pos, obj, resolver.Context(), resolver.Dir());
    // FIXME: isolatedRuns() could be a hash of object->run and then we could cheaply
    // ASSERT here that we didn't create multiple objects for the same inline.
    resolver.SetWhitespaceCollapsingTransitionForIsolatedRun(
      *isolatedRun, resolver.GetWhitespaceCollapsingState().CurrentTransition());
    resolver.IsolatedRuns().push_back(BidiIsolatedRun(obj, pos, root, *isolatedRun));
    resolver.Runs().AppendRun(krys::move(isolatedRun));
  }

  class IsolateTracker
  {
  private:
    uint32 _nestedIsolateCount;
    bool _haveAddedFakeRunForRootIsolate {false};

  public:
    explicit IsolateTracker(uint32 nestedIsolateCount) noexcept : _nestedIsolateCount(nestedIsolateCount)
    {
    }

    void EnterIsolate() noexcept
    {
      _nestedIsolateCount++;
    }
    void ExitIsolate() noexcept
    {
      krys_debug_assert(_nestedIsolateCount >= 1);
      _nestedIsolateCount--;
      if (!InIsolate())
      {
        _haveAddedFakeRunForRootIsolate = false;
      }
    }
    KRYS_NODISCARD bool InIsolate() const noexcept
    {
      return _nestedIsolateCount;
    }

    // We don't care if we encounter bidi directional overrides.
    void Embed(UCharDirection, BidiEmbeddingSource) noexcept
    {
    }
    void CommitExplicitEmbedding() noexcept
    {
    }

    void AddFakeRunIfNecessary(RenderObject &, uint32 position, uint32 end, RenderElement &root,
                               InlineBidiResolver &) noexcept;
  };
}
