#include "Krystal.Booey/SVG/Internals/SVGPathBlender.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSegType.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathSource.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <functional>

namespace krys::boo::svg
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
  KRYS_NODISCARD static inline FloatPoint BlendFloatPoint(const FloatPoint &a, const FloatPoint &b,
                                                          float progress) noexcept
  {
    BlendingContext context {progress};
    return FloatPoint(Blend(a.X(), b.X(), context), Blend(a.Y(), b.Y(), context));
  }

  float SVGPathBlender::BlendAnimatedDimensonalFloat(float from, float to, FloatBlendMode blendMode,
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

    float fromValue = blendMode == FloatBlendMode::Horizontal ? _fromCurrentPoint.X() : _fromCurrentPoint.Y();
    float toValue = blendMode == FloatBlendMode::Horizontal ? _toCurrentPoint.X() : _toCurrentPoint.Y();

    // Transform toY to the coordinate mode of fromY
    float animValue = Blend(
      from, _fromMode == PathCoordinateMode::AbsoluteCoordinates ? to + toValue : to - toValue, {progress});

    if (_isInFirstHalfOfAnimation)
    {
      return animValue;
    }

    // Transform the animated point to the coordinate mode, needed for the current progress.
    float currentValue = Blend(fromValue, toValue, {progress});
    return _toMode == PathCoordinateMode::AbsoluteCoordinates ? animValue + currentValue
                                                              : animValue - currentValue;
  }

  FloatPoint SVGPathBlender::BlendAnimatedFloatPoint(const FloatPoint &fromPoint, const FloatPoint &toPoint,
                                                     float progress) noexcept
  {
    if (_addTypesCount)
    {
      krys_debug_assert(_fromMode == _toMode);
      FloatPoint repeatedToPoint = toPoint;
      repeatedToPoint.Scale(static_cast<float>(_addTypesCount));
      return fromPoint + repeatedToPoint;
    }

    if (_fromMode == _toMode)
    {
      return BlendFloatPoint(fromPoint, toPoint, progress);
    }

    // Transform toPoint to the coordinate mode of fromPoint
    FloatPoint animatedPoint = toPoint;
    if (_fromMode == PathCoordinateMode::AbsoluteCoordinates)
    {
      animatedPoint += _toCurrentPoint;
    }
    else
    {
      animatedPoint.Move(-_toCurrentPoint.X(), -_toCurrentPoint.Y());
    }

    animatedPoint = BlendFloatPoint(fromPoint, animatedPoint, progress);

    if (_isInFirstHalfOfAnimation)
    {
      return animatedPoint;
    }

    // Transform the animated point to the coordinate mode, needed for the current progress.
    FloatPoint currentPoint = BlendFloatPoint(_fromCurrentPoint, _toCurrentPoint, progress);
    if (_toMode == PathCoordinateMode::AbsoluteCoordinates)
    {
      return animatedPoint + currentPoint;
    }

    animatedPoint.Move(-currentPoint.X(), -currentPoint.Y());
    return animatedPoint;
  }

  template <typename Function>
  using InvokeResult = typename std::invoke_result_t<Function, SVGPathSource, FloatPoint>::value_type;
  template <typename Function>
  using ResultPair = std::pair<InvokeResult<Function>, InvokeResult<Function>>;
  template <typename Function>
  KRYS_NODISCARD static Maybe<ResultPair<Function>>
    PullFromSources(SVGPathSource &fromSource, SVGPathSource &toSource, Function &&function,
                    FloatPoint currentPoint) noexcept
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

    _consumer->MoveTo(BlendAnimatedFloatPoint(from.TargetPoint, to.TargetPoint, progress), false,
                      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint =
      _toMode == PathCoordinateMode::AbsoluteCoordinates ? to.TargetPoint : _toCurrentPoint + to.TargetPoint;

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

    _consumer->LineTo(BlendAnimatedFloatPoint(from.TargetPoint, to.TargetPoint, progress),
                      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint =
      _toMode == PathCoordinateMode::AbsoluteCoordinates ? to.TargetPoint : _toCurrentPoint + to.TargetPoint;
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
      BlendAnimatedDimensonalFloat(from.X, to.X, FloatBlendMode::Horizontal, progress),
      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint.SetX(
      _fromMode == PathCoordinateMode::AbsoluteCoordinates ? from.X : _fromCurrentPoint.X() + from.X);
    _toCurrentPoint.SetX(_toMode == PathCoordinateMode::AbsoluteCoordinates ? to.X
                                                                            : _toCurrentPoint.X() + to.X);
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

    _consumer->LineToVertical(BlendAnimatedDimensonalFloat(from.Y, to.Y, FloatBlendMode::Vertical, progress),
                              _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint.SetY(
      _fromMode == PathCoordinateMode::AbsoluteCoordinates ? from.Y : _fromCurrentPoint.Y() + from.Y);
    _toCurrentPoint.SetY(_toMode == PathCoordinateMode::AbsoluteCoordinates ? to.Y
                                                                            : _toCurrentPoint.Y() + to.Y);
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

    _consumer->CurveToCubic(BlendAnimatedFloatPoint(from.Point1, to.Point1, progress),
                            BlendAnimatedFloatPoint(from.Point2, to.Point2, progress),
                            BlendAnimatedFloatPoint(from.TargetPoint, to.TargetPoint, progress),
                            _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint =
      _toMode == PathCoordinateMode::AbsoluteCoordinates ? to.TargetPoint : _toCurrentPoint + to.TargetPoint;
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

    _consumer->CurveToCubicSmooth(BlendAnimatedFloatPoint(from.Point2, to.Point2, progress),
                                  BlendAnimatedFloatPoint(from.TargetPoint, to.TargetPoint, progress),
                                  _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint =
      _toMode == PathCoordinateMode::AbsoluteCoordinates ? to.TargetPoint : _toCurrentPoint + to.TargetPoint;
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

    _consumer->CurveToQuadratic(BlendAnimatedFloatPoint(from.Point1, to.Point1, progress),
                                BlendAnimatedFloatPoint(from.TargetPoint, to.TargetPoint, progress),
                                _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint =
      _toMode == PathCoordinateMode::AbsoluteCoordinates ? to.TargetPoint : _toCurrentPoint + to.TargetPoint;
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

    _consumer->CurveToQuadraticSmooth(BlendAnimatedFloatPoint(from.TargetPoint, to.TargetPoint, progress),
                                      _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    _fromCurrentPoint = _fromMode == PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint =
      _toMode == PathCoordinateMode::AbsoluteCoordinates ? to.TargetPoint : _toCurrentPoint + to.TargetPoint;
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
                       BlendAnimatedFloatPoint(from.TargetPoint, to.TargetPoint, progress),
                       _isInFirstHalfOfAnimation ? _fromMode : _toMode);
    }
    _fromCurrentPoint = _fromMode == PathCoordinateMode::AbsoluteCoordinates
                          ? from.TargetPoint
                          : _fromCurrentPoint + from.TargetPoint;
    _toCurrentPoint =
      _toMode == PathCoordinateMode::AbsoluteCoordinates ? to.TargetPoint : _toCurrentPoint + to.TargetPoint;
    return true;
  }

  KRYS_NODISCARD static inline PathCoordinateMode CoordinateModeOfCommand(const SVGPathSegType &type) noexcept
  {
    if (type < SVGPathSegType::MoveToAbs)
    {
      return PathCoordinateMode::AbsoluteCoordinates;
    }

    // Odd number = relative command
    if (ToUnderlying(type) % 2)
    {
      return PathCoordinateMode::RelativeCoordinates;
    }

    return PathCoordinateMode::AbsoluteCoordinates;
  }

  KRYS_NODISCARD static inline bool IsSegmentEqual(const SVGPathSegType &fromType,
                                                   const SVGPathSegType &toType,
                                                   const PathCoordinateMode &fromMode,
                                                   const PathCoordinateMode &toMode) noexcept
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
    if (fromMode == PathCoordinateMode::AbsoluteCoordinates)
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
