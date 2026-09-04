#pragma once

#include "Krystal.Lib/Color/ColorComponents.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Maths.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include <tuple>

namespace Krys
{
  enum class RGBBoundedness
  {
    Bounded,
    Extended
  };

  template <typename, RGBBoundedness>
  struct RGBModel;

  template <typename>
  struct AlphaTraits;

  template <typename>
  struct ColorComponentInfo;

  template <typename>
  struct HSLModel;

  template <typename>
  struct HWBModel;

  template <typename>
  struct LCHModel;

  template <typename>
  struct LabModel;

  template <typename>
  struct OKLCHModel;

  template <typename>
  struct OKLabModel;

  template <typename>
  struct XYZModel;

  // MARK: Resolved/Unresolved Definitions

  template <typename ColorType, typename ColorModel = typename ColorType::Model>
  struct ExposedColorType;

  template <typename ColorType>
  struct ResolvedColorType;

  template <typename ColorType>
  struct UnresolvedColorType;

  template <typename ColorType>
  KRYS_NODISCARD constexpr ResolvedColorType<ColorType> ResolvedColor(ColorType input) noexcept
  {
    return ResolvedColorType<ColorType> {input};
  }

  template <typename ColorType>
  KRYS_NODISCARD constexpr UnresolvedColorType<ColorType> UnresolvedColor(ColorType input) noexcept
  {
    return UnresolvedColorType<ColorType> {input};
  }

  KRYS_NODISCARD constexpr ColorComponents<uint8, 4>
    ResolveColorComponents(const ColorComponents<uint8, 4> &colorComponents) noexcept
  {
    return colorComponents;
  }

  KRYS_NODISCARD constexpr ColorComponents<float, 4>
    ResolveColorComponents(const ColorComponents<float, 4> &colorComponents) noexcept
  {
    return colorComponents.Map([](float component) { return IsNaN(component) ? 0.0f : component; });
  }

  template <typename ColorType>
  struct ResolvedColorType : ExposedColorType<ColorType, typename ColorType::Model>
  {
    using CanonicalType = ColorType;

    // Calling Resolved() or Unresolved() on a type that is already Resolved is a no-op, so we can
    // just return ourselves.
    constexpr auto Resolved() const noexcept
    {
      return *this;
    }

    constexpr auto Unresolved() const noexcept
    {
      return *this;
    }

  private:
    template <typename C>
    friend constexpr ResolvedColorType<C> ResolvedColor(C) noexcept;

    explicit constexpr ResolvedColorType(ColorType color) noexcept
        : ExposedColorType<ColorType, typename ColorType::Model> {resolve(color)}
    {
    }

    template <typename C>
    requires SameType<typename C::ComponentType, float>
    constexpr static C Resolve(C color) noexcept
    {
      auto [c1, c2, c3, alpha] =
        ResolveColorComponents(AsColorComponents(ExposedColorType<C, typename C::Model> {color}));
      return ColorType {c1, c2, c3, alpha};
    }

    template <typename C>
    requires SameType<typename C::ComponentType, uint8>
    constexpr static C Resolve(C color) noexcept
    {
      return color;
    }
  };

  template <typename ColorType>
  struct UnresolvedColorType : ExposedColorType<ColorType, typename ColorType::Model>
  {
    using CanonicalType = ColorType;

    // Calling Unresolved() on a type that is already Unresolved is a no-op, so we can
    // just return ourselves.
    constexpr auto Unresolved() const noexcept
    {
      return *this;
    }

    constexpr bool AnyComponentIsNone() const noexcept
    {
      auto [c1, c2, c3, alpha] = *this;
      return IsNaN(c1) || IsNaN(c2) || IsNaN(c3) || IsNaN(alpha);
    }

  private:
    template <typename C>
    friend constexpr UnresolvedColorType<C> UnresolvedColor(C) noexcept;

    explicit constexpr UnresolvedColorType(ColorType color) noexcept
        : ExposedColorType<ColorType, typename ColorType::Model> {color}
    {
    }
  };

  // MARK: - Color Model support types.

