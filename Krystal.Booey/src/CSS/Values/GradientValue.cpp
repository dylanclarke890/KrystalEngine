#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGradientImage.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::css
{
  GradientValue::~GradientValue() noexcept = default;

  GradientValue::GradientValue(boo::css::Gradient &&gradient) noexcept
      : Value(ValueType::Gradient), _gradient(krys::move(gradient))
  {
  }

  CSSOMString GradientValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _gradient);
  }

  bool GradientValue::Equals(const GradientValue &other) const noexcept
  {
    return _gradient == other._gradient;
  }

  IterationStatus GradientValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _gradient);
  }

  namespace
  {
    template <typename>
    struct StyleImageIsUncacheable;

    template <typename TCSSType>
    KRYS_NODISCARD bool IsStyleImageUncacheable(const TCSSType &value) noexcept
    {
      return StyleImageIsUncacheable<TCSSType> {}(value);
    }

    template <>
    struct StyleImageIsUncacheable<GradientColorInterpolationMethod>
    {
      KRYS_NODISCARD constexpr bool operator()(const auto &) noexcept
      {
        return false;
      }
    };

    template <>
    struct StyleImageIsUncacheable<Color>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return ContainsCurrentColor(value) || ContainsColorSchemeDependentColor(value);
      }
    };

    template <ValueId C>
    struct StyleImageIsUncacheable<KeywordValueConstant<C>>
    {
      KRYS_NODISCARD constexpr bool operator()(const auto &) noexcept
      {
        return false;
      }
    };

    template <UnitEnum TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD constexpr bool operator()(const auto &value) noexcept
      {
        return ConversionToCanonicalUnitRequiresConversionData(value);
      }
    };

    template <NumericRaw TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD constexpr bool operator()(const auto &value) noexcept
      {
        return IsStyleImageUncacheable(value.Unit);
      }
    };

    template <Calc TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return value.ProtectedCalc()->RequiresConversionData();
      }
    };

    template <OptionalLike TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return value && IsStyleImageUncacheable(*value);
      }
    };

    template <TupleLike TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return krys::apply([&](const auto &...x) { return (IsStyleImageUncacheable(x) || ...); }, value);
      }
    };

    template <RangeLike TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return std::ranges::any_of(value,
                                   [](const auto &element) { return IsStyleImageUncacheable(element); });
      }
    };

    template <VariantLike TCSSType>
    struct StyleImageIsUncacheable<TCSSType>
    {
      KRYS_NODISCARD bool operator()(const auto &value) noexcept
      {
        return krys::SwitchOn(value,
                              [](const auto &alternative) { return IsStyleImageUncacheable(alternative); });
      }
    };
  }

  RefPtr<StyleImage> GradientValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    if (_cachedStyleImage)
    {
      return _cachedStyleImage;
    }

    auto styleImage = StyleGradientImage::Create(style::ConvertToStyle(_gradient, state));
    if (!IsStyleImageUncacheable(_gradient))
    {
      _cachedStyleImage = styleImage;
    }

    return styleImage;
  }
}
