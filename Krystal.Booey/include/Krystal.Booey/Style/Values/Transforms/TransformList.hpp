#pragma once

#include "Krystal.Booey/Style/Values/Transforms/TransformFunction.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo
{
  class FloatSize;
  class LayoutSize;
  class TransformationMatrix;

  namespace style
  {
    struct Transform;

    // <transform-list> = <transform-function>+
    // https://drafts.csswg.org/css-transforms-1/#typedef-transform-list
    struct TransformList
    {
      using Container = css::SpaceSeparatedFixedList<TransformFunction>;
      using const_iterator = typename Container::const_iterator;
      using const_reverse_iterator = typename Container::const_reverse_iterator;
      using value_type = typename Container::value_type;

      TransformList() noexcept = default;
      TransformList(Container &&value) noexcept : _value {krys::move(value)}
      {
      }

      TransformList(TransformFunction &&transformFunction) noexcept : _value {krys::move(transformFunction)}
      {
      }

      const_iterator begin() const KRYS_LIFETIME_BOUND
      {
        return _value.begin();
      }
      const_iterator end() const KRYS_LIFETIME_BOUND
      {
        return _value.end();
      }
      const_reverse_iterator rbegin() const KRYS_LIFETIME_BOUND
      {
        return _value.rbegin();
      }
      const_reverse_iterator rend() const KRYS_LIFETIME_BOUND
      {
        return _value.rend();
      }

      KRYS_NODISCARD bool isEmpty() const noexcept
      {
        return _value.isEmpty();
      }
      KRYS_NODISCARD size_t size() const noexcept
      {
        return _value.size();
      }
      const TransformFunction &operator[](size_t i) const KRYS_LIFETIME_BOUND
      {
        return _value[i];
      }

      KRYS_NODISCARD bool operator==(const TransformList &) const noexcept = default;

      template <TransformFunctionType>
      KRYS_NODISCARD bool HasTransformOfType() const noexcept;

      void Apply(TransformationMatrix &, const FloatSize &, size_t start = 0) const noexcept;

      // Return true if any of the operation types are 3D operation types (even if the
      // values describe affine transforms)
      KRYS_NODISCARD bool Has3DOperation() const noexcept;
      KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept;
      KRYS_NODISCARD bool AffectedByTransformOrigin() const noexcept;
      KRYS_NODISCARD bool ContainsNonInvertibleMatrix(const LayoutSize &) const noexcept;

      KRYS_NODISCARD TransformFunctionSizeDependencies ComputeSizeDependencies() const noexcept;

    private:
      friend struct Blending<TransformList>;
      friend struct Transform;

      KRYS_NODISCARD bool IsInvertible(const LayoutSize &) const noexcept;

      Container _value;
    };

    template <TransformFunctionType operationType>
    bool TransformList::HasTransformOfType() const noexcept
    {
      return std::ranges::any_of(_value, [](auto &op) { return op->GetType() == operationType; });
    }

    template <>
    struct Blending<TransformList>
    {
      KRYS_NODISCARD bool CanBlend(const TransformList &, const TransformList &, CompositeOperation) noexcept;
      KRYS_NODISCARD TransformList Blend(const TransformList &, const TransformList &,
                                         const BlendingContext &) noexcept;
    };

  }
}

DEFINE_SPACE_SEPARATED_RANGE_LIKE_CONFORMANCE(krys::boo::style::TransformList)
