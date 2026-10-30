#pragma once

#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/PathElement.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <array>
#include <cmath>

namespace krys::boo
{
  enum SVGMarkerType : uint8
  {
    StartMarker,
    MidMarker,
    EndMarker
  };

  /// @brief Where a marker is drawn on a path, and at what angle.
  struct MarkerPosition
  {
    MarkerPosition(SVGMarkerType useType, const FloatPoint &useOrigin, float useAngle) noexcept
        : Type(useType), Origin(useOrigin), Angle(useAngle)
    {
    }

    SVGMarkerType Type;
    FloatPoint Origin;
    float Angle;
  };

  /// @brief Collects the marker positions of a path, element by element.
  class SVGMarkerData
  {
  private:
    List<MarkerPosition> &_positions;
    uint32 _elementIndex {0};
    FloatPoint _origin;
    FloatPoint _subpathStart;
    std::array<FloatPoint, 2> _inslopePoints;
    std::array<FloatPoint, 2> _outslopePoints;
    bool _reverseStart;
    bool _previousWasMoveTo {false};

  public:
    SVGMarkerData(List<MarkerPosition> &positions, bool reverseStart) noexcept
        : _positions(positions), _reverseStart(reverseStart)
    {
    }

    static void UpdateFromPathElement(SVGMarkerData &markerData, const PathElement &element) noexcept
    {
      // First update the outslope for the previous element.
      if (element.ElementType != PathElement::Type::MoveToPoint)
      {
        markerData.UpdateOutslope(element.Points[0]);
      }

      // Record the marker for the previous element.
      if (markerData._elementIndex > 0)
      {
        SVGMarkerType markerType = markerData._elementIndex == 1 ? StartMarker : MidMarker;
        SVGMarkerType markerTypeForOrientation;
        if (markerData._previousWasMoveTo)
        {
          markerTypeForOrientation = StartMarker;
        }
        else if (element.ElementType == PathElement::Type::MoveToPoint)
        {
          markerTypeForOrientation = EndMarker;
        }
        else
        {
          markerTypeForOrientation = markerType;
        }
        markerData._positions.emplace_back(markerType, markerData._origin,
                                           markerData.CurrentAngle(markerTypeForOrientation));
      }

      // Update our marker data for this element.
      markerData.UpdateMarkerDataForPathElement(element);
      markerData._previousWasMoveTo = element.ElementType == PathElement::Type::MoveToPoint;
      ++markerData._elementIndex;
    }

    void PathIsDone() noexcept
    {
      _positions.emplace_back(EndMarker, _origin, CurrentAngle(EndMarker));
    }

  private:
    KRYS_NODISCARD float CurrentAngle(SVGMarkerType type) const noexcept
    {
      // For details of this calculation, see:
      // http://www.w3.org/TR/SVG/single-page.html#painting-MarkerElement
      FloatPoint inSlope(_inslopePoints[1] - _inslopePoints[0]);
      FloatPoint outSlope(_outslopePoints[1] - _outslopePoints[0]);

      double inAngle = rad2deg(static_cast<double>(inSlope.SlopeAngleRadians()));
      double outAngle = rad2deg(static_cast<double>(outSlope.SlopeAngleRadians()));

      switch (type)
      {
        case StartMarker:
        {
          if (_reverseStart)
          {
            return static_cast<float>(outAngle - 180);
          }
          return static_cast<float>(outAngle);
        }
        case MidMarker:
        {
          // WK193015: Prevent bugs due to angles being non-continuous.
          if (std::abs(inAngle - outAngle) > 180)
          {
            inAngle += 360;
          }
          return static_cast<float>((inAngle + outAngle) / 2);
        }
        case EndMarker:
        {
          return static_cast<float>(inAngle);
        }
      }

      krys_assert_msg(false, "Not reached");
      return 0;
    }

    void UpdateOutslope(const FloatPoint &point) noexcept
    {
      _outslopePoints[0] = _origin;
      _outslopePoints[1] = point;
    }

    void UpdateInslope(const FloatPoint &point) noexcept
    {
      _inslopePoints[0] = _origin;
      _inslopePoints[1] = point;
    }

    void UpdateMarkerDataForPathElement(const PathElement &element) noexcept
    {
      const auto &points = element.Points;

      switch (element.ElementType)
      {
        case PathElement::Type::AddQuadCurveToPoint:
        {
          // FIXME: https://bugs.webkit.org/show_bug.cgi?id=33115 (PathElement::Type::AddQuadCurveToPoint not
          // handled for <marker>)
          _origin = points[1];
          break;
        }
        case PathElement::Type::AddCurveToPoint:
        {
          _inslopePoints[0] = points[1];
          _inslopePoints[1] = points[2];
          _origin = points[2];
          break;
        }
        case PathElement::Type::MoveToPoint:
        {
          _subpathStart = points[0];
          UpdateInslope(points[0]);
          _origin = points[0];
          break;
        }
        case PathElement::Type::AddLineToPoint:
        {
          UpdateInslope(points[0]);
          _origin = points[0];
          break;
        }
        case PathElement::Type::CloseSubpath:
        {
          UpdateInslope(points[0]);
          _origin = _subpathStart;
          _subpathStart = FloatPoint();
          break;
        }
      }
    }
  };
}
