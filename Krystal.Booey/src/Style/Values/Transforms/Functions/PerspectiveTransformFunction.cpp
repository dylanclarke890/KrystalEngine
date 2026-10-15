#include "Krystal.Booey/Style/Values/Transforms/Functions/PerspectiveTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/PerspectiveTransformOperation.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo::style
{
  PerspectiveTransformFunction::PerspectiveTransformFunction(Perspective p) noexcept
      : TransformFunctionBase(TransformFunctionBase::Type::Perspective), _p(p)
  {
  }

  Ref<const PerspectiveTransformFunction> PerspectiveTransformFunction::Create(Perspective p) noexcept
  {
    return AdoptRef(*new PerspectiveTransformFunction(p));
  }

  Ref<const TransformFunctionBase> PerspectiveTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new PerspectiveTransformFunction(_p));
  }

  Ref<TransformOperation> PerspectiveTransformFunction::ToPlatform(const FloatSize &) const noexcept
  {
    return krys::SwitchOn(
      _p, [](const css::keywords::None &) -> Ref<TransformOperation>
      { return PerspectiveTransformOperation::Create(null); },
      [](const Perspective::Length &value) -> Ref<TransformOperation>
      { return PerspectiveTransformOperation::Create(Evaluate<float>(value, ZoomNeeded {})); });
  }

  bool PerspectiveTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    auto &otherPerspective = Downcast<PerspectiveTransformFunction>(other);
    return _p == otherPerspective._p;
  }

  void PerspectiveTransformFunction::Apply(TransformationMatrix &transform, const FloatSize &) const noexcept
  {
    if (auto value = _p.TryValue())
    {
      transform.ApplyPerspective(std::max(1.0f, Evaluate<float>(*value, ZoomNeeded {})));
    }
  }

  Maybe<float> PerspectiveTransformFunction::UnresolvedFloatValue() const noexcept
  {
    return krys::SwitchOn(
      _p, [](const css::keywords::None &) -> Maybe<float> { return std::nullopt; },
      [](const Perspective::Length &value) -> Maybe<float>
      {
        // From https://drafts.csswg.org/css-transforms-2/#perspective-property:
        // "As very small <length> values can produce bizarre rendering results and stress the numerical
        // accuracy of transform calculations, values less than 1px must be treated as 1px for rendering
        // purposes. (This clamping does not affect the underlying value, so perspective: 0; in a stylesheet
        // will still serialize back as 0.)"
        return std::max(1.0f, value.UnresolvedValue());
      });
  }

  Ref<const TransformFunctionBase> PerspectiveTransformFunction::Blend(const TransformFunctionBase *from,
                                                                       const BlendingContext &context,
                                                                       bool blendToIdentity) const noexcept
  {
    if (!SharedPrimitiveType(from))
    {
      return ShareRef(*this);
    }

    // https://drafts.csswg.org/css-transforms-2/#interpolation-of-transform-functions
    // says that we should run matrix decomposition and then run the rules for
    // interpolation of matrices, but we know what those rules are going to
    // yield, so just do that directly.
    auto getInverse = [](const auto &operation)
    {
      if (auto value = operation->UnresolvedFloatValue())
      {
        return 1.0 / *value;
      }
      return 0.0;
    };

    double ourInverse = getInverse(this);
    double fromPInverse, toPInverse;
    if (blendToIdentity)
    {
      fromPInverse = ourInverse;
      toPInverse = 0.0;
    }
    else
    {
      fromPInverse = from ? getInverse(Downcast<PerspectiveTransformFunction>(from)) : 0.0;
      toPInverse = ourInverse;
    }

    double pInverse = boo::Blend(fromPInverse, toPInverse, context);
    if (pInverse > 0.0 && std::isnormal(pInverse))
    {
      return PerspectiveTransformFunction::Create(Perspective::Length {static_cast<float>(1.0 / pInverse)});
    }
    return PerspectiveTransformFunction::Create(css::keywords::None {});
  }

}
