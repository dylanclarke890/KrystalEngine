#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo
{
  template <typename T>
  class RectEdges
  {
  private:
    Array<T, 4> _sides;

  public:
    RectEdges() noexcept
    requires(DefaultConstructible<T>)
        : _sides {}
    {
    }

    RectEdges(const T &value) noexcept : _sides {value, value, value, value}
    {
    }

    template <typename U>
    RectEdges(U &&top, U &&right, U &&bottom, U &&left) noexcept
        : _sides(
            {{std::forward<U>(top), std::forward<U>(right), std::forward<U>(bottom), std::forward<U>(left)}})
    {
    }

    RectEdges(const RectEdges &) noexcept = default;
    RectEdges &operator=(const RectEdges &) noexcept = default;

    template <typename U>
    RectEdges(const RectEdges<U> &other) noexcept
        : RectEdges(other.Top(), other.Right(), other.Bottom(), other.Left())
    {
    }

    template <typename U, typename Mapper>
    KRYS_NODISCARD static RectEdges<T> Map(RectEdges<U> &&other, Mapper &&mapper) noexcept
    {
      return RectEdges<T> {
        mapper(other.Top()),
        mapper(other.Right()),
        mapper(other.Bottom()),
        mapper(other.Left()),
      };
    }

    KRYS_NODISCARD T &At(BoxSide side) noexcept
    {
      return _sides[static_cast<size_t>(side)];
    }

    KRYS_NODISCARD T &operator[](BoxSide side) noexcept
    {
      return _sides[static_cast<size_t>(side)];
    }

    KRYS_NODISCARD T &Top() noexcept
    {
      return At(BoxSide::Top);
    }

    KRYS_NODISCARD T &Right() noexcept
    {
      return At(BoxSide::Right);
    }

    KRYS_NODISCARD T &Bottom() noexcept
    {
      return At(BoxSide::Bottom);
    }

    KRYS_NODISCARD T &Left() noexcept
    {
      return At(BoxSide::Left);
    }

    KRYS_NODISCARD const T &At(BoxSide side) const noexcept
    {
      return _sides[static_cast<size_t>(side)];
    }

    KRYS_NODISCARD const T &operator[](BoxSide side) const noexcept
    {
      return _sides[static_cast<size_t>(side)];
    }

    KRYS_NODISCARD const T &Top() const noexcept
    {
      return At(BoxSide::Top);
    }

    KRYS_NODISCARD const T &Right() const noexcept
    {
      return At(BoxSide::Right);
    }

    KRYS_NODISCARD const T &Bottom() const noexcept
    {
      return At(BoxSide::Bottom);
    }

    KRYS_NODISCARD const T &Left() const noexcept
    {
      return At(BoxSide::Left);
    }

    void SetAt(BoxSide side, const T &v) noexcept
    {
      At(side) = v;
    }

    void SetTop(const T &top) noexcept
    {
      SetAt(BoxSide::Top, top);
    }

    void SetRight(const T &right) noexcept
    {
      SetAt(BoxSide::Right, right);
    }

    void SetBottom(const T &bottom) noexcept
    {
      SetAt(BoxSide::Bottom, bottom);
    }

    void SetLeft(const T &left) noexcept
    {
      SetAt(BoxSide::Left, left);
    }

    KRYS_NODISCARD RectEdges<T> XFlippedCopy() const noexcept
    {
      RectEdges<T> copy {*this};
      copy.Left() = Right();
      copy.Right() = Left();

      return copy;
    }

    KRYS_NODISCARD RectEdges<T> YFlippedCopy() const noexcept
    {
      RectEdges<T> copy {*this};
      copy.Top() = Bottom();
      copy.Bottom() = Top();

      return copy;
    }

    KRYS_NODISCARD T &Before(WritingMode writingMode) noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::BlockStart));
    }

    KRYS_NODISCARD T &After(WritingMode writingMode) noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::BlockEnd));
    }

    KRYS_NODISCARD T &Start(WritingMode writingMode) noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::InlineStart));
    }

    KRYS_NODISCARD T &End(WritingMode writingMode) noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::InlineEnd));
    }

    KRYS_NODISCARD T &LogicalLeft(WritingMode writingMode) noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::LogicalLeft));
    }

    KRYS_NODISCARD T &LogicalRight(WritingMode writingMode) noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::LogicalRight));
    }

    KRYS_NODISCARD const T &Before(WritingMode writingMode) const noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::BlockStart));
    }

    KRYS_NODISCARD const T &After(WritingMode writingMode) const noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::BlockEnd));
    }

    KRYS_NODISCARD const T &Start(WritingMode writingMode) const noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::InlineStart));
    }

    KRYS_NODISCARD const T &End(WritingMode writingMode) const noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::InlineEnd));
    }

    KRYS_NODISCARD const T &LogicalLeft(WritingMode writingMode) const noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::LogicalLeft));
    }

    KRYS_NODISCARD const T &LogicalRight(WritingMode writingMode) const noexcept
    {
      return At(MapSideLogicalToPhysical(writingMode, LogicalBoxSide::LogicalRight));
    }

    void SetBefore(const T &before, WritingMode writingMode) noexcept
    {
      this->Before(writingMode) = before;
    }

    void SetAfter(const T &after, WritingMode writingMode) noexcept
    {
      this->After(writingMode) = after;
    }

    void SetStart(const T &start, WritingMode writingMode) noexcept
    {
      this->Start(writingMode) = start;
    }

    void SetEnd(const T &end, WritingMode writingMode) noexcept
    {
      this->End(writingMode) = end;
    }

    void SetLogicalLeft(const T &logicalLeft, WritingMode writingMode) noexcept
    {
      this->LogicalLeft(writingMode) = logicalLeft;
    }

    void SetLogicalRight(const T &logicalRight, WritingMode writingMode) noexcept
    {
      this->LogicalRight(writingMode) = logicalRight;
    }

    KRYS_NODISCARD RectEdges<T> BlockFlippedCopy(WritingMode writingMode) const noexcept
    {
      if (writingMode.IsHorizontal())
      {
        return YFlippedCopy();
      }

      return XFlippedCopy();
    }

    KRYS_NODISCARD RectEdges<T> InlineFlippedCopy(WritingMode writingMode) const noexcept
    {
      if (writingMode.IsHorizontal())
      {
        return XFlippedCopy();
      }

      return YFlippedCopy();
    }

    template <typename F>
    KRYS_NODISCARD bool AnyOf(F &&functor) const noexcept
    {
      return std::ranges::any_of(_sides, std::forward<F>(functor));
    }

    template <typename F>
    KRYS_NODISCARD bool AllOf(F &&functor) const noexcept
    {
      return std::ranges::all_of(_sides, std::forward<F>(functor));
    }

    template <typename F>
    KRYS_NODISCARD bool NoneOf(F &&functor) const noexcept
    {
      return std::ranges::none_of(_sides, std::forward<F>(functor));
    }

    KRYS_NODISCARD bool IsZero() const noexcept
    requires(requires {
      { !std::declval<T>() } -> std::same_as<bool>;
    })
    {
      return AllOf([](auto &edge) { return !edge; });
    }

    bool operator==(const RectEdges<T> &) const noexcept = default;
  };

  template <typename T>
  KRYS_NODISCARD constexpr RectEdges<T> operator+(const RectEdges<T> &a, const RectEdges<T> &b) noexcept
  {
    return {a.Top() + b.Top(), a.Right() + b.Right(), a.Bottom() + b.Bottom(), a.Left() + b.Left()};
  }

  template <typename T>
  RectEdges<T> &operator+=(RectEdges<T> &a, const RectEdges<T> &b) noexcept
  {
    a = a + b;
    return a;
  }

  template <typename T>
  KRYS_NODISCARD constexpr RectEdges<T> operator-(const RectEdges<T> &a, const RectEdges<T> &b) noexcept
  {
    return {a.Top() - b.Top(), a.Right() - b.Right(), a.Bottom() - b.Bottom(), a.Left() - b.Left()};
  }

  template <typename T>
  RectEdges<T> &operator-=(RectEdges<T> &a, const RectEdges<T> &b) noexcept
  {
    a = a - b;
    return a;
  }

  template <typename T, typename F>
  KRYS_NODISCARD RectEdges<T> blend(const RectEdges<T> &a, const RectEdges<T> &b, F &&functor) noexcept
  {
    return {functor(a.Top(), b.Top(), BoxSide::Top), functor(a.Right(), b.Right(), BoxSide::Right),
            functor(a.Bottom(), b.Bottom(), BoxSide::Bottom), functor(a.Left(), b.Left(), BoxSide::Left)};
  }

  template <typename T>
  KRYS_NODISCARD RectEdges<T> max(const RectEdges<T> &a, const RectEdges<T> &b) noexcept
  {
    return blend(a, b, [](const T &a, const T &b, BoxSide) { return std::max(a, b); });
  }
}
