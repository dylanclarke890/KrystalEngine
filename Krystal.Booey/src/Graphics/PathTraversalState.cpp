#include "Krystal.Booey/Graphics/PathTraversalState.hpp"

#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  static const float kPathSegmentLengthTolerance = 0.00001f;

  KRYS_NODISCARD static inline float DistanceLine(const FloatPoint &start, const FloatPoint &end) noexcept
  {
    return std::hypot(end.X() - start.X(), end.Y() - start.Y());
  }

  struct QuadraticBezier
  {
    QuadraticBezier() noexcept = default;
    QuadraticBezier(const FloatPoint &s, const FloatPoint &c, const FloatPoint &e) noexcept
        : Start(s), Control(c), End(e)
    {
    }

    KRYS_NODISCARD bool operator==(const QuadraticBezier &) const noexcept = default;

    KRYS_NODISCARD float ApproximateDistance() const noexcept
    {
      return DistanceLine(Start, Control) + DistanceLine(Control, End);
    }

    KRYS_NODISCARD bool Split(QuadraticBezier &left, QuadraticBezier &right) const noexcept
    {
      left.Control = MidPoint(Start, Control);
      right.Control = MidPoint(Control, End);

      FloatPoint leftControlToRightControl = MidPoint(left.Control, right.Control);
      left.End = leftControlToRightControl;
      right.Start = leftControlToRightControl;

      left.Start = Start;
      right.End = End;

      return !(left == *this) && !(right == *this);
    }

    FloatPoint Start;
    FloatPoint Control;
    FloatPoint End;
  };

  struct CubicBezier
  {
    CubicBezier() noexcept = default;
    CubicBezier(const FloatPoint &s, const FloatPoint &c1, const FloatPoint &c2, const FloatPoint &e) noexcept
        : Start(s), Control1(c1), Control2(c2), End(e)
    {
    }

    KRYS_NODISCARD bool operator==(const CubicBezier &) const noexcept = default;

    KRYS_NODISCARD float ApproximateDistance() const noexcept
    {
      return DistanceLine(Start, Control1) + DistanceLine(Control1, Control2) + DistanceLine(Control2, End);
    }

    KRYS_NODISCARD bool Split(CubicBezier &left, CubicBezier &right) const noexcept
    {
      FloatPoint startToControl1 = MidPoint(Control1, Control2);

      left.Start = Start;
      left.Control1 = MidPoint(Start, Control1);
      left.Control2 = MidPoint(left.Control1, startToControl1);

      right.Control2 = MidPoint(Control2, End);
      right.Control1 = MidPoint(right.Control2, startToControl1);
      right.End = End;

      FloatPoint leftControl2ToRightControl1 = MidPoint(left.Control2, right.Control1);
      left.End = leftControl2ToRightControl1;
      right.Start = leftControl2ToRightControl1;

      return !(left == *this) && !(right == *this);
    }

    FloatPoint Start;
    FloatPoint Control1;
    FloatPoint Control2;
    FloatPoint End;
  };

  // FIXME: This function is possibly very slow due to the ifs required for proper path measuring
  // A simple speed-up would be to use an additional boolean template parameter to control whether
  // to use the "fast" version of this function with no PathTraversalState updating, vs. the slow
  // version which does update the PathTraversalState.  We'll have to shark it to see if that's necessary.
  // Another check which is possible up-front (to send us down the fast path) would be to check if
  // approximateDistance() + current total distance > desired distance
  template <class CurveType>
  KRYS_NODISCARD static float CurveLength(const PathTraversalState &traversalState,
                                          const CurveType &originalCurve, FloatPoint &previous,
                                          FloatPoint &current) noexcept
  {
    static const unsigned curveStackDepthLimit = 20;
    CurveType curve = originalCurve;
    SmallList<CurveType, curveStackDepthLimit> curveStack;
    float totalLength = 0;

    while (true)
    {
      float length = curve.ApproximateDistance();

      CurveType leftCurve;
      CurveType rightCurve;

      if ((length - DistanceLine(curve.Start, curve.End)) > kPathSegmentLengthTolerance
          && curveStack.size() < curveStackDepthLimit && curve.Split(leftCurve, rightCurve))
      {
        curve = leftCurve;
        curveStack.push_back(rightCurve);
        continue;
      }

      totalLength += length;
      if (traversalState.GetAction() == PathTraversalState::Action::VectorAtLength)
      {
        previous = curve.Start;
        current = curve.End;
        if (traversalState.TotalLength() + totalLength > traversalState.DesiredLength())
        {
          break;
        }
      }

      if (curveStack.empty())
      {
        break;
      }

      curve = curveStack.back();
      curveStack.pop_back();
    }

    if (traversalState.GetAction() != PathTraversalState::Action::VectorAtLength)
    {
      krys_debug_assert(curve.End == originalCurve.End);
      previous = curve.Start;
      current = curve.End;
    }

    return totalLength;
  }

  PathTraversalState::PathTraversalState(Action action, float desiredLength) noexcept
      : _action(action), _desiredLength(desiredLength)
  {
    krys_debug_assert(action != Action::TotalLength || !desiredLength);
  }

  void PathTraversalState::CloseSubpath() noexcept
  {
    _totalLength += DistanceLine(_current, _start);
    _current = _start;
  }

  void PathTraversalState::MoveTo(const FloatPoint &point) noexcept
  {
    _previous = _current = _start = point;
  }

  void PathTraversalState::LineTo(const FloatPoint &point) noexcept
  {
    _totalLength += DistanceLine(_current, point);
    _current = point;
  }

  void PathTraversalState::QuadraticBezierTo(const FloatPoint &newControl, const FloatPoint &newEnd) noexcept
  {
    _totalLength +=
      CurveLength<QuadraticBezier>(*this, QuadraticBezier(_current, newControl, newEnd), _previous, _current);
  }

  void PathTraversalState::CubicBezierTo(const FloatPoint &newControl1, const FloatPoint &newControl2,
                                         const FloatPoint &newEnd) noexcept
  {
    _totalLength += CurveLength<CubicBezier>(*this, CubicBezier(_current, newControl1, newControl2, newEnd),
                                             _previous, _current);
  }

  bool PathTraversalState::FinalizeAppendPathElement() noexcept
  {
    if (_action == Action::TotalLength)
    {
      return false;
    }

    if (_action == Action::SegmentAtLength)
    {
      if (_totalLength >= _desiredLength)
      {
        _success = true;
      }
      return _success;
    }

    krys_debug_assert(_action == Action::VectorAtLength);

    if (_totalLength >= _desiredLength)
    {
      float slope = FloatPoint(_current - _previous).SlopeAngleRadians();
      float offset = _desiredLength - _totalLength;
      _current.Move(offset * std::cos(slope), offset * std::sin(slope));

      if (!_isZeroVector && !_desiredLength)
      {
        _isZeroVector = true;
      }
      else
      {
        _success = true;
        _normalAngle = rad2deg(slope);
      }
    }

    _previous = _current;
    return _success;
  }

  bool PathTraversalState::AppendPathElement(PathElement::Type type,
                                             std::span<const FloatPoint> points) noexcept
  {
    switch (type)
    {
      case PathElement::Type::MoveToPoint:         MoveTo(points[0]); break;
      case PathElement::Type::AddLineToPoint:      LineTo(points[0]); break;
      case PathElement::Type::AddQuadCurveToPoint: QuadraticBezierTo(points[0], points[1]); break;
      case PathElement::Type::AddCurveToPoint:     CubicBezierTo(points[0], points[1], points[2]); break;
      case PathElement::Type::CloseSubpath:        CloseSubpath(); break;
    }

    return FinalizeAppendPathElement();
  }

  bool PathTraversalState::ProcessPathElement(PathElement::Type type,
                                              std::span<const FloatPoint> points) noexcept
  {
    if (_success)
    {
      return true;
    }

    if (_isZeroVector)
    {
      PathTraversalState traversalState(*this);
      _success = traversalState.AppendPathElement(type, points);
      _normalAngle = traversalState._normalAngle;
      return _success;
    }

    return AppendPathElement(type, points);
  }
}
