#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  namespace css
  {
    struct AppleColorFilter;
    struct AppleColorFilterData;
  }

  namespace style
  {
    // Any <apple-color-filter-function>.
    // (Equivalent of https://drafts.fxtf.org/filter-effects/#typedef-filter-function)
    struct AppleColorFilterValue
    {
      Ref<FilterOperation> Value;

      AppleColorFilterValue(Ref<FilterOperation> value) noexcept : Value {krys::move(value)}
      {
      }

      KRYS_NODISCARD const FilterOperation &Get() const noexcept
      {
        return *Value;
      }
      KRYS_NODISCARD const FilterOperation &Platform() const noexcept
      {
        return *Value;
      }
      KRYS_NODISCARD const FilterOperation *operator->() const noexcept
      {
        return Value.get();
      }

      KRYS_NODISCARD bool operator==(const AppleColorFilterValue &other) const noexcept
      {
        return Value.get() == other.Value.get() || *Value == *other.Value;
      }
    };

    // <apple-color-filter-value-list> = [ <apple-color-filter-function> | <url> ]+
    // (Equivalent of https://drafts.fxtf.org/filter-effects/#typedef-filter-value-list)
    using AppleColorFilterValueList = css::SpaceSeparatedFixedList<AppleColorFilterValue>;

    // <'-apple-color-filter'> = none | <apple-color-filter-value-list>
    // (Equivalent of https://drafts.fxtf.org/filter-effects/#propdef-filter)
    struct AppleColorFilter : css::ListOrNone<AppleColorFilterValueList>
    {
      friend struct Blending<AppleColorFilter>;

      using css::ListOrNone<AppleColorFilterValueList>::ListOrNone;

      KRYS_NODISCARD static const AppleColorFilter &None() noexcept;

      KRYS_NODISCARD bool TransformColor(boo::Color &) const noexcept;
      KRYS_NODISCARD bool InverseTransformColor(boo::Color &) const noexcept;

      template <FilterOperation::Type Type>
      KRYS_NODISCARD bool HasFilterOfType() const noexcept;
    };

    template <FilterOperation::Type type>
    bool AppleColorFilter::HasFilterOfType() const noexcept
    {
      return std::ranges::any_of(*this, [](const auto &op) { return op->GetType() == type; });
    }

    // MARK: - Conversion

    template <>
    struct ToCSS<AppleColorFilterValue>
    {
      KRYS_NODISCARD css::AppleColorFilterData operator()(const AppleColorFilterValue &,
                                                          const RenderStyle &) noexcept;
    };
    template <>
    struct ToStyle<css::AppleColorFilterData>
    {
      KRYS_NODISCARD AppleColorFilterValue operator()(const css::AppleColorFilterData &,
                                                      const BuilderState &) noexcept;
    };

    template <>
    struct ToCSS<AppleColorFilter>
    {
      KRYS_NODISCARD css::AppleColorFilter operator()(const AppleColorFilter &, const RenderStyle &) noexcept;
    };
    template <>
    struct ToStyle<css::AppleColorFilter>
    {
      KRYS_NODISCARD AppleColorFilter operator()(const css::AppleColorFilter &,
                                                 const BuilderState &) noexcept;
    };
    template <>
    struct CSSValueConversion<AppleColorFilter>
    {
      KRYS_NODISCARD AppleColorFilter operator()(BuilderState &, const css::Value &) noexcept;
    };
    template <>
    struct CSSValueCreation<AppleColorFilter>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const AppleColorFilter &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<AppleColorFilter>
    {
      void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                      const AppleColorFilter &) noexcept;
    };

    // MARK: - Blending

    template <>
    struct Blending<AppleColorFilter>
    {
      KRYS_NODISCARD bool CanBlend(const AppleColorFilter &, const AppleColorFilter &,
                                   CompositeOperation) noexcept;
      KRYS_NODISCARD constexpr bool
        RequiresInterpolationForAccumulativeIteration(const AppleColorFilter &,
                                                      const AppleColorFilter &) noexcept
      {
        return true;
      }
      KRYS_NODISCARD AppleColorFilter Blend(const AppleColorFilter &, const AppleColorFilter &,
                                            const BlendingContext &) noexcept;
    };

    // MARK: - Platform

    template <>
    struct ToPlatform<AppleColorFilterValue>
    {
      KRYS_NODISCARD Ref<FilterOperation> operator()(const AppleColorFilterValue &) noexcept;
    };
    template <>
    struct ToPlatform<AppleColorFilter>
    {
      KRYS_NODISCARD FilterOperations operator()(const AppleColorFilter &) noexcept;
    };
  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::AppleColorFilter)
