#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveListFunction.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGLengthList.hpp"
#include "Krystal.Booey/SVG/SVGNumberList.hpp"
#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "Krystal.Booey/SVG/SVGTransformDistance.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  class SVGAnimationLengthListFunction : public SVGAnimationAdditiveListFunction<SVGLengthList>
  {
    using Base = SVGAnimationAdditiveListFunction<SVGLengthList>;

  public:
    SVGAnimationLengthListFunction(AnimationMode animationMode, CalcMode calcMode, bool isAccumulated,
                                   bool isAdditive, SVGLengthMode lengthMode) noexcept
        : Base(animationMode, calcMode, isAccumulated, isAdditive, lengthMode)
    {
    }

    void SetFromAndToValues(SVGElement &, dom::DOMStringView from, dom::DOMStringView to) noexcept override
    {
      (void)_from->Parse(from);
      (void)_to->Parse(to);
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      (void)_toAtEndOfDuration->Parse(toAtEndOfDuration);
    }

    void Animate(SVGElement &targetElement, float progress, uint32 repeatCount,
                 RefPtr<SVGLengthList> &animated) noexcept
    {
      if (!AdjustAnimatedList(_animationMode, progress, animated))
      {
        return;
      }

      const auto &fromItems = _animationMode == AnimationMode::To ? animated->Items() : _from->Items();
      const auto &toItems = _to->Items();
      const auto &toAtEndOfDurationItems = ToAtEndOfDuration()->Items();
      auto &animatedItems = animated->Items();
      SVGLengthMode lengthMode = animated->LengthMode();

      SVGLengthContext lengthContext(&targetElement);
      for (size_t i = 0; i < toItems.size(); ++i)
      {
        SVGLengthType lengthType =
          (i < fromItems.size() && progress < 0.5 ? fromItems : toItems)[i]->Value().LengthType();

        float from = i < fromItems.size() ? fromItems[i]->Value().Value(lengthContext) : 0;
        float to = toItems[i]->Value().Value(lengthContext);
        float toAtEndOfDuration =
          i < toAtEndOfDurationItems.size() ? toAtEndOfDurationItems[i]->Value().Value(lengthContext) : 0;
        float value = animatedItems[i]->Value().Value(lengthContext);

        value = Base::Animate(progress, repeatCount, from, to, toAtEndOfDuration, value);
        (void)animatedItems[i]->Value().SetValue(lengthContext, value, lengthType, lengthMode);
      }
    }

  private:
    void AddFromAndToValues(SVGElement &targetElement) noexcept override
    {
      const auto &fromItems = _from->Items();
      const auto &toItems = _to->Items();

      if (!fromItems.size() || fromItems.size() != toItems.size())
      {
        return;
      }

      SVGLengthContext lengthContext(&targetElement);
      for (size_t i = 0; i < fromItems.size(); ++i)
      {
        const SVGLengthValue &fromValue = fromItems[i]->Value();
        SVGLengthValue &toValue = toItems[i]->Value();
        (void)toValue.SetValue(lengthContext, toValue.Value(lengthContext) + fromValue.Value(lengthContext));
      }
    }
  };

  class SVGAnimationNumberListFunction : public SVGAnimationAdditiveListFunction<SVGNumberList>
  {
  public:
    using Base = SVGAnimationAdditiveListFunction<SVGNumberList>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &, dom::DOMStringView from, dom::DOMStringView to) noexcept override
    {
      (void)_from->Parse(from);
      (void)_to->Parse(to);
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      (void)_toAtEndOfDuration->Parse(toAtEndOfDuration);
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount, RefPtr<SVGNumberList> &animated) noexcept
    {
      if (!AdjustAnimatedList(_animationMode, progress, animated))
      {
        return;
      }

      const auto &fromItems = _animationMode == AnimationMode::To ? animated->Items() : _from->Items();
      const auto &toItems = _to->Items();
      const auto &toAtEndOfDurationItems = ToAtEndOfDuration()->Items();
      auto &animatedItems = animated->Items();

      for (size_t i = 0; i < toItems.size(); ++i)
      {
        float from = i < fromItems.size() ? fromItems[i]->Value() : 0;
        float to = toItems[i]->Value();
        float toAtEndOfDuration = i < toAtEndOfDurationItems.size() ? toAtEndOfDurationItems[i]->Value() : 0;

        float &value = animatedItems[i]->Value();
        value = Base::Animate(progress, repeatCount, from, to, toAtEndOfDuration, value);
      }
    }

  private:
    void AddFromAndToValues(SVGElement &) noexcept override
    {
      const auto &fromItems = _from->Items();
      auto &toItems = _to->Items();

      if (!fromItems.size() || fromItems.size() != toItems.size())
      {
        return;
      }

      for (size_t i = 0; i < fromItems.size(); ++i)
      {
        toItems[i]->SetValue(fromItems[i]->Value() + toItems[i]->Value());
      }
    }
  };

  class SVGAnimationPointListFunction : public SVGAnimationAdditiveListFunction<SVGPointList>
  {
  public:
    using Base = SVGAnimationAdditiveListFunction<SVGPointList>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &, dom::DOMStringView from, dom::DOMStringView to) noexcept override
    {
      (void)_from->Parse(from);
      (void)_to->Parse(to);
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      (void)_toAtEndOfDuration->Parse(toAtEndOfDuration);
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount, RefPtr<SVGPointList> &animated) noexcept
    {
      if (!AdjustAnimatedList(_animationMode, progress, animated))
      {
        return;
      }

      const auto &fromItems = _animationMode == AnimationMode::To ? animated->Items() : _from->Items();
      const auto &toItems = _to->Items();
      const auto &toAtEndOfDurationItems = ToAtEndOfDuration()->Items();
      auto &animatedItems = animated->Items();

      for (size_t i = 0; i < toItems.size(); ++i)
      {
        FloatPoint from = i < fromItems.size() ? fromItems[i]->Value() : FloatPoint();
        FloatPoint to = toItems[i]->Value();
        FloatPoint toAtEndOfDuration =
          i < toAtEndOfDurationItems.size() ? toAtEndOfDurationItems[i]->Value() : FloatPoint();
        FloatPoint &animatedPoint = animatedItems[i]->Value();

        float animatedX =
          Base::Animate(progress, repeatCount, from.X(), to.X(), toAtEndOfDuration.X(), animatedPoint.X());
        float animatedY =
          Base::Animate(progress, repeatCount, from.Y(), to.Y(), toAtEndOfDuration.Y(), animatedPoint.Y());

        animatedPoint = {animatedX, animatedY};
      }
    }

  private:
    void AddFromAndToValues(SVGElement &) noexcept override
    {
      const auto &fromItems = _from->Items();
      auto &toItems = _to->Items();

      if (!fromItems.size() || fromItems.size() != toItems.size())
      {
        return;
      }

      for (size_t i = 0; i < fromItems.size(); ++i)
      {
        toItems[i]->SetValue(fromItems[i]->Value() + toItems[i]->Value());
      }
    }
  };

  class SVGAnimationTransformListFunction : public SVGAnimationAdditiveListFunction<SVGTransformList>
  {
  public:
    using Base = SVGAnimationAdditiveListFunction<SVGTransformList>;
    using Base::Base;

    void SetFromAndToValues(SVGElement &, dom::DOMStringView from, dom::DOMStringView to) noexcept override
    {
      _from->Parse(from);
      _to->Parse(to);
    }

    void SetToAtEndOfDurationValue(SVGElement &, dom::DOMStringView toAtEndOfDuration) noexcept override
    {
      _toAtEndOfDuration->Parse(toAtEndOfDuration);
    }

    void Animate(SVGElement &, float progress, uint32 repeatCount,
                 RefPtr<SVGTransformList> &animated) noexcept
    {
      // Pass false to 'resizeAnimatedIfNeeded', as the special post-multiplication behavior of
      // <animateTransform> needs to be respected below.
      if (!AdjustAnimatedList(_animationMode, progress, animated, false))
      {
        return;
      }

      // Spec: To animations provide specific functionality to get a smooth change from the underlying
      // value to the 'to' attribute value, which conflicts mathematically with the requirement for
      // additive transform animations to be post-multiplied. As a consequence, in SVG 1.1 the behavior
      // of to animations for 'animateTransform' is undefined.
      const auto &fromItems = _from->Items();
      const auto &toItems = _to->Items();
      const auto &toAtEndOfDurationItems = ToAtEndOfDuration()->Items();
      auto &animatedItems = animated->Items();

      // Never resize the animatedList to the _to size, instead either clear the list
      // or append to it.
      if (!animatedItems.empty() && (!_isAdditive || _animationMode == AnimationMode::To))
      {
        animated->ClearItems();
      }

      auto fromItemsSize = fromItems.size();

      static constexpr AffineTransform zerosAffineTransform = {0, 0, 0, 0, 0, 0};
      const SVGTransformValue &to = toItems[0]->Value();
      const SVGTransformValue zerosTransform = SVGTransformValue(to.Type(), zerosAffineTransform);

      const SVGTransformValue &from = fromItemsSize ? fromItems[0]->Value() : zerosTransform;
      SVGTransformValue current =
        SVGTransformDistance(from, to).ScaledDistance(progress).AddToSVGTransform(from);

      if (_isAccumulated && repeatCount)
      {
        const SVGTransformValue &toAtEndOfDuration =
          toAtEndOfDurationItems.size() ? toAtEndOfDurationItems[0]->Value() : zerosTransform;
        animatedItems.push_back(SVGTransform::Create(
          SVGTransformDistance::AddSVGTransforms(current, toAtEndOfDuration, repeatCount)));
      }
      else
      {
        animatedItems.push_back(SVGTransform::Create(current));
      }
    }

  private:
    void AddFromAndToValues(SVGElement &) noexcept override
    {
      const auto &fromItems = _from->Items();
      auto &toItems = _to->Items();

      if (!fromItems.size() || fromItems.size() != toItems.size())
      {
        return;
      }

      krys_debug_assert(fromItems.size() == 1);
      const Ref<SVGTransform> &from = fromItems[0];
      Ref<SVGTransform> &to = toItems[0];

      to->SetValue(SVGTransformDistance::AddSVGTransforms(from->Value(), to->Value()));
    }
  };
}
