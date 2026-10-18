#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/ColourComponents.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <tuple>

namespace krys
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
  struct ColourComponentInfo;

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

  template <typename ColourType, typename ColourModel = typename ColourType::Model>
  struct ExposedColourType;

  template <typename ColourType>
  struct ResolvedColourType;

  template <typename ColourType>
  struct UnresolvedColourType;

  template <typename ColourType>
  KRYS_NODISCARD constexpr ResolvedColourType<ColourType> ResolvedColour(ColourType input) noexcept
  {
    return ResolvedColourType<ColourType> {input};
  }

  template <typename ColourType>
  KRYS_NODISCARD constexpr UnresolvedColourType<ColourType> UnresolvedColour(ColourType input) noexcept
  {
    return UnresolvedColourType<ColourType> {input};
  }

  KRYS_NODISCARD constexpr ColourComponents<uint8, 4>
    ResolveColourComponents(const ColourComponents<uint8, 4> &colorComponents) noexcept
  {
    return colorComponents;
  }

  KRYS_NODISCARD constexpr ColourComponents<float, 4>
    ResolveColourComponents(const ColourComponents<float, 4> &colorComponents) noexcept
  {
    return colorComponents.Map([](float component) { return IsNaN(component) ? 0.0f : component; });
  }

  template <typename ColourType>
  struct ResolvedColourType : ExposedColourType<ColourType, typename ColourType::Model>
  {
    using CanonicalType = ColourType;

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
    friend constexpr ResolvedColourType<C> ResolvedColour(C) noexcept;

    explicit constexpr ResolvedColourType(ColourType color) noexcept
        : ExposedColourType<ColourType, typename ColourType::Model> {Resolve(color)}
    {
    }

    template <typename C>
    requires SameType<typename C::ComponentType, float>
    constexpr static C Resolve(C color) noexcept
    {
      auto [c1, c2, c3, alpha] =
        ResolveColourComponents(AsColourComponents(ExposedColourType<C, typename C::Model> {color}));
      return ColourType {c1, c2, c3, alpha};
    }

    template <typename C>
    requires SameType<typename C::ComponentType, uint8>
    constexpr static C Resolve(C color) noexcept
    {
      return color;
    }
  };

  template <typename ColourType>
  struct UnresolvedColourType : ExposedColourType<ColourType, typename ColourType::Model>
  {
    using CanonicalType = ColourType;

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
    friend constexpr UnresolvedColourType<C> UnresolvedColour(C) noexcept;

    explicit constexpr UnresolvedColourType(ColourType color) noexcept
        : ExposedColourType<ColourType, typename ColourType::Model> {color}
    {
    }
  };

  // MARK: - Color Model support types.

  template <>
  struct AlphaTraits<float>
  {
    constexpr static float Transparent = 0.0f;
    constexpr static float Opaque = 1.0f;
  };

  template <>
  struct AlphaTraits<uint8>
  {
    constexpr static uint8 Transparent = 0;
    constexpr static uint8 Opaque = 255;
  };

  // Analogous components categories as defined by CSS Color 4 -
  // https://drafts.csswg.org/css-color-4/#analogous-components
  enum class ColourComponentCategory
  {
    Reds,          // red (RGBModel), x (XYZModel)
    Greens,        // green (RGBModel), y (XYZModel)
    Blues,         // blue (RGBModel), z (XYZModel)
    Lightness,     // lightness (LCHModel, LabModel, OKLCHModel, OKLabModel)
    Colourfulness, // chroma (LCHModel, OKLCHModel), saturation (HSLModel)
    Hue,           // hue (LCHModel, OKLCHModel, HSLModel, HWBModel)
    OpponentA,     // a (LabModel, OKLabModel)
    OpponentB      // b (LabModel, OKLabModel)
  };

  enum class ColourComponentType
  {
    Angle,
    Number
  };

  enum class ColourSpaceCoordinateSystem
  {
    RectangularOrthogonal,
    CylindricalPolar
  };

  template <typename T>
  struct ColourComponentInfo
  {
    T Min;
    T Max;
    ColourComponentType Type;
    Maybe<ColourComponentCategory> Category;
  };

  // MARK: - Color Model Definitions

