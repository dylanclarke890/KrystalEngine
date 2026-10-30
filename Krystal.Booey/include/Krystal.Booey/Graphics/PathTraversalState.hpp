#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"

namespace krys::boo
{
  class PathTraversalState
  {
  public:
    enum class Action
    {
      TotalLength,
      VectorAtLength,
      SegmentAtLength,
    };

    PathTraversalState(Action, float desiredLength = 0) noexcept;

  public:
    KRYS_NODISCARD bool ProcessPathElement(PathElement::Type, std::span<const FloatPoint>) noexcept;
    KRYS_NODISCARD bool ProcessPathElement(const PathElement &element) noexcept
    {
      return ProcessPathElement(element.ElementType, element.Points);
    }

    KRYS_NODISCARD Action GetAction() const noexcept
    {
      return _action;
    }
    void SetAction(Action action) noexcept
    {
      _action = action;
    }
    KRYS_NODISCARD float DesiredLength() const noexcept
    {
      return _desiredLength;
    }
    void SetDesiredLength(float desiredLength) noexcept
    {
      _desiredLength = desiredLength;
    }

    // Traversing output -- should be read only
    KRYS_NODISCARD bool Success() const noexcept
    {
      return _success;
    }
    KRYS_NODISCARD float TotalLength() const noexcept
    {
      return _totalLength;
    }
    KRYS_NODISCARD FloatPoint Current() const noexcept
    {
      return _current;
    }
    KRYS_NODISCARD float NormalAngle() const noexcept
    {
      return _normalAngle;
    }

  private:
    void CloseSubpath() noexcept;
    void MoveTo(const FloatPoint &) noexcept;
    void LineTo(const FloatPoint &) noexcept;
    void QuadraticBezierTo(const FloatPoint &, const FloatPoint &) noexcept;
    void CubicBezierTo(const FloatPoint &, const FloatPoint &, const FloatPoint &) noexcept;

    KRYS_NODISCARD bool FinalizeAppendPathElement() noexcept;
    KRYS_NODISCARD bool AppendPathElement(PathElement::Type, std::span<const FloatPoint>) noexcept;

  private:
    Action _action;
    bool _success {false};

    FloatPoint _current;
    FloatPoint _start;

    float _totalLength {0};
    float _desiredLength {0};

    // For normal calculations
    FloatPoint _previous;
    float _normalAngle {0}; // degrees
    bool _isZeroVector {false};
  };
}
