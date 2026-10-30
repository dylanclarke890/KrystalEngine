#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo::style
{
  enum class PaintType : uint8
  {
    Fill,
    Stroke,
    Markers
  };

  // <'paint-order'> = normal | [ fill || stroke || markers ]
  // https://svgwg.org/svg2-draft/painting.html#PaintOrderProperty
  // NOTE: A `css::SpaceSeparatedEnumSet` cannot be used here as the order of the
  // values is relevant to the interpretation and serialization.
  struct SVGPaintOrder
  {
    using value_type = std::span<const PaintType, 3>::value_type;
    using iterator = std::span<const PaintType, 3>::iterator;

    constexpr SVGPaintOrder(css::keywords::Normal) noexcept
    {
    }
    constexpr SVGPaintOrder(css::keywords::Fill) noexcept : _type {Type::FillStrokeMarkers}
    {
    }
    constexpr SVGPaintOrder(css::keywords::Fill, css::keywords::Markers) noexcept
        : _type {Type::FillMarkersStroke}
    {
    }
    constexpr SVGPaintOrder(css::keywords::Stroke) noexcept : _type {Type::StrokeFillMarkers}
    {
    }
    constexpr SVGPaintOrder(css::keywords::Stroke, css::keywords::Markers) noexcept
        : _type {Type::StrokeMarkersFill}
    {
    }
    constexpr SVGPaintOrder(css::keywords::Markers) noexcept : _type {Type::MarkersFillStroke}
    {
    }
    constexpr SVGPaintOrder(css::keywords::Markers, css::keywords::Stroke) noexcept
        : _type {Type::MarkersStrokeFill}
    {
    }

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return _type == Type::Normal;
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_type)
      {
        case Type::Normal:            return visitor(css::keywords::Normal {});
        case Type::FillStrokeMarkers: return visitor(css::keywords::Fill {});
        case Type::FillMarkersStroke:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Fill {}, css::keywords::Markers {}});
        case Type::StrokeFillMarkers: return visitor(css::keywords::Stroke {});
        case Type::StrokeMarkersFill:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Stroke {}, css::keywords::Markers {}});
        case Type::MarkersFillStroke: return visitor(css::keywords::Markers {});
        case Type::MarkersStrokeFill:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Markers {}, css::keywords::Stroke {}});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD std::span<const PaintType, 3> PaintTypes() const noexcept;

    KRYS_NODISCARD iterator begin() const noexcept
    {
      return PaintTypes().begin();
    }
    KRYS_NODISCARD iterator end() const noexcept
    {
      return PaintTypes().end();
    }

    KRYS_NODISCARD constexpr bool operator==(const SVGPaintOrder &) const noexcept = default;

    // NOTE: The Type is exposed only to allow efficient storage using a bitfield.
    enum class Type : uint8
    {
      Normal,
      FillStrokeMarkers,
      FillMarkersStroke,
      StrokeFillMarkers,
      StrokeMarkersFill,
      MarkersFillStroke,
      MarkersStrokeFill,
    };
    constexpr SVGPaintOrder(Type type) noexcept : _type {type}
    {
    }
    KRYS_NODISCARD constexpr Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD constexpr static SVGPaintOrder FromRaw(std::underlying_type_t<Type> rawValue) noexcept
    {
      return SVGPaintOrder {static_cast<Type>(rawValue)};
    }
    KRYS_NODISCARD constexpr uint8 ToRaw() const noexcept
    {
      return ToUnderlying(_type);
    }

  private:
    Type _type {Type::Normal};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGPaintOrder>
  {
    KRYS_NODISCARD SVGPaintOrder operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGPaintOrder)