#pragma region HSLModel

  template <>
  struct HSLModel<float>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{0, 360, ColourComponentType::Angle, ColourComponentCategory::Hue},
       {0, Limits<float>::infinity(), ColourComponentType::Number, ColourComponentCategory::Colourfulness},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::Lightness}}};
    constexpr static bool IsInvertible = false;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::CylindricalPolar;
  };

  template <typename ColourType>
  struct ExposedColourType<ColourType, HSLModel<typename ColourType::ComponentType>> : ColourType
  {
    using ColourType::Alpha;
    using ColourType::Hue;
    using ColourType::Lightness;
    using ColourType::Saturation;
  };

  template <typename T, typename ColourType>
  KRYS_NODISCARD constexpr ColourComponents<T, 4>
    AsColourComponents(const ExposedColourType<ColourType, HSLModel<T>> &c) noexcept
  {
    return {c.Hue, c.Saturation, c.Lightness, c.Alpha};
  }

  template <typename ColourType>
  constexpr bool UsesHSLModel =
    SameType<typename ColourType::Model, HSLModel<typename ColourType::ComponentType>>;

#pragma endregion

#pragma region HWBModel

  template <>
  struct HWBModel<float>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{0, 360, ColourComponentType::Angle, ColourComponentCategory::Hue},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number, null},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number, null}}};

    constexpr static bool IsInvertible = false;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::CylindricalPolar;
  };

  template <typename ColourType>
  struct ExposedColourType<ColourType, HWBModel<typename ColourType::ComponentType>> : ColourType
  {
    using ColourType::Alpha;
    using ColourType::Blackness;
    using ColourType::Hue;
    using ColourType::Whiteness;
  };

  template <typename T, typename ColourType>
  KRYS_NODISCARD constexpr ColourComponents<T, 4>
    AsColourComponents(const ExposedColourType<ColourType, HWBModel<T>> &c) noexcept
  {
    return {c.Hue, c.Whiteness, c.Blackness, c.Alpha};
  }

  template <typename ColourType>
  constexpr bool UsesHWBModel =
    SameType<typename ColourType::Model, HWBModel<typename ColourType::ComponentType>>;

#pragma endregion

#pragma region LabModel

  template <>
  struct LabModel<float>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{0, 100, ColourComponentType::Number, ColourComponentCategory::Lightness},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::OpponentA},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::OpponentB}}};

    constexpr static bool IsInvertible = false;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::RectangularOrthogonal;
    // `achromaticEpsilon` is based on the value from color-js and derived from "reference extent" / 100000.
    constexpr static auto AchromaticEpsilon = 250.0 / 100000.0;
  };

  template <typename ColourType>
  struct ExposedColourType<ColourType, LabModel<typename ColourType::ComponentType>> : ColourType
  {
    using ColourType::A;
    using ColourType::Alpha;
    using ColourType::B;
    using ColourType::Lightness;
  };

  template <typename T, typename ColourType>
  KRYS_NODISCARD constexpr ColourComponents<T, 4>
    AsColourComponents(const ExposedColourType<ColourType, LabModel<T>> &c) noexcept
  {
    return {c.Lightness, c.A, c.B, c.Alpha};
  }

  template <typename ColourType>
  constexpr bool UsesLabModel =
    SameType<typename ColourType::Model, LabModel<typename ColourType::ComponentType>>;

#pragma endregion

