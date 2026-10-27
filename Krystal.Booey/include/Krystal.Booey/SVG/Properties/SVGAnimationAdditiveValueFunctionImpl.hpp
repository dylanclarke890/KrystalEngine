#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveValueFunction.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/SVG/SVGAngleValue.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGLengthValue.hpp"
#include <cmath>

namespace krys::boo::svg
{
  class SVGAnimatedAngleOrientAnimator;
  class SVGAnimatedIntegerPairAnimator;
  class SVGAnimatedNumberPairAnimator;

  class SVGAnimationAngleFunction : public SVGAnimationAdditiveValueFunction<SVGAngleValue>
  {
    friend class SVGAnimatedAngleOrientAnimator;

  public:
    using Base = SVGAnimationAdditiveValueFunction<SVGAngleValue>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &, dom::DOMStringView, dom::DOMStringView) noexcept override
    {
      // Values will be set by SVGAnimatedAngleOrientAnimator.
      krys_debug_assert_msg(false, "The angle's values are set by its orient animator");
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount, SVGAngleValue &animated) noexcept
    {
      float number = animated.Value();
      number =
        Base::Animate(progress, repeatCount, _from.Value(), _to.Value(), ToAtEndOfDuration().Value(), number);
      animated.SetValue(number);
    }

  private:
    void AddFromAndToValues(SVGElement &) noexcept override
    {
      _to.SetValue(_to.Value() + _from.Value());
    }
  };

  class SVGAnimationColorFunction : public SVGAnimationAdditiveValueFunction<Color>
  {
  public:
    using Base = SVGAnimationAdditiveValueFunction<Color>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _from = SVGPropertyTraits<Color>::FromString(targetElement, from);
      _to = SVGPropertyTraits<Color>::FromString(targetElement, to);
    }

    void SetToAtEndOfDurationValue(SVGElement &targetElement,
                                   dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      _toAtEndOfDuration = SVGPropertyTraits<Color>::FromString(targetElement, toAtEndOfDuration);
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount, Color &animated) noexcept
    {
      auto simpleAnimated = animated.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
      auto simpleFrom = _animationMode == AnimationMode::To
                          ? simpleAnimated
                          : _from.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
      auto simpleTo = _to.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
      auto simpleToAtEndOfDuration = ToAtEndOfDuration().ToColorTypeLossy<SRGBA<uint8>>().Resolved();

      float red = Base::Animate(progress, repeatCount, simpleFrom.Red, simpleTo.Red,
                                simpleToAtEndOfDuration.Red, simpleAnimated.Red);
      float green = Base::Animate(progress, repeatCount, simpleFrom.Green, simpleTo.Green,
                                  simpleToAtEndOfDuration.Green, simpleAnimated.Green);
      float blue = Base::Animate(progress, repeatCount, simpleFrom.Blue, simpleTo.Blue,
                                 simpleToAtEndOfDuration.Blue, simpleAnimated.Blue);
      float alpha = Base::Animate(progress, repeatCount, simpleFrom.Alpha, simpleTo.Alpha,
                                  simpleToAtEndOfDuration.Alpha, simpleAnimated.Alpha);

      animated = CreateFromComponentsClamping<SRGBA<uint8>>(std::lround(red), std::lround(green),
                                                            std::lround(blue), std::lround(alpha));
    }

    KRYS_NODISCARD Maybe<float> CalculateDistance(SVGElement &, dom::DOMStringView from,
                                                  dom::DOMStringView to) const noexcept override;

  private:
    void AddFromAndToValues(SVGElement &) noexcept override
    {
      auto simpleFrom = _from.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
      auto simpleTo = _to.ToColorTypeLossy<SRGBA<uint8>>().Resolved();

      // Ignores any alpha and sets alpha on result to 100% opaque.
      _to = CreateFromComponentsClamping<SRGBA<uint8>>(
        simpleTo.Red + simpleFrom.Red, simpleTo.Green + simpleFrom.Green, simpleTo.Blue + simpleFrom.Blue);
    }
  };

  class SVGAnimationIntegerFunction final : public SVGAnimationAdditiveValueFunction<int32>
  {
    friend class SVGAnimatedIntegerPairAnimator;

  public:
    using Base = SVGAnimationAdditiveValueFunction<int32>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _from = SVGPropertyTraits<int32>::FromString(targetElement, from);
      _to = SVGPropertyTraits<int32>::FromString(targetElement, to);
    }

    void SetToAtEndOfDurationValue(SVGElement &targetElement,
                                   dom::DOMStringView toAtEndOfDuration) noexcept final
    {
      _toAtEndOfDuration = SVGPropertyTraits<int32>::FromString(targetElement, toAtEndOfDuration);
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount, int32 &animated) noexcept
    {
      animated = static_cast<int32>(
        std::roundf(Base::Animate(progress, repeatCount, static_cast<float>(_from), static_cast<float>(_to),
                                  static_cast<float>(ToAtEndOfDuration()), static_cast<float>(animated))));
    }

    KRYS_NODISCARD Maybe<float> CalculateDistance(SVGElement &, dom::DOMStringView,
                                                  dom::DOMStringView) const noexcept final;

  private:
    void AddFromAndToValues(SVGElement &) noexcept final
    {
      _to += _from;
    }
  };

  class SVGAnimationLengthFunction : public SVGAnimationAdditiveValueFunction<SVGLengthValue>
  {
    using Base = SVGAnimationAdditiveValueFunction<SVGLengthValue>;

  private:
    SVGLengthMode _lengthMode;

  public:
    SVGAnimationLengthFunction(AnimationMode animationMode, CalcMode calcMode, bool isAccumulated,
                               bool isAdditive, SVGLengthMode lengthMode) noexcept
        : Base(animationMode, calcMode, isAccumulated, isAdditive), _lengthMode(lengthMode)
    {
    }

    void SetFromAndToValues(SVGElement &, dom::DOMStringView from, dom::DOMStringView to) noexcept override
    {
      _from = SVGLengthValue(_lengthMode, from);
      _to = SVGLengthValue(_lengthMode, to);
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      _toAtEndOfDuration = SVGLengthValue(_lengthMode, toAtEndOfDuration);
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount,
                 SVGLengthValue &animated) noexcept
    {
      SVGLengthContext lengthContext(&targetElement);
      SVGLengthType lengthType = progress < 0.5 ? _from.LengthType() : _to.LengthType();

      float from = (_animationMode == AnimationMode::To ? animated : _from).Value(lengthContext);
      float to = _to.Value(lengthContext);
      float toAtEndOfDuration = ToAtEndOfDuration().Value(lengthContext);
      float value = animated.Value(lengthContext);

      value = Base::Animate(progress, repeatCount, from, to, toAtEndOfDuration, value);
      animated = {lengthContext, value, lengthType, _lengthMode};
    }

    KRYS_NODISCARD Maybe<float> CalculateDistance(SVGElement &targetElement, dom::DOMStringView from,
                                                  dom::DOMStringView to) const noexcept override
    {
      SVGLengthContext lengthContext(&targetElement);
      auto fromLength = SVGLengthValue(_lengthMode, from);
      auto toLength = SVGLengthValue(_lengthMode, to);
      return std::abs(toLength.Value(lengthContext) - fromLength.Value(lengthContext));
    }

  private:
    void AddFromAndToValues(SVGElement &targetElement) noexcept override
    {
      SVGLengthContext lengthContext(&targetElement);
      (void)_to.SetValue(lengthContext, _to.Value(lengthContext) + _from.Value(lengthContext));
    }
  };

  class SVGAnimationNumberFunction : public SVGAnimationAdditiveValueFunction<float>
  {
    friend class SVGAnimatedNumberPairAnimator;

  public:
    using Base = SVGAnimationAdditiveValueFunction<float>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _from = SVGPropertyTraits<float>::FromString(targetElement, from);
      _to = SVGPropertyTraits<float>::FromString(targetElement, to);
    }

    void SetToAtEndOfDurationValue(SVGElement &targetElement,
                                   dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      _toAtEndOfDuration = SVGPropertyTraits<float>::FromString(targetElement, toAtEndOfDuration);
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount, float &animated) noexcept
    {
      float from = _animationMode == AnimationMode::To ? animated : _from;
      animated = Base::Animate(progress, repeatCount, from, _to, ToAtEndOfDuration(), animated);
    }

    KRYS_NODISCARD Maybe<float> CalculateDistance(SVGElement &, dom::DOMStringView from,
                                                  dom::DOMStringView to) const noexcept override;

  private:
    void AddFromAndToValues(SVGElement &) noexcept override
    {
      _to += _from;
    }
  };

  class SVGAnimationPathSegListFunction : public SVGAnimationAdditiveValueFunction<SVGPathByteStream>
  {
  public:
    using Base = SVGAnimationAdditiveValueFunction<SVGPathByteStream>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &, dom::DOMStringView from, dom::DOMStringView to) noexcept override
    {
      _from = SVGPathByteStream(from);
      _to = SVGPathByteStream(to);
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      _toAtEndOfDuration = SVGPathByteStream(toAtEndOfDuration);
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount, SVGPathByteStream &animated) noexcept
    {
      SVGPathByteStream underlyingPath;
      if (_animationMode == AnimationMode::To)
      {
        underlyingPath = animated;
      }

      const SVGPathByteStream &from = _animationMode == AnimationMode::To ? underlyingPath : _from;

      // Cache the current animated value before the buildAnimatedSVGPathByteStream() clears animatedPath.
      SVGPathByteStream lastAnimated;
      if (!from.Size() || (_isAdditive && _animationMode != AnimationMode::To))
      {
        lastAnimated = animated;
      }

      (void)BuildAnimatedSVGPathByteStream(from, _to, animated, progress);

      // Handle additive='sum'.
      if (!lastAnimated.IsEmpty())
      {
        (void)AddToSVGPathByteStream(animated, lastAnimated);
      }

      // Handle accumulate='sum'.
      if (_isAccumulated && repeatCount)
      {
        (void)AddToSVGPathByteStream(animated, ToAtEndOfDuration(), repeatCount);
      }
    }

  private:
    void AddFromAndToValues(SVGElement &) noexcept override
    {
      if (!_from.Size() || _from.Size() != _to.Size())
      {
        return;
      }
      (void)AddToSVGPathByteStream(_to, _from);
    }
  };

  class SVGAnimationRectFunction : public SVGAnimationAdditiveValueFunction<FloatRect>
  {
  public:
    using Base = SVGAnimationAdditiveValueFunction<FloatRect>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &targetElement, dom::DOMStringView from,
                            dom::DOMStringView to) noexcept override
    {
      _from = SVGPropertyTraits<FloatRect>::FromString(targetElement, from);
      _to = SVGPropertyTraits<FloatRect>::FromString(targetElement, to);
    }

    void SetToAtEndOfDurationValue(SVGElement &targetElement,
                                   dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      _toAtEndOfDuration = SVGPropertyTraits<FloatRect>::FromString(targetElement, toAtEndOfDuration);
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount, FloatRect &animated) noexcept
    {
      FloatRect from = _animationMode == AnimationMode::To ? animated : _from;

      float x =
        Base::Animate(progress, repeatCount, from.X(), _to.X(), ToAtEndOfDuration().X(), animated.X());
      float y =
        Base::Animate(progress, repeatCount, from.Y(), _to.Y(), ToAtEndOfDuration().Y(), animated.Y());
      float width = Base::Animate(progress, repeatCount, from.Width(), _to.Width(),
                                  ToAtEndOfDuration().Width(), animated.Width());
      float height = Base::Animate(progress, repeatCount, from.Height(), _to.Height(),
                                   ToAtEndOfDuration().Height(), animated.Height());

      animated = {x, y, width, height};
    }

  private:
    void AddFromAndToValues(SVGElement &) noexcept override
    {
      _to += _from;
    }
  };
}