  template <>
  struct AlphaTraits<float>
  {
    constexpr static float transparent = 0.0f;
    constexpr static float opaque = 1.0f;
  };

  template <>
  struct AlphaTraits<uint8>
  {
    constexpr static uint8 transparent = 0;
    constexpr static uint8 opaque = 255;
  };

  // Analogous components categories as defined by CSS Color 4 -
  // https://drafts.csswg.org/css-color-4/#analogous-components
  enum class ColorComponentCategory
  {
    Reds,         // red (RGBModel), x (XYZModel)
    Greens,       // green (RGBModel), y (XYZModel)
    Blues,        // blue (RGBModel), z (XYZModel)
    Lightness,    // lightness (LCHModel, LabModel, OKLCHModel, OKLabModel)
    Colorfulness, // chroma (LCHModel, OKLCHModel), saturation (HSLModel)
    Hue,          // hue (LCHModel, OKLCHModel, HSLModel, HWBModel)
    OpponentA,    // a (LabModel, OKLabModel)
    OpponentB     // b (LabModel, OKLabModel)
  };

  enum class ColorComponentType
  {
    Angle,
    Number
  };

  enum class ColorSpaceCoordinateSystem
  {
    RectangularOrthogonal,
    CylindricalPolar
  };

  template <typename T>
  struct ColorComponentInfo
  {
    T min;
    T max;
    ColorComponentType type;
    Maybe<ColorComponentCategory> category;
  };

  // MARK: - Color Model Definitions

#pragma region HSLModel

  template <>
  struct HSLModel<float>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{0, 360, ColorComponentType::Angle, ColorComponentCategory::Hue},
       {0, Limits<float>::infinity(), ColorComponentType::Number, ColorComponentCategory::Colorfulness},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::Lightness}}};
    constexpr static bool isInvertible = false;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::CylindricalPolar;
  };

  template <typename ColorType>
  struct ExposedColorType<ColorType, HSLModel<typename ColorType::ComponentType>> : ColorType
  {
    using ColorType::alpha;
    using ColorType::hue;
    using ColorType::lightness;
    using ColorType::saturation;
  };

  template <typename T, typename ColorType>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    AsColorComponents(const ExposedColorType<ColorType, HSLModel<T>> &c) noexcept
  {
    return {c.hue, c.saturation, c.lightness, c.alpha};
  }

  template <typename ColorType>
  constexpr bool UsesHSLModel =
    SameType<typename ColorType::Model, HSLModel<typename ColorType::ComponentType>>;

#pragma endregion

#pragma region HWBModel

  template <>
  struct HWBModel<float>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{0, 360, ColorComponentType::Angle, ColorComponentCategory::Hue},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number, Null},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number, Null}}};

    constexpr static bool isInvertible = false;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::CylindricalPolar;
  };

  template <typename ColorType>
  struct ExposedColorType<ColorType, HWBModel<typename ColorType::ComponentType>> : ColorType
  {
    using ColorType::alpha;
    using ColorType::blackness;
    using ColorType::hue;
    using ColorType::whiteness;
  };

  template <typename T, typename ColorType>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    AsColorComponents(const ExposedColorType<ColorType, HWBModel<T>> &c) noexcept
  {
    return {c.hue, c.whiteness, c.blackness, c.alpha};
  }

  template <typename ColorType>
  constexpr bool UsesHWBModel =
    SameType<typename ColorType::Model, HWBModel<typename ColorType::ComponentType>>;

#pragma endregion

#pragma region LabModel

  template <>
  struct LabModel<float>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{0, 100, ColorComponentType::Number, ColorComponentCategory::Lightness},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::OpponentA},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::OpponentB}}};

    constexpr static bool isInvertible = false;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::RectangularOrthogonal;
    // `achromaticEpsilon` is based on the value from color-js and derived from "reference extent" / 100000.
    constexpr static auto achromaticEpsilon = 250.0 / 100000.0;
  };

  template <typename ColorType>
  struct ExposedColorType<ColorType, LabModel<typename ColorType::ComponentType>> : ColorType
  {
    using ColorType::a;
    using ColorType::alpha;
    using ColorType::b;
    using ColorType::lightness;
  };

  template <typename T, typename ColorType>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    AsColorComponents(const ExposedColorType<ColorType, LabModel<T>> &c) noexcept
  {
    return {c.lightness, c.a, c.b, c.alpha};
  }

  template <typename ColorType>
  constexpr bool UsesLabModel =
    SameType<typename ColorType::Model, LabModel<typename ColorType::ComponentType>>;