#pragma region LCHModel

  template <>
  struct LCHModel<float>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{0, 100, ColourComponentType::Number, ColourComponentCategory::Lightness},
       {0, Limits<float>::infinity(), ColourComponentType::Number, ColourComponentCategory::Colourfulness},
       {0, 360, ColourComponentType::Angle, ColourComponentCategory::Hue}}};

    constexpr static bool IsInvertible = false;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::CylindricalPolar;
  };

  template <typename ColourType>
  struct ExposedColourType<ColourType, LCHModel<typename ColourType::ComponentType>> : ColourType
  {
    using ColourType::Alpha;
    using ColourType::Chroma;
    using ColourType::Hue;
    using ColourType::Lightness;
  };

  template <typename T, typename ColourType>
  KRYS_NODISCARD constexpr ColourComponents<T, 4>
    AsColourComponents(const ExposedColourType<ColourType, LCHModel<T>> &c) noexcept
  {
    return {c.Lightness, c.Chroma, c.Hue, c.Alpha};
  }

  template <typename ColourType>
  constexpr bool UsesLCHModel =
    SameType<typename ColourType::Model, LCHModel<typename ColourType::ComponentType>>;

#pragma endregion

#pragma region OKLabModel

  template <>
  struct OKLabModel<float>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{0, 1, ColourComponentType::Number, ColourComponentCategory::Lightness},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::OpponentA},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::OpponentB}}};

    constexpr static bool IsInvertible = false;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::RectangularOrthogonal;
    // `achromaticEpsilon` is based on the value from color-js and derived from "reference extent" / 100000.
    constexpr static auto AchromaticEpsilon = 0.8 / 100000.0;
  };

  template <typename ColourType>
  struct ExposedColourType<ColourType, OKLabModel<typename ColourType::ComponentType>> : ColourType
  {
    using ColourType::A;
    using ColourType::Alpha;
    using ColourType::B;
    using ColourType::Lightness;
  };

  template <typename T, typename ColourType>
  KRYS_NODISCARD constexpr ColourComponents<T, 4>
    AsColourComponents(const ExposedColourType<ColourType, OKLabModel<T>> &c) noexcept
  {
    return {c.Lightness, c.A, c.B, c.Alpha};
  }

  template <typename ColourType>
  constexpr bool UsesOKLabModel =
    SameType<typename ColourType::Model, OKLabModel<typename ColourType::ComponentType>>;

#pragma endregion

#pragma region OKLCHModel

  template <>
  struct OKLCHModel<float>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{0, 1, ColourComponentType::Number, ColourComponentCategory::Lightness},
       {0, Limits<float>::infinity(), ColourComponentType::Number, ColourComponentCategory::Colourfulness},
       {0, 360, ColourComponentType::Angle, ColourComponentCategory::Hue}}};
    constexpr static bool IsInvertible = false;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::CylindricalPolar;
  };

  template <typename ColourType>
  struct ExposedColourType<ColourType, OKLCHModel<typename ColourType::ComponentType>> : ColourType
  {
    using ColourType::Alpha;
    using ColourType::Chroma;
    using ColourType::Hue;
    using ColourType::Lightness;
  };

  template <typename T, typename ColourType>
  KRYS_NODISCARD constexpr ColourComponents<T, 4>
    AsColourComponents(const ExposedColourType<ColourType, OKLCHModel<T>> &c) noexcept
  {
    return {c.Lightness, c.Chroma, c.Hue, c.Alpha};
  }

  template <typename ColourType>
  constexpr bool UsesOKLCHModel =
    SameType<typename ColourType::Model, OKLCHModel<typename ColourType::ComponentType>>;

#pragma endregion

