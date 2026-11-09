/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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
 *
 */

#include "Krystal.Booey/Graphics/Transforms/TransformOperations.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/Matrix3DTransformOperation.hpp"

namespace krys::boo
{
  TransformOperations::TransformOperations(Ref<TransformOperation> &&operation) noexcept
  {
    _operations.push_back(krys::move(operation));
  }

  TransformOperations::TransformOperations(List<Ref<TransformOperation>> &&operations) noexcept
      : _operations(krys::move(operations))
  {
  }

  bool TransformOperations::operator==(const TransformOperations &o) const noexcept
  {
    return std::ranges::equal(_operations, o._operations,
                              [](const auto &a, const auto &b) { return a.get() == b.get() || *a == *b; });
  }

  TransformOperations TransformOperations::Clone() const noexcept
  {
    List<Ref<TransformOperation>> operations;
    operations.reserve(_operations.size());
    for (const auto &op : _operations)
    {
      operations.push_back(op->Clone());
    }
    return TransformOperations {krys::move(operations)};
  }

  void TransformOperations::Apply(TransformationMatrix &matrix, uint32 start) const noexcept
  {
    for (size_t i = start; i < _operations.size(); ++i)
    {
      _operations[i]->Apply(matrix);
    }
  }

  bool TransformOperations::IsInvertible() const noexcept
  {
    TransformationMatrix transform;
    Apply(transform);
    return transform.IsInvertible();
  }

  bool TransformOperations::ContainsNonInvertibleMatrix() const noexcept
  {
    return (HasTransformOfType<TransformOperation::Type::Matrix>()
            || HasTransformOfType<TransformOperation::Type::Matrix3D>())
           && !IsInvertible();
  }

  TransformOperations Blend(const TransformOperations &from, const TransformOperations &to,
                            const BlendingContext &context) noexcept
  {
    bool shouldFallBackToDiscreteInterpolation =
      from.ContainsNonInvertibleMatrix() || to.ContainsNonInvertibleMatrix();

    auto createBlendedMatrixOperationFromOperationsSuffix = [&](uint32 start) -> Ref<TransformOperation>
    {
      TransformationMatrix fromTransform;
      from.Apply(fromTransform, start);

      TransformationMatrix toTransform;
      to.Apply(toTransform, start);

      auto progress = context.Progress;
      auto compositeOperation = context.CompositeOperation;
      if (shouldFallBackToDiscreteInterpolation)
      {
        progress = progress < 0.5 ? 0 : 1;
        compositeOperation = CompositeOperation::Replace;
      }

      toTransform.Blend(fromTransform, progress, compositeOperation);
      return Matrix3DTransformOperation::Create(toTransform);
    };

    if (shouldFallBackToDiscreteInterpolation)
    {
      return TransformOperations {createBlendedMatrixOperationFromOperationsSuffix(0)};
    }

    auto fromOperationCount = static_cast<uint32>(from.size());
    auto toOperationCount = static_cast<uint32>(to.size());
    auto maxOperationCount = std::max(fromOperationCount, toOperationCount);

    List<Ref<TransformOperation>> operations;
    operations.reserve(maxOperationCount);

    for (uint32 i = 0; i < maxOperationCount; i++)
    {
      const TransformOperation *fromOperation = from.At(i);
      const TransformOperation *toOperation = to.At(i);

      // If either of the transform list is empty, then we should not attempt to do a matrix blend.
      if (fromOperationCount && toOperationCount)
      {
        if (fromOperation && toOperation && !fromOperation->SharedPrimitiveType(toOperation))
        {
          operations.push_back(createBlendedMatrixOperationFromOperationsSuffix(i));
          operations.shrink_to_fit();
          return TransformOperations {krys::move(operations)};
        }
      }

      // Either list may be shorter; its missing operations blend to (or from) identity.
      auto blendedOperation = [&]() -> Ref<TransformOperation>
      {
        if (fromOperation && toOperation)
        {
          return toOperation->Blend(fromOperation, context);
        }
        if (!fromOperation)
        {
          return toOperation->Blend(nullptr, BlendingContext {1 - context.Progress}, true);
        }
        return fromOperation->Blend(nullptr, context, true);
      }();
      operations.push_back(krys::move(blendedOperation));
    }

    return TransformOperations {krys::move(operations)};
  }
}
