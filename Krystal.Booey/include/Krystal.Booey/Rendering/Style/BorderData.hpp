#pragma once

#include "Krystal.Booey/Core/RectCorners.hpp"
#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Rendering/Style/BorderValue.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBorderImageData.hpp"
#include "Krystal.Booey/Style/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/Style/Values/Borders/CornerShapeValue.hpp"
#include "Krystal.Core/Types/DataRef.hpp"

namespace krys::boo
{
  using namespace css::Literals;

  struct BorderData
  {
    using RadiiType = style::BorderRadius;

    BorderData() noexcept;

    KRYS_NODISCARD bool HasBorder() const noexcept
    {
      return Edges.AnyOf([](const auto &edge) { return edge.NonZero(); });
    }

    KRYS_NODISCARD bool HasVisibleBorder() const noexcept
    {
      return Edges.AnyOf([](const auto &edge) { return edge.IsVisible(); });
    }

    KRYS_NODISCARD bool HasBorderImage() const noexcept
    {
      return !BorderImage->BorderImage.Source.IsNone();
    }

    KRYS_NODISCARD bool HasBorderRadius() const noexcept
    {
      return Radii.AnyOf([](const auto &corner) { return !style::IsValueKnownEmpty(corner); });
    }

    template <BoxSide side>
    KRYS_NODISCARD style::LineWidth BorderEdgeWidth() const noexcept
    {
      if (!Edges[side].HasVisibleStyle())
      {
        return 0_css_px;
      }
      if (BorderImage->BorderImage.Width.OverridesBorderWidths())
      {
        if (auto fixedBorderWidthValue = BorderImage->BorderImage.Width.Values[side].TryFixed())
        {
          return style::LineWidth {fixedBorderWidthValue->UnresolvedValue()};
        }
      }
      return Edges[side].Width;
    }

    KRYS_NODISCARD style::LineWidth BorderLeftWidth() const noexcept
    {
      return BorderEdgeWidth<BoxSide::Left>();
    }
    KRYS_NODISCARD style::LineWidth BorderRightWidth() const noexcept
    {
      return BorderEdgeWidth<BoxSide::Right>();
    }
    KRYS_NODISCARD style::LineWidth BorderTopWidth() const noexcept
    {
      return BorderEdgeWidth<BoxSide::Top>();
    }
    KRYS_NODISCARD style::LineWidth BorderBottomWidth() const noexcept
    {
      return BorderEdgeWidth<BoxSide::Bottom>();
    }

    KRYS_NODISCARD style::LineWidthBox BorderWidth() const noexcept
    {
      return {BorderTopWidth(), BorderRightWidth(), BorderBottomWidth(), BorderLeftWidth()};
    }

    // `BorderEdgesView` provides a RectEdges-like interface for efficiently working with
    // the values stored in BorderValue by edge. This allows `RenderStyle` code generation
    // to work as if the `border-{edge}-*`properties were stored in a RectEdges, while
    // instead storing them grouped together by edge in BorderValue.
    //
    // FIXME: Currently this is only implemented for `border-{edge}-color` and `border-{edge}-style`,
    // due to `border-{edge}-width` needing to return the computed value from BorderEdgeWidth() from
    // its getter.
    template <bool isConst, template <BoxSide> typename Accessor, typename GetterType,
              typename SetterType = GetterType>
    struct BorderEdgesView
    {
      KRYS_NODISCARD GetterType Top() const noexcept
      {
        return Accessor<BoxSide::Top>::Get(Data);
      }
      KRYS_NODISCARD GetterType Right() const noexcept
      {
        return Accessor<BoxSide::Right>::Get(Data);
      }
      KRYS_NODISCARD GetterType Bottom() const noexcept
      {
        return Accessor<BoxSide::Bottom>::Get(Data);
      }
      KRYS_NODISCARD GetterType Left() const noexcept
      {
        return Accessor<BoxSide::Left>::Get(Data);
      }

      void SetTop(SetterType value) noexcept
      requires(!isConst)
      {
        Accessor<BoxSide::Top>::Set(Data, std::forward<SetterType>(value));
      }
      void SetRight(SetterType value) noexcept
      requires(!isConst)
      {
        Accessor<BoxSide::Right>::Set(Data, std::forward<SetterType>(value));
      }
      void SetBottom(SetterType value) noexcept
      requires(!isConst)
      {
        Accessor<BoxSide::Bottom>::Set(Data, std::forward<SetterType>(value));
      }
      void SetLeft(SetterType value) noexcept
      requires(!isConst)
      {
        Accessor<BoxSide::Left>::Set(Data, std::forward<SetterType>(value));
      }

      std::conditional_t<isConst, const BorderData &, BorderData &> Data;
    };

