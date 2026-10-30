#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Rendering/Style/DropShadowFilterOperationWithStyleColor.hpp"

namespace krys::boo
{
  FilterOperations::FilterOperations(List<Ref<FilterOperation>> &&operations) noexcept
      : _operations(krys::move(operations))
  {
  }

  bool FilterOperations::operator==(const FilterOperations &other) const noexcept
  {
    return std::ranges::equal(_operations, other._operations,
                              [](const auto &a, const auto &b) { return a.get() == b.get() || *a == *b; });
  }

  FilterOperations FilterOperations::Clone() const noexcept
  {
    List<Ref<FilterOperation>> operations;
    operations.reserve(_operations.size());
    for (const auto &op : _operations)
    {
      operations.push_back(op->Clone());
    }
    return FilterOperations {krys::move(operations)};
  }

  bool FilterOperations::OperationsMatch(const FilterOperations &other) const noexcept
  {
    return std::ranges::equal(_operations, other._operations,
                              [](const auto &a, const auto &b) { return a->IsSameType(*b); });
  }

  bool FilterOperations::HasReferenceFilter() const noexcept
  {
    return HasFilterOfType<FilterOperation::Type::Reference>();
  }

  IntOutsets FilterOperations::Outsets() const noexcept
  {
    IntOutsets totalOutsets;
    for (const auto &operation : _operations)
    {
      switch (operation->GetType())
      {
        case FilterOperation::Type::Blur:
        {
          const auto &blurOperation = Downcast<BlurFilterOperation>(*operation);
          float stdDeviation = blurOperation.StdDeviation();
          IntSize outsetSize = FEGaussianBlur::CalculateOutsetSize({stdDeviation, stdDeviation});
          IntOutsets outsets {outsetSize.Height(), outsetSize.Width(), outsetSize.Height(),
                              outsetSize.Width()};
          totalOutsets += outsets;
          break;
        }
        case FilterOperation::Type::DropShadow:
        case FilterOperation::Type::DropShadowWithStyleColor:
        {
          const auto &dropShadowOperation = Downcast<DropShadowFilterOperationBase>(*operation);
          auto stdDeviation = static_cast<float>(dropShadowOperation.StdDeviation());
          IntSize outsetSize = FEGaussianBlur::CalculateOutsetSize({stdDeviation, stdDeviation});

          int top = std::max(0, outsetSize.Height() - dropShadowOperation.Y());
          int right = std::max(0, outsetSize.Width() + dropShadowOperation.X());
          int bottom = std::max(0, outsetSize.Height() + dropShadowOperation.Y());
          int left = std::max(0, outsetSize.Width() - dropShadowOperation.X());

          auto outsets = IntOutsets {top, right, bottom, left};
          totalOutsets += outsets;
          break;
        }
        case FilterOperation::Type::Reference: krys_debug_assert_msg(false, "Unreachable"); break;
        default:                               break;
      }
    }
    return totalOutsets;
  }

  bool FilterOperations::HasFilterThatAffectsOpacity() const noexcept
  {
    return std::ranges::any_of(_operations, [](const auto &op) { return op->AffectsOpacity(); });
  }

  bool FilterOperations::HasFilterThatMovesPixels() const noexcept
  {
    return std::ranges::any_of(_operations, [](const auto &op) { return op->MovesPixels(); });
  }

  bool FilterOperations::HasFilterThatShouldBeRestrictedBySecurityOrigin() const noexcept
  {
    return std::ranges::any_of(_operations,
                               [](const auto &op) { return op->ShouldBeRestrictedBySecurityOrigin(); });
  }

  bool FilterOperations::CanInterpolate(const FilterOperations &to,
                                        CompositeOperation compositeOperation) const noexcept
  {
    // https://drafts.fxtf.org/filter-effects/#interpolation-of-filters

    // We can't interpolate between lists if a reference filter is involved.
    if (HasReferenceFilter() || to.HasReferenceFilter())
    {
      return false;
    }

    // Additive and accumulative composition will always yield interpolation.
    if (compositeOperation != CompositeOperation::Replace)
    {
      return true;
    }

    // Provided the two filter lists have a shared set of initial primitives, we will be able to interpolate.
    // Note that this means that if either list is empty, interpolation is supported.
    auto numItems = std::min(size(), to.size());
    for (size_t i = 0; i < numItems; ++i)
    {
      const auto *fromOperation = At(i);
      const auto *toOperation = to.At(i);
      if (!!fromOperation != !!toOperation)
      {
        return false;
      }
      if (fromOperation && toOperation && fromOperation->GetType() != toOperation->GetType())
      {
        return false;
      }
    }

    return true;
  }

  FilterOperations FilterOperations::Blend(const FilterOperations &to,
                                           const BlendingContext &context) const noexcept
  {
    if (context.CompositeOperation == CompositeOperation::Add)
    {
      krys_debug_assert(context.Progress == 1.0);

      List<Ref<FilterOperation>> operations;
      operations.reserve(size() + to.size());
      operations.insert(operations.end(), _operations.begin(), _operations.end());
      operations.insert(operations.end(), to._operations.begin(), to._operations.end());

      return FilterOperations {krys::move(operations)};
    }

    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? to : *this;
    }

    auto fromSize = _operations.size();
    auto toSize = to._operations.size();
    auto size = std::max(fromSize, toSize);

    List<Ref<FilterOperation>> operations;
    operations.reserve(size);

    for (size_t i = 0; i < size; ++i)
    {
      RefPtr<FilterOperation> fromOp = i < fromSize ? RefPtr<FilterOperation> {_operations[i]} : nullptr;
      RefPtr<FilterOperation> toOp = i < toSize ? RefPtr<FilterOperation> {to._operations[i]} : nullptr;

      RefPtr<FilterOperation> blendedOp;
      if (toOp)
      {
        blendedOp = toOp->Blend(fromOp.get(), context);
      }
      else if (fromOp)
      {
        blendedOp = fromOp->Blend(nullptr, context, true);
      }

      if (blendedOp)
      {
        operations.push_back(Ref<FilterOperation> {krys::move(blendedOp)});
      }
      else
      {
        RefPtr<FilterOperation> fallback = context.Progress > 0.5 ? toOp : fromOp;
        if (fallback)
        {
          operations.push_back(Ref<FilterOperation> {krys::move(fallback)});
        }
        else
        {
          operations.push_back(PassthroughFilterOperation::Create());
        }
      }
    }

    return FilterOperations {krys::move(operations)};
  }
}
