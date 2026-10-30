#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo
{
  template <typename T>
  class RectCorners
  {
  private:
    Array<T, 4> _corners;

  public:
    RectCorners() noexcept
    requires(DefaultConstructible<T>)
        : _corners {}
    {
    }

    RectCorners(const T &value) noexcept : _corners {value, value, value, value}
    {
    }

    template <typename U>
    RectCorners(U &&topLeft, U &&topRight, U &&bottomLeft, U &&bottomRight) noexcept
        : _corners({{std::forward<U>(topLeft), std::forward<U>(topRight), std::forward<U>(bottomLeft),
                     std::forward<U>(bottomRight)}})
    {
    }

    RectCorners(const RectCorners &) noexcept = default;
    RectCorners &operator=(const RectCorners &) noexcept = default;

    template <typename U>
    RectCorners(const RectCorners<U> &other) noexcept
        : RectCorners(other.TopLeft(), other.TopRight(), other.BottomLeft(), other.BottomRight())
    {
    }

    KRYS_NODISCARD T &At(BoxCorner corner) noexcept
    {
      return _corners[static_cast<size_t>(corner)];
    }

    KRYS_NODISCARD T &operator[](BoxCorner corner) noexcept
    {
      return _corners[static_cast<size_t>(corner)];
    }

    KRYS_NODISCARD T &TopLeft() noexcept
    {
      return At(BoxCorner::TopLeft);
    }

    KRYS_NODISCARD T &TopRight() noexcept
    {
      return At(BoxCorner::TopRight);
    }

    KRYS_NODISCARD T &BottomLeft() noexcept
    {
      return At(BoxCorner::BottomLeft);
    }

    KRYS_NODISCARD T &BottomRight() noexcept
    {
      return At(BoxCorner::BottomRight);
    }

    KRYS_NODISCARD const T &At(BoxCorner corner) const noexcept
    {
      return _corners[static_cast<size_t>(corner)];
    }

    KRYS_NODISCARD const T &operator[](BoxCorner corner) const noexcept
    {
      return _corners[static_cast<size_t>(corner)];
    }

    KRYS_NODISCARD const T &TopLeft() const noexcept
    {
      return At(BoxCorner::TopLeft);
    }

    KRYS_NODISCARD const T &TopRight() const noexcept
    {
      return At(BoxCorner::TopRight);
    }

    KRYS_NODISCARD const T &BottomLeft() const noexcept
    {
      return At(BoxCorner::BottomLeft);
    }

    KRYS_NODISCARD const T &BottomRight() const noexcept
    {
      return At(BoxCorner::BottomRight);
    }

    void SetAt(BoxCorner corner, const T &v) noexcept
    {
      At(corner) = v;
    }

    void SetTopLeft(const T &topLeft) noexcept
    {
      SetAt(BoxCorner::TopLeft, topLeft);
    }

    void SetTopRight(const T &topRight) noexcept
    {
      SetAt(BoxCorner::TopRight, topRight);
    }

    void SetBottomLeft(const T &bottomLeft) noexcept
    {
      SetAt(BoxCorner::BottomLeft, bottomLeft);
    }

    void SetBottomRight(const T &bottomRight) noexcept
    {
      SetAt(BoxCorner::BottomRight, bottomRight);
    }

    template <typename F>
    KRYS_NODISCARD bool AnyOf(F &&functor) const noexcept
    {
      return std::ranges::any_of(_corners, std::forward<F>(functor));
    }

    template <typename F>
    KRYS_NODISCARD bool AllOf(F &&functor) const noexcept
    {
      return std::ranges::all_of(_corners, std::forward<F>(functor));
    }

    template <typename F>
    KRYS_NODISCARD bool NoneOf(F &&functor) const noexcept
    {
      return std::ranges::none_of(_corners, std::forward<F>(functor));
    }

    KRYS_NODISCARD bool IsZero() const noexcept
    {
      return AllOf([](auto &corner) { return !corner; });
    }

    bool operator==(const RectCorners<T> &) const noexcept = default;
  };
}