#pragma endregion

#pragma region LCHModel

  template <>
  struct LCHModel<float>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{0, 100, ColorComponentType::Number, ColorComponentCategory::Lightness},
       {0, Limits<float>::infinity(), ColorComponentType::Number, ColorComponentCategory::Colorfulness},
       {0, 360, ColorComponentType::Angle, ColorComponentCategory::Hue}}};

    constexpr static bool isInvertible = false;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::CylindricalPolar;
  };

  template <typename ColorType>
  struct ExposedColorType<ColorType, LCHModel<typename ColorType::ComponentType>> : ColorType
  {
    using ColorType::alpha;
    using ColorType::chroma;
    using ColorType::hue;
    using ColorType::lightness;
  };

  template <typename T, typename ColorType>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    AsColorComponents(const ExposedColorType<ColorType, LCHModel<T>> &c) noexcept
  {
    return {c.lightness, c.chroma, c.hue, c.alpha};
  }

  template <typename ColorType>
  constexpr bool UsesLCHModel =
    SameType<typename ColorType::Model, LCHModel<typename ColorType::ComponentType>>;

#pragma endregion

#pragma region OKLabModel

  template <>
  struct OKLabModel<float>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{0, 1, ColorComponentType::Number, ColorComponentCategory::Lightness},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::OpponentA},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::OpponentB}}};

    constexpr static bool isInvertible = false;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::RectangularOrthogonal;
    // `achromaticEpsilon` is based on the value from color-js and derived from "reference extent" / 100000.
    constexpr static auto achromaticEpsilon = 0.8 / 100000.0;
  };

  template <typename ColorType>
  struct ExposedColorType<ColorType, OKLabModel<typename ColorType::ComponentType>> : ColorType
  {
    using ColorType::a;
    using ColorType::alpha;
    using ColorType::b;
    using ColorType::lightness;
  };

  template <typename T, typename ColorType>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    AsColorComponents(const ExposedColorType<ColorType, OKLabModel<T>> &c) noexcept
  {
    return {c.lightness, c.a, c.b, c.alpha};
  }

  template <typename ColorType>
  constexpr bool UsesOKLabModel =
    SameType<typename ColorType::Model, OKLabModel<typename ColorType::ComponentType>>;

#pragma endregion

#pragma region OKLCHModel

  template <>
  struct OKLCHModel<float>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{0, 1, ColorComponentType::Number, ColorComponentCategory::Lightness},
       {0, Limits<float>::infinity(), ColorComponentType::Number, ColorComponentCategory::Colorfulness},
       {0, 360, ColorComponentType::Angle, ColorComponentCategory::Hue}}};
    constexpr static bool isInvertible = false;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::CylindricalPolar;
  };

  template <typename ColorType>
  struct ExposedColorType<ColorType, OKLCHModel<typename ColorType::ComponentType>> : ColorType
  {
    using ColorType::alpha;
    using ColorType::chroma;
    using ColorType::hue;
    using ColorType::lightness;
  };

  template <typename T, typename ColorType>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    AsColorComponents(const ExposedColorType<ColorType, OKLCHModel<T>> &c) noexcept
  {
    return {c.lightness, c.chroma, c.hue, c.alpha};
  }

  template <typename ColorType>
  constexpr bool UsesOKLCHModel =
    SameType<typename ColorType::Model, OKLCHModel<typename ColorType::ComponentType>>;

#pragma endregion