    template <BoxSide side>
    struct ColorAccessor
    {
      KRYS_NODISCARD static const style::Color &Get(const BorderData &data) noexcept
      {
        return data.Edges[side].Color;
      }
      static void Set(BorderData &data, style::Color &&color) noexcept
      {
        data.Edges[side].Color = krys::move(color);
      }
    };
    using BorderColorsView = BorderEdgesView<false, ColorAccessor, const style::Color &, style::Color &&>;
    using BorderColorsConstView = BorderEdgesView<true, ColorAccessor, const style::Color &, style::Color &&>;
    KRYS_NODISCARD BorderColorsView Colors() noexcept
    {
      return {.Data = *this};
    }
    KRYS_NODISCARD BorderColorsConstView Colors() const noexcept
    {
      return {.Data = *this};
    }

    template <BoxSide side>
    struct StyleAccessor
    {
      KRYS_NODISCARD static uint8 Get(const BorderData &data) noexcept
      {
        return data.Edges[side].Style;
      }
      static void Set(BorderData &data, uint8 style) noexcept
      {
        data.Edges[side].Style = style;
      }
    };
    using BorderStylesView = BorderEdgesView<false, StyleAccessor, uint8>;
    using BorderStylesConstView = BorderEdgesView<true, StyleAccessor, uint8>;
    KRYS_NODISCARD BorderStylesView Styles() noexcept
    {
      return {.Data = *this};
    }
    KRYS_NODISCARD BorderStylesConstView Styles() const noexcept
    {
      return {.Data = *this};
    }

    KRYS_NODISCARD BorderValue &Left() noexcept
    {
      return Edges.Left();
    }
    KRYS_NODISCARD BorderValue &Right() noexcept
    {
      return Edges.Right();
    }
    KRYS_NODISCARD BorderValue &Top() noexcept
    {
      return Edges.Top();
    }
    KRYS_NODISCARD BorderValue &Bottom() noexcept
    {
      return Edges.Bottom();
    }

    KRYS_NODISCARD const BorderValue &Left() const noexcept
    {
      return Edges.Left();
    }
    KRYS_NODISCARD const BorderValue &Right() const noexcept
    {
      return Edges.Right();
    }
    KRYS_NODISCARD const BorderValue &Top() const noexcept
    {
      return Edges.Top();
    }
    KRYS_NODISCARD const BorderValue &Bottom() const noexcept
    {
      return Edges.Bottom();
    }

    KRYS_NODISCARD style::BorderRadiusValue &TopLeftRadius() noexcept
    {
      return Radii.TopLeft();
    }
    KRYS_NODISCARD style::BorderRadiusValue &TopRightRadius() noexcept
    {
      return Radii.TopRight();
    }
    KRYS_NODISCARD style::BorderRadiusValue &BottomLeftRadius() noexcept
    {
      return Radii.BottomLeft();
    }
    KRYS_NODISCARD style::BorderRadiusValue &BottomRightRadius() noexcept
    {
      return Radii.BottomRight();
    }

    KRYS_NODISCARD const style::BorderRadiusValue &TopLeftRadius() const noexcept
    {
      return Radii.TopLeft();
    }
    KRYS_NODISCARD const style::BorderRadiusValue &TopRightRadius() const noexcept
    {
      return Radii.TopRight();
    }
    KRYS_NODISCARD const style::BorderRadiusValue &BottomLeftRadius() const noexcept
    {
      return Radii.BottomLeft();
    }
    KRYS_NODISCARD const style::BorderRadiusValue &BottomRightRadius() const noexcept
    {
      return Radii.BottomRight();
    }

    KRYS_NODISCARD const style::CornerShapeValue &TopLeftCornerShape() const noexcept
    {
      return CornerShapes.TopLeft();
    }
    KRYS_NODISCARD const style::CornerShapeValue &TopRightCornerShape() const noexcept
    {
      return CornerShapes.TopRight();
    }
    KRYS_NODISCARD const style::CornerShapeValue &BottomLeftCornerShape() const noexcept
    {
      return CornerShapes.BottomLeft();
    }
    KRYS_NODISCARD const style::CornerShapeValue &BottomRightCornerShape() const noexcept
    {
      return CornerShapes.BottomRight();
    }

    KRYS_NODISCARD bool ContainsCurrentColor() const noexcept;

    KRYS_NODISCARD bool operator==(const BorderData &) const noexcept = default;

    RectEdges<BorderValue> Edges;
    style::BorderRadius Radii {style::BorderRadiusValue {0_css_px, 0_css_px}};
    style::CornerShape CornerShapes {style::CornerShapeValue(css::keywords::Round {})};
    DataRef<StyleBorderImageData> BorderImage;
  };
}
