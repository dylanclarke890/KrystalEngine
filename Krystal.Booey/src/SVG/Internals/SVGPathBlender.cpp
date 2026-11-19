/*
 * Copyright (C) Research In Motion Limited 2010, 2011. All rights reserved.
 * Copyright (C) 2015-2024 Apple Inc. All rights reserved.
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
 */

#include "Krystal.Booey/SVG/Internals/SVGPathBlender.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSegType.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSource.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <functional>

namespace krys::boo
{
  bool SVGPathBlender::AddAnimatedPath(SVGPathSource &fromSource, SVGPathSource &toSource,
                                       SVGPathConsumer &consumer, unsigned repeatCount) noexcept
  {
    SVGPathBlender blender(fromSource, toSource, &consumer);
    return blender.AddAnimatedPath(repeatCount);
  }

  bool SVGPathBlender::BlendAnimatedPath(SVGPathSource &fromSource, SVGPathSource &toSource,
                                         SVGPathConsumer &consumer, float progress) noexcept
  {
    SVGPathBlender blender(fromSource, toSource, &consumer);
    return blender.BlendAnimatedPath(progress);
  }

  bool SVGPathBlender::CanBlendPaths(SVGPathSource &fromSource, SVGPathSource &toSource) noexcept
  {
    SVGPathBlender blender(fromSource, toSource);
    return blender.CanBlendPaths();
  }

  SVGPathBlender::SVGPathBlender(SVGPathSource &fromSource, SVGPathSource &toSource,
                                 SVGPathConsumer *consumer) noexcept
      : _fromSource(fromSource), _toSource(toSource), _consumer(consumer)
  {
  }

  // Helper functions
  KRYS_NODISCARD static inline PointF BlendPointF(const PointF &a, const PointF &b, float progress) noexcept
  {
    BlendingContext context {progress};
    return PointF(Blend(a.X(), b.X(), context), Blend(a.Y(), b.Y(), context));
  }

  float SVGPathBlender::BlendAnimatedDimensonalFloat(float from, float to, svg::FloatBlendMode blendMode,
                                                     float progress) noexcept
  {
    if (_addTypesCount)
    {
      krys_debug_assert(_fromMode == _toMode);
      return from + to * _addTypesCount;
    }

    if (_fromMode == _toMode)
    {
      return Blend(from, to, {progress});
    }

    float fromValue =
      blendMode == svg::FloatBlendMode::Horizontal ? _fromCurrentPoint.X() : _fromCurrentPoint.Y();
    float toValue = blendMode == svg::FloatBlendMode::Horizontal ? _toCurrentPoint.X() : _toCurrentPoint.Y();

    // Transform toY to the coordinate mode of fromY
    float animValue =
      Blend(from, _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates ? to + toValue : to - toValue,
            {progress});

    if (_isInFirstHalfOfAnimation)
    {
      return animValue;
    }

    // Transform the animated point to the coordinate mode, needed for the current progress.
    float currentValue = Blend(fromValue, toValue, {progress});
    return _toMode == svg::PathCoordinateMode::AbsoluteCoordinates ? animValue + currentValue
                                                                   : animValue - currentValue;
  }

  PointF SVGPathBlender::BlendAnimatedPointF(const PointF &fromPoint, const PointF &toPoint,
                                             float progress) noexcept
  {
    if (_addTypesCount)
    {
      krys_debug_assert(_fromMode == _toMode);
      PointF repeatedToPoint = toPoint;
      repeatedToPoint.Scale(static_cast<float>(_addTypesCount));
      return fromPoint + repeatedToPoint;
    }

    if (_fromMode == _toMode)
    {
      return BlendPointF(fromPoint, toPoint, progress);
    }

    // Transform toPoint to the coordinate mode of fromPoint
    PointF animatedPoint = toPoint;
    if (_fromMode == svg::PathCoordinateMode::AbsoluteCoordinates)
    {
      animatedPoint += _toCurrentPoint;
    }
    else
    {
      animatedPoint.Move(-_toCurrentPoint.X(), -_toCurrentPoint.Y());
    }

    animatedPoint = BlendPointF(fromPoint, animatedPoint, progress);

    if (_isInFirstHalfOfAnimation)
    {
      return animatedPoint;
    }

    // Transform the animated point to the coordinate mode, needed for the current progress.
    PointF currentPoint = BlendPointF(_fromCurrentPoint, _toCurrentPoint, progress);
    if (_toMode == svg::PathCoordinateMode::AbsoluteCoordinates)
    {
      return animatedPoint + currentPoint;
    }

    animatedPoint.Move(-currentPoint.X(), -currentPoint.Y());
    return animatedPoint;
  }