#pragma region RGBModel

  template <>
  struct RGBModel<float, RGBBoundedness::Bounded>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{0, 1, ColourComponentType::Number, ColourComponentCategory::Reds},
       {0, 1, ColourComponentType::Number, ColourComponentCategory::Greens},
       {0, 1, ColourComponentType::Number, ColourComponentCategory::Blues}}};

    constexpr static bool IsInvertible = true;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::RectangularOrthogonal;
  };

  template <>
  struct RGBModel<uint8, RGBBoundedness::Bounded>
  {
    constexpr static Array<ColourComponentInfo<uint8>, 3> ComponentInfo {
      {{0, 255, ColourComponentType::Number, ColourComponentCategory::Reds},
       {0, 255, ColourComponentType::Number, ColourComponentCategory::Greens},
       {0, 255, ColourComponentType::Number, ColourComponentCategory::Blues}}};

    constexpr static bool IsInvertible = true;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::RectangularOrthogonal;
  };

  template <>
  struct RGBModel<float, RGBBoundedness::Extended>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::Reds},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::Greens},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::Blues}}};

    constexpr static bool IsInvertible = false;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::RectangularOrthogonal;
  };

  template <typename ColourType, RGBBoundedness boundedness>
  struct ExposedColourType<ColourType, RGBModel<typename ColourType::ComponentType, boundedness>> : ColourType
  {
    using ColourType::Alpha;
    using ColourType::Blue;
    using ColourType::Green;
    using ColourType::Red;
  };

  template <typename T, typename ColourType, RGBBoundedness boundedness>
  KRYS_NODISCARD constexpr ColourComponents<T, 4>
    AsColourComponents(const ExposedColourType<ColourType, RGBModel<T, boundedness>> &c) noexcept
  {
    return {c.Red, c.Green, c.Blue, c.Alpha};
  }

  template <typename ColourType>
  constexpr bool UsesRGBModel =
    SameType<typename ColourType::Model,
             RGBModel<typename ColourType::ComponentType, RGBBoundedness::Bounded>>
    || SameType<typename ColourType::Model,
                RGBModel<typename ColourType::ComponentType, RGBBoundedness::Extended>>;
#pragma endregion

#pragma region XYZModel

  template <>
  struct XYZModel<float>
  {
    constexpr static Array<ColourComponentInfo<float>, 3> ComponentInfo {
      {{-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::Reds},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::Greens},
       {-Limits<float>::infinity(), Limits<float>::infinity(), ColourComponentType::Number,
        ColourComponentCategory::Blues}}};

    constexpr static bool IsInvertible = false;
    constexpr static auto CoordinateSystem = ColourSpaceCoordinateSystem::RectangularOrthogonal;
  };

  template <typename ColourType>
  struct ExposedColourType<ColourType, XYZModel<typename ColourType::ComponentType>> : public ColourType
  {
    using ColourType::Alpha;
    using ColourType::X;
    using ColourType::Y;
    using ColourType::Z;
  };

  template <typename T, typename ColourType>
  KRYS_NODISCARD constexpr ColourComponents<T, 4>
    AsColourComponents(const ExposedColourType<ColourType, XYZModel<T>> &c) noexcept
  {
    return {c.X, c.Y, c.Z, c.Alpha};
  }

  template <typename ColourType>
  constexpr bool UsesXYZModel =
    SameType<typename ColourType::Model, XYZModel<typename ColourType::ComponentType>>;

#pragma endregion

  // get<> overload (along with std::tuple_size and std::tuple_element below) to support destructuring of
  // explicitly Resolved and Unresolved colors.

  template <size_t I, typename ColourType>
  KRYS_NODISCARD constexpr typename ColourType::ComponentType
    get(const ExposedColourType<ColourType> &color) noexcept
  {
    return AsColourComponents(color)[I];
  }
}

namespace std
{
  template <typename ColourType>
  class tuple_size<krys::ResolvedColourType<ColourType>> : public std::integral_constant<size_t, 4>
  {
  };

  template <size_t I, typename ColourType>
  class tuple_element<I, krys::ResolvedColourType<ColourType>>
  {
  public:
    using type = typename krys::ResolvedColourType<ColourType>::ComponentType;
  };

  template <typename ColourType>
  class tuple_size<krys::UnresolvedColourType<ColourType>> : public std::integral_constant<size_t, 4>
  {
  };

  template <size_t I, typename ColourType>
  class tuple_element<I, krys::UnresolvedColourType<ColourType>>
  {
  public:
    using type = typename krys::UnresolvedColourType<ColourType>::ComponentType;
  };
}