#pragma region RGBModel

  template <>
  struct RGBModel<float, RGBBoundedness::Bounded>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{0, 1, ColorComponentType::Number, ColorComponentCategory::Reds},
       {0, 1, ColorComponentType::Number, ColorComponentCategory::Greens},
       {0, 1, ColorComponentType::Number, ColorComponentCategory::Blues}}};

    constexpr static bool isInvertible = true;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::RectangularOrthogonal;
  };

  template <>
  struct RGBModel<uint8, RGBBoundedness::Bounded>
  {
    constexpr static Array<ColorComponentInfo<uint8>, 3> componentInfo {
      {{0, 255, ColorComponentType::Number, ColorComponentCategory::Reds},
       {0, 255, ColorComponentType::Number, ColorComponentCategory::Greens},
       {0, 255, ColorComponentType::Number, ColorComponentCategory::Blues}}};

    constexpr static bool isInvertible = true;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::RectangularOrthogonal;
  };

  template <>
  struct RGBModel<float, RGBBoundedness::Extended>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::Reds},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::Greens},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::Blues}}};

    constexpr static bool isInvertible = false;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::RectangularOrthogonal;
  };

  template <typename ColorType, RGBBoundedness boundedness>
  struct ExposedColorType<ColorType, RGBModel<typename ColorType::ComponentType, boundedness>> : ColorType
  {
    using ColorType::alpha;
    using ColorType::blue;
    using ColorType::green;
    using ColorType::red;
  };

  template <typename T, typename ColorType, RGBBoundedness boundedness>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    AsColorComponents(const ExposedColorType<ColorType, RGBModel<T, boundedness>> &c) noexcept
  {
    return {c.red, c.green, c.blue, c.alpha};
  }

  template <typename ColorType>
  constexpr bool UsesRGBModel =
    SameType<typename ColorType::Model, RGBModel<typename ColorType::ComponentType, RGBBoundedness::Bounded>>
    || SameType<typename ColorType::Model,
                RGBModel<typename ColorType::ComponentType, RGBBoundedness::Extended>>;
#pragma endregion

#pragma region XYZModel

  template <>
  struct XYZModel<float>
  {
    constexpr static Array<ColorComponentInfo<float>, 3> componentInfo {
      {{-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::Reds},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::Greens},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColorComponentType::Number,
        ColorComponentCategory::Blues}}};

    constexpr static bool isInvertible = false;
    constexpr static auto coordinateSystem = ColorSpaceCoordinateSystem::RectangularOrthogonal;
  };

  template <typename ColorType>
  struct ExposedColorType<ColorType, XYZModel<typename ColorType::ComponentType>> : public ColorType
  {
    using ColorType::alpha;
    using ColorType::x;
    using ColorType::y;
    using ColorType::z;
  };

  template <typename T, typename ColorType>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    AsColorComponents(const ExposedColorType<ColorType, XYZModel<T>> &c) noexcept
  {
    return {c.x, c.y, c.z, c.alpha};
  }

  template <typename ColorType>
  constexpr bool UsesXYZModel =
    SameType<typename ColorType::Model, XYZModel<typename ColorType::ComponentType>>;

#pragma endregion

  // get<> overload (along with std::tuple_size and std::tuple_element below) to support destructuring of
  // explicitly Resolved and Unresolved colors.

  template <size_t I, typename ColorType>
  KRYS_NODISCARD constexpr typename ColorType::ComponentType
    get(const ExposedColorType<ColorType> &color) noexcept
  {
    return AsColorComponents(color)[I];
  }
}

namespace std
{
  template <typename ColorType>
  class tuple_size<Krys::ResolvedColorType<ColorType>> : public std::integral_constant<size_t, 4>
  {
  };

  template <size_t I, typename ColorType>
  class tuple_element<I, Krys::ResolvedColorType<ColorType>>
  {
  public:
    using type = typename Krys::ResolvedColorType<ColorType>::ComponentType;
  };

  template <typename ColorType>
  class tuple_size<Krys::UnresolvedColorType<ColorType>> : public std::integral_constant<size_t, 4>
  {
  };

  template <size_t I, typename ColorType>
  class tuple_element<I, Krys::UnresolvedColorType<ColorType>>
  {
  public:
    using type = typename Krys::UnresolvedColorType<ColorType>::ComponentType;
  };
}