#include "Krystal.Booey/Style/Values/Transforms/TransformList.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/Matrix3DTransformFunction.hpp"

namespace krys::boo::style
{
  void TransformList::Apply(TransformationMatrix &matrix, const FloatSize &size, size_t start) const noexcept
  {
    for (size_t i = start; i < _value.size(); ++i)
    {
      _value[i]->Apply(matrix, size);
    }
  }

  bool TransformList::Has3DOperation() const noexcept
  {
    return std::ranges::any_of(*this, [](auto &function) { return function->Is3DOperation(); });
  }

  bool TransformList::IsRepresentableIn2D() const noexcept
  {
    return std::ranges::all_of(*this, [](auto &function) { return function->IsRepresentableIn2D(); });
  }

  bool TransformList::AffectedByTransformOrigin() const noexcept
  {
    return std::ranges::any_of(*this, [](auto &function) { return function->IsAffectedByTransformOrigin(); });
  }

  bool TransformList::IsInvertible(const LayoutSize &size) const noexcept
  {
    TransformationMatrix transform;
    Apply(transform, size);
    return transform.IsInvertible();
  }

  bool TransformList::ContainsNonInvertibleMatrix(const LayoutSize &boxSize) const noexcept
  {
    return (HasTransformOfType<TransformFunctionType::Matrix>()
            || HasTransformOfType<TransformFunctionType::Matrix3D>())
           && !IsInvertible(boxSize);
  }

  TransformFunctionSizeDependencies TransformList::ComputeSizeDependencies() const noexcept
  {
    TransformFunctionSizeDependencies result {};
    for (auto &function : _value)
    {
      auto [isWidthDependent, isHeightDependent] = function->ComputeSizeDependencies();
      if (isWidthDependent)
      {
        result.IsWidthDependent = true;
      }
      if (isHeightDependent)
      {
        result.IsHeightDependent = true;
      }
    }
    return result;
  }

  // MARK: - Blending

  KRYS_NODISCARD static bool ShouldFallBackToDiscreteInterpolation(const TransformList &from,
                                                                   const TransformList &to,
                                                                   const LayoutSize &boxSize) noexcept
  {
    return from.ContainsNonInvertibleMatrix(boxSize) || to.ContainsNonInvertibleMatrix(boxSize);
  }

  bool Blending<TransformList>::CanBlend(const TransformList &from, const TransformList &to,
                                         CompositeOperation compositeOperation) noexcept
  {
    if (compositeOperation == CompositeOperation::Replace)
    {
      return !ShouldFallBackToDiscreteInterpolation(from, to, {});
    }
    return true;
  }

  TransformList Blending<TransformList>::Blend(const TransformList &from, const TransformList &to,
                                               const BlendingContext &context) noexcept
  {
    auto fromLength = static_cast<uint32>(from.size());
    auto toLength = static_cast<uint32>(to.size());
    auto maxLength = std::max(fromLength, toLength);

    if (context.CompositeOperation == CompositeOperation::Add)
    {
      krys_debug_assert(context.Progress == 1.0);

      return TransformList::Container::createWithSizeFromGenerator(fromLength + toLength,
                                                                   [&](auto index)
                                                                   {
                                                                     if (index < fromLength)
                                                                     {
                                                                       return from[index];
                                                                     }
                                                                     return to[index - fromLength];
                                                                   });
    }

    // FIXME: Blend against the border box of the interpolation client's renderer, once there's an
    // interpolation context and render tree. Until then, size-dependent functions blend against an empty box.
    LayoutSize boxSize;

    bool shouldFallBackToDiscrete = ShouldFallBackToDiscreteInterpolation(from, to, boxSize);

    auto createBlendedMatrixFunctionFromOperationsSuffix = [&](size_t i) -> TransformFunction
    {
      TransformationMatrix fromTransform;
      from.Apply(fromTransform, boxSize, i);

      TransformationMatrix toTransform;
      to.Apply(toTransform, boxSize, i);

      auto progress = context.Progress;
      auto compositeOperation = context.CompositeOperation;
      if (shouldFallBackToDiscrete)
      {
        progress = progress < 0.5 ? 0 : 1;
        compositeOperation = CompositeOperation::Replace;
      }

      toTransform.Blend(fromTransform, progress, compositeOperation);
      return TransformFunction {Matrix3DTransformFunction::Create(toTransform)};
    };

    auto createBlendedFunction = [&](size_t i) -> TransformFunction
    {
      const TransformFunctionBase *fromFunction = (i < fromLength) ? from._value[i].Value.get() : nullptr;
      const TransformFunctionBase *toFunction = (i < toLength) ? to._value[i].Value.get() : nullptr;

      if (fromFunction && toFunction)
      {
        return TransformFunction {toFunction->Blend(fromFunction, context)};
      }
      if (!fromFunction)
      {
        return TransformFunction {toFunction->Blend(nullptr, 1 - context.Progress, true)};
      }
      return TransformFunction {fromFunction->Blend(nullptr, context, true)};
    };

    if (shouldFallBackToDiscrete)
    {
      return TransformList {createBlendedMatrixFunctionFromOperationsSuffix(0)};
    }

    // FIXME: Use the interpolation client's precomputed prefix of shared primitives (for `transform`, when
    // not accumulating), once there's an interpolation context. Until then the prefix comes from the two
    // lists.
    Maybe<uint32> prefixLengthFromContext;

    auto prefixLength = [&]() -> Maybe<uint32>
    {
      // If either of the transform list is empty, then we should not attempt to do a matrix blend.
      if (!fromLength || !toLength)
      {
        return {};
      }

      for (uint32 i = 0; i < maxLength; i++)
      {
        if (prefixLengthFromContext && i >= *prefixLengthFromContext)
        {
          return i;
        }

        const TransformFunctionBase *fromFunction = (i < fromLength) ? from._value[i].Value.get() : nullptr;
        const TransformFunctionBase *toFunction = (i < toLength) ? to._value[i].Value.get() : nullptr;
        if (fromFunction && toFunction && !fromFunction->SharedPrimitiveType(toFunction))
        {
          return i;
        }
      }

      return {};
    }();

    if (prefixLength)
    {
      return TransformList::Container::createWithSizeFromGenerator(
        *prefixLength + 1,
        [&](auto i)
        {
          if (i == *prefixLength)
          {
            return createBlendedMatrixFunctionFromOperationsSuffix(i);
          }
          return createBlendedFunction(i);
        });
    }

    return TransformList::Container::createWithSizeFromGenerator(maxLength, [&](auto i)
                                                                 { return createBlendedFunction(i); });
  }
}