  template <typename Function>
  using InvokeResult = typename std::invoke_result_t<Function, SVGPathSource, PointF>::value_type;
  template <typename Function>
  using ResultPair = std::pair<InvokeResult<Function>, InvokeResult<Function>>;

  template <typename Function>
  KRYS_NODISCARD static Maybe<ResultPair<Function>>
    PullFromSources(SVGPathSource &fromSource, SVGPathSource &toSource, Function &&function,
                    PointF currentPoint) noexcept
  {
    InvokeResult<Function> fromResult;
    if (fromSource.HasMoreData())
    {
      auto parsedFrom = std::invoke(function, fromSource, currentPoint);
      if (!parsedFrom)
      {
        return null;
      }
      fromResult = krys::move(*parsedFrom);
    }

    auto parsedTo = std::invoke(std::forward<Function>(function), toSource, currentPoint);
    if (!parsedTo)
    {
      return null;
    }

    return ResultPair<Function> {krys::move(fromResult), krys::move(*parsedTo)};
  }

  bool SVGPathBlender::BlendMoveToSegment(float progress) noexcept
  {
    auto result =
      PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseMoveToSegment, _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    _consumer->MoveTo(BlendAnimatedPointF(from.TargetPoint, to.TargetPoint, progress), false,
                      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint = _toMode == svg::PathCoordinateMode::AbsoluteCoordinates
                        ? to.TargetPoint
                        : _toCurrentPoint + to.TargetPoint;

    _fromSubpathPoint = _fromCurrentPoint;
    _toSubpathPoint = _toCurrentPoint;

    return true;
  }

  bool SVGPathBlender::BlendLineToSegment(float progress) noexcept
  {
    auto result =
      PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseLineToSegment, _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    _consumer->LineTo(BlendAnimatedPointF(from.TargetPoint, to.TargetPoint, progress),
                      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint = _toMode == svg::PathCoordinateMode::AbsoluteCoordinates
                        ? to.TargetPoint
                        : _toCurrentPoint + to.TargetPoint;
    return true;
  }

  bool SVGPathBlender::BlendLineToHorizontalSegment(float progress) noexcept
  {
    auto result = PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseLineToHorizontalSegment,
                                  _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    _consumer->LineToHorizontal(
      BlendAnimatedDimensonalFloat(from.X, to.X, svg::FloatBlendMode::Horizontal, progress),
      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint.SetX(
      _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates ? from.X : _fromCurrentPoint.X() + from.X);
    _toCurrentPoint.SetX(
      _toMode == svg::PathCoordinateMode::AbsoluteCoordinates ? to.X : _toCurrentPoint.X() + to.X);
    return true;
  }

  bool SVGPathBlender::BlendLineToVerticalSegment(float progress) noexcept
  {
    auto result =
      PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseLineToVerticalSegment, _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    _consumer->LineToVertical(
      BlendAnimatedDimensonalFloat(from.Y, to.Y, svg::FloatBlendMode::Vertical, progress),
      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint.SetY(
      _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates ? from.Y : _fromCurrentPoint.Y() + from.Y);
    _toCurrentPoint.SetY(
      _toMode == svg::PathCoordinateMode::AbsoluteCoordinates ? to.Y : _toCurrentPoint.Y() + to.Y);
    return true;
  }

  bool SVGPathBlender::BlendCurveToCubicSegment(float progress) noexcept
  {
    auto result =
      PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseCurveToCubicSegment, _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    _consumer->CurveToCubic(BlendAnimatedPointF(from.Point1, to.Point1, progress),
                            BlendAnimatedPointF(from.Point2, to.Point2, progress),
                            BlendAnimatedPointF(from.TargetPoint, to.TargetPoint, progress),
                            _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint = _toMode == svg::PathCoordinateMode::AbsoluteCoordinates
                        ? to.TargetPoint
                        : _toCurrentPoint + to.TargetPoint;
    return true;
  }

  bool SVGPathBlender::BlendCurveToCubicSmoothSegment(float progress) noexcept
  {
    auto result = PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseCurveToCubicSmoothSegment,
                                  _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    _consumer->CurveToCubicSmooth(BlendAnimatedPointF(from.Point2, to.Point2, progress),
                                  BlendAnimatedPointF(from.TargetPoint, to.TargetPoint, progress),
                                  _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint = _toMode == svg::PathCoordinateMode::AbsoluteCoordinates
                        ? to.TargetPoint
                        : _toCurrentPoint + to.TargetPoint;
    return true;
  }

  bool SVGPathBlender::BlendCurveToQuadraticSegment(float progress) noexcept
  {
    auto result = PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseCurveToQuadraticSegment,
                                  _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    _consumer->CurveToQuadratic(BlendAnimatedPointF(from.Point1, to.Point1, progress),
                                BlendAnimatedPointF(from.TargetPoint, to.TargetPoint, progress),
                                _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint = _toMode == svg::PathCoordinateMode::AbsoluteCoordinates
                        ? to.TargetPoint
                        : _toCurrentPoint + to.TargetPoint;
    return true;
  }

  bool SVGPathBlender::BlendCurveToQuadraticSmoothSegment(float progress) noexcept
  {
    auto result = PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseCurveToQuadraticSmoothSegment,
                                  _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    _consumer->CurveToQuadraticSmooth(BlendAnimatedPointF(from.TargetPoint, to.TargetPoint, progress),
                                      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint = _toMode == svg::PathCoordinateMode::AbsoluteCoordinates
                        ? to.TargetPoint
                        : _toCurrentPoint + to.TargetPoint;
    return true;
  }

  bool SVGPathBlender::BlendArcToSegment(float progress) noexcept
  {
    auto result =
      PullFromSources(_fromSource, _toSource, &SVGPathSource::ParseArcToSegment, _fromCurrentPoint);
    if (!result)
    {
      return false;
    }

    if (!_consumer)
    {
      return true;
    }

    auto [from, to] = *result;

    if (_addTypesCount)
    {
      krys_debug_assert(_fromMode == _toMode);
      auto scaledToTargetPoint = to.TargetPoint;
      scaledToTargetPoint.Scale(static_cast<float>(_addTypesCount));
      _consumer->ArcTo(from.Rx + to.Rx * _addTypesCount, from.Ry + to.Ry * _addTypesCount,
                       from.Angle + to.Angle * _addTypesCount, from.LargeArc || to.LargeArc,
                       from.Sweep || to.Sweep, from.TargetPoint + scaledToTargetPoint, _fromMode);
    }
    else
    {
      BlendingContext context {progress};
      _consumer->ArcTo(Blend(from.Rx, to.Rx, context), Blend(from.Ry, to.Ry, context),
                       Blend(from.Angle, to.Angle, context),
                       _isInFirstHalfOfAnimation ? from.LargeArc : to.LargeArc,
                       _isInFirstHalfOfAnimation ? from.Sweep : to.Sweep,
                       BlendAnimatedPointF(from.TargetPoint, to.TargetPoint, progress),
                       _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    }
    _fromCurrentPoint = _fromMode == svg::PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint = _toMode == svg::PathCoordinateMode::AbsoluteCoordinates
                        ? to.TargetPoint
                        : _toCurrentPoint + to.TargetPoint;
    return true;
  }

  KRYS_NODISCARD static inline svg::PathCoordinateMode
    CoordinateModeOfCommand(const SVGPathSegType &type) noexcept
  {
    if (type < SVGPathSegType::MoveToAbs)
    {
      return svg::PathCoordinateMode::AbsoluteCoordinates;
    }

    // Odd number = relative command
    if (ToUnderlying(type) % 2)
    {
      return svg::PathCoordinateMode::RelativeCoordinates;
    }

    return svg::PathCoordinateMode::AbsoluteCoordinates;
  }

  KRYS_NODISCARD static inline bool IsSegmentEqual(const SVGPathSegType &fromType,
                                                   const SVGPathSegType &toType,
                                                   const svg::PathCoordinateMode &fromMode,
                                                   const svg::PathCoordinateMode &toMode) noexcept
  {
    if (fromType == toType && (fromType == SVGPathSegType::Unknown || fromType == SVGPathSegType::ClosePath))
    {
      return true;
    }

    auto from = ToUnderlying(fromType);
    auto to = ToUnderlying(toType);
    if (fromMode == toMode)
    {
      return from == to;
    }
    if (fromMode == svg::PathCoordinateMode::AbsoluteCoordinates)
    {
      return from == to - 1;
    }
    return to == from - 1;
  }

  bool SVGPathBlender::AddAnimatedPath(unsigned repeatCount) noexcept
  {
    SetForScope change(_addTypesCount, repeatCount);
    return BlendAnimatedPath(0);
  }

  bool SVGPathBlender::CanBlendPaths() noexcept
  {
    float progress = 0.5;
    bool fromSourceHadData = _fromSource.HasMoreData();
    while (_toSource.HasMoreData())
    {
      SVGPathSegType fromCommand;
      if (fromSourceHadData)
      {
        auto parsedFromCommand = _fromSource.ParseSVGSegmentType();
        if (!parsedFromCommand)
        {
          return false;
        }
        fromCommand = *parsedFromCommand;
      }
      auto parsedtoCommand = _toSource.ParseSVGSegmentType();
      if (!parsedtoCommand)
      {
        return false;
      }
      SVGPathSegType toCommand = *parsedtoCommand;

      _toMode = CoordinateModeOfCommand(toCommand);
      _fromMode = fromSourceHadData ? CoordinateModeOfCommand(fromCommand) : _toMode;
      if (_fromMode != _toMode && _addTypesCount)
      {
        return false;
      }

      if (fromSourceHadData && !IsSegmentEqual(fromCommand, toCommand, _fromMode, _toMode))
      {
        return false;
      }

      switch (toCommand)
      {
        case SVGPathSegType::MoveToRel:
        case SVGPathSegType::MoveToAbs:
          if (!BlendMoveToSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToRel:
        case SVGPathSegType::LineToAbs:
          if (!BlendLineToSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToHorizontalRel:
        case SVGPathSegType::LineToHorizontalAbs:
          if (!BlendLineToHorizontalSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToVerticalRel:
        case SVGPathSegType::LineToVerticalAbs:
          if (!BlendLineToVerticalSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::ClosePath: break;
        case SVGPathSegType::CurveToCubicRel:
        case SVGPathSegType::CurveToCubicAbs:
          if (!BlendCurveToCubicSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToCubicSmoothRel:
        case SVGPathSegType::CurveToCubicSmoothAbs:
          if (!BlendCurveToCubicSmoothSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToQuadraticRel:
        case SVGPathSegType::CurveToQuadraticAbs:
          if (!BlendCurveToQuadraticSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToQuadraticSmoothRel:
        case SVGPathSegType::CurveToQuadraticSmoothAbs:
          if (!BlendCurveToQuadraticSmoothSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::ArcRel:
        case SVGPathSegType::ArcAbs:
          if (!BlendArcToSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::Unknown: return false;
      }

      if (!fromSourceHadData)
      {
        continue;
      }
      if (_fromSource.HasMoreData() != _toSource.HasMoreData())
      {
        return false;
      }
      if (!_fromSource.HasMoreData() || !_toSource.HasMoreData())
      {
        return true;
      }
    }

    return true;
  }

  bool SVGPathBlender::BlendAnimatedPath(float progress) noexcept
  {
    _isInFirstHalfOfAnimation = progress < 0.5f;

    bool fromSourceHadData = _fromSource.HasMoreData();
    while (_toSource.HasMoreData())
    {
      SVGPathSegType fromCommand;
      if (fromSourceHadData)
      {
        auto parsedFromCommand = _fromSource.ParseSVGSegmentType();
        if (!parsedFromCommand)
        {
          return false;
        }
        fromCommand = *parsedFromCommand;
      }
      auto parsedToCommand = _toSource.ParseSVGSegmentType();
      if (!parsedToCommand)
      {
        return false;
      }
      SVGPathSegType toCommand = *parsedToCommand;

      _toMode = CoordinateModeOfCommand(toCommand);
      _fromMode = fromSourceHadData ? CoordinateModeOfCommand(fromCommand) : _toMode;
      if (_fromMode != _toMode && _addTypesCount)
      {
        return false;
      }

      if (fromSourceHadData && !IsSegmentEqual(fromCommand, toCommand, _fromMode, _toMode))
      {
        return false;
      }

      switch (toCommand)
      {
        case SVGPathSegType::MoveToRel:
        case SVGPathSegType::MoveToAbs:
          if (!BlendMoveToSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToRel:
        case SVGPathSegType::LineToAbs:
          if (!BlendLineToSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToHorizontalRel:
        case SVGPathSegType::LineToHorizontalAbs:
          if (!BlendLineToHorizontalSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::LineToVerticalRel:
        case SVGPathSegType::LineToVerticalAbs:
          if (!BlendLineToVerticalSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::ClosePath:
          _consumer->ClosePath();
          _fromCurrentPoint = _fromSubpathPoint;
          _toCurrentPoint = _toSubpathPoint;
          break;
        case SVGPathSegType::CurveToCubicRel:
        case SVGPathSegType::CurveToCubicAbs:
          if (!BlendCurveToCubicSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToCubicSmoothRel:
        case SVGPathSegType::CurveToCubicSmoothAbs:
          if (!BlendCurveToCubicSmoothSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToQuadraticRel:
        case SVGPathSegType::CurveToQuadraticAbs:
          if (!BlendCurveToQuadraticSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::CurveToQuadraticSmoothRel:
        case SVGPathSegType::CurveToQuadraticSmoothAbs:
          if (!BlendCurveToQuadraticSmoothSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::ArcRel:
        case SVGPathSegType::ArcAbs:
          if (!BlendArcToSegment(progress))
          {
            return false;
          }
          break;
        case SVGPathSegType::Unknown: return false;
      }

      if (!fromSourceHadData)
      {
        continue;
      }
      if (_fromSource.HasMoreData() != _toSource.HasMoreData())
      {
        return false;
      }
      if (!_fromSource.HasMoreData() || !_toSource.HasMoreData())
      {
        return true;
      }
    }

    return true;
  }
}
