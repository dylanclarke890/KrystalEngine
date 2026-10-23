#pragma once

#include "Krystal.Booey/DOM/AbstractRange.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief A range in a highlight, with the positions its painting starts and ends at (worked out at each
  /// rendering update).
  class HighlightRange : public RefCounted<HighlightRange>, public CanMakeWeakPtr<HighlightRange>
  {
  private:
    const Ref<AbstractRange> _range;
    Position _startPosition;
    Position _endPosition;

    explicit HighlightRange(Ref<AbstractRange> &&range) noexcept : _range(krys::move(range))
    {
    }

  public:
    KRYS_NODISCARD static Ref<HighlightRange> Create(Ref<AbstractRange> &&range) noexcept
    {
      return AdoptRef(*new HighlightRange(krys::move(range)));
    }

    KRYS_NODISCARD AbstractRange &Range() const noexcept
    {
      return *_range;
    }

    KRYS_NODISCARD const Position &StartPosition() const noexcept
    {
      return _startPosition;
    }
    void SetStartPosition(Position &&startPosition) noexcept
    {
      _startPosition = krys::move(startPosition);
    }

    KRYS_NODISCARD const Position &EndPosition() const noexcept
    {
      return _endPosition;
    }
    void SetEndPosition(Position &&endPosition) noexcept
    {
      _endPosition = krys::move(endPosition);
    }
  };

  /// @brief A set of ranges painted in their own way: a custom highlight, named in a `HighlightRegistry` and
  /// styled by `::highlight(name)`.
  /// @see https://drafts.csswg.org/css-highlight-api-1/#highlight
  class Highlight : public RefCounted<Highlight>
  {
  public:
    /// @see https://drafts.csswg.org/css-highlight-api-1/#enumdef-highlighttype
    enum class Type : uint8
    {
      Highlight,
      SpellingError,
      GrammarError
    };

  private:
    List<Ref<HighlightRange>> _highlightRanges;
    Type _type {Type::Highlight};
    int32 _priority {0};

    explicit Highlight(List<Ref<AbstractRange>> &&initialRanges) noexcept;

  public:
    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-highlight-highlight
    KRYS_NODISCARD static Ref<Highlight> Create(List<Ref<AbstractRange>> &&initialRanges = {}) noexcept;

    /// @brief Repaints the renderers of the nodes @p range intersects.
    static void RepaintRange(const AbstractRange &range) noexcept;

    /// @brief The number of ranges (the setlike's `size`).
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _highlightRanges.size();
    }

    /// @brief Whether @p range is one of the highlight's ranges.
    KRYS_NODISCARD bool Has(const AbstractRange &range) const noexcept;

    /// @brief Adds @p range, or moves it to the end if it's there already.
    /// @return Whether it was added.
    bool Add(AbstractRange &range) noexcept;

    /// @brief Removes @p range.
    /// @return Whether it was there.
    bool Delete(const AbstractRange &range) noexcept;

    /// @brief Removes every range.
    void Clear() noexcept;

    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-highlight-type
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }
    void SetType(Type type) noexcept
    {
      _type = type;
    }

    /// @brief Which of overlapping highlights is painted on top: the higher priority.
    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-highlight-priority
    KRYS_NODISCARD int32 Priority() const noexcept
    {
      return _priority;
    }
    void SetPriority(int32 priority) noexcept;

    void Repaint() noexcept;

    KRYS_NODISCARD const List<Ref<HighlightRange>> &HighlightRanges() const noexcept
    {
      return _highlightRanges;
    }
  };
}
