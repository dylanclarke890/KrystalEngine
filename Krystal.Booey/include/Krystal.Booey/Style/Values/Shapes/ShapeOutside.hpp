#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"
#include "Krystal.Booey/Style/Values/Shapes/BasicShape.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'shape-outside'> = none | [ <basic-shape> || <shape-box> ] | <image>
  // https://www.w3.org/TR/css-shapes/#propdef-shape-outside
  struct ShapeOutside
  {
    using Shape = BasicShape;
    using ShapeBox = CSSBoxType;

    struct ShapeAndShapeBox
    {
      BasicShape Shape;
      ShapeBox Box;

      KRYS_NODISCARD bool operator==(const ShapeAndShapeBox &) const noexcept = default;
    };
    struct Image
    {
      ImageWrapper Value;

      KRYS_NODISCARD bool IsValid() const noexcept;

      KRYS_NODISCARD bool operator==(const Image &) const noexcept = default;
    };

    ShapeOutside(css::keywords::None) noexcept
    {
    }
    ShapeOutside(Shape &&value) noexcept : _value {Value::Create(krys::move(value))}
    {
    }
    ShapeOutside(ShapeBox &&value) noexcept : _value {Value::Create(krys::move(value))}
    {
    }
    ShapeOutside(ShapeAndShapeBox &&value) noexcept : _value {Value::Create(krys::move(value))}
    {
    }
    ShapeOutside(Image &&value) noexcept : _value {Value::Create(krys::move(value))}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_value;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }

      return krys::SwitchOn(
        _value->Contents, [&](const Shape &shape) { return visitor(shape); },
        [&](const ShapeBox &box) { return visitor(box); }, [&](const ShapeAndShapeBox &shapeAndShapeBox)
        { return visitor(shapeAndShapeBox); }, [&](const Image &image) { return visitor(image); });
    }

    const BasicShape *GetShape() const
    {
      RefPtr value = _value;
      return value ? value->GetShape() : nullptr;
    }
    KRYS_NODISCARD CSSBoxType EffectiveCSSBox() const noexcept
    {
      RefPtr value = _value;
      return value ? value->EffectiveCSSBox() : CSSBoxType::BoxMissing;
    }
    KRYS_NODISCARD RefPtr<StyleImage> GetImage() const noexcept
    {
      RefPtr value = _value;
      return value ? value->GetImage() : nullptr;
    }

    bool operator==(const ShapeOutside &other) const noexcept
    {
      return ArePointingToEqualData(_value, other._value);
    }

  private:
    friend struct Blending<ShapeOutside>;

    class Value : public RefCounted<Value>
    {
    public:
      using Kind = Variant<Shape, ShapeBox, ShapeAndShapeBox, Image>;

      KRYS_NODISCARD static Ref<Value> Create(Kind &&value) noexcept
      {
        return AdoptRef(*new Value(krys::move(value)));
      }

      explicit Value(Kind &&value) noexcept : Contents {krys::move(value)}
      {
      }

      inline const BasicShape *GetShape() const;
      KRYS_NODISCARD inline CSSBoxType EffectiveCSSBox() const noexcept;
      KRYS_NODISCARD inline RefPtr<StyleImage> GetImage() const noexcept;

      bool operator==(const Value &other) const noexcept
      {
        return Contents == other.Contents;
      }

      Kind Contents;
    };

    RefPtr<Value> _value {};
  };

  DEFINE_TYPE_WRAPPER_GET(ShapeOutside::Image, Value);

  template <size_t I>
  const auto &get(const ShapeOutside::ShapeAndShapeBox &value)
  {
    if constexpr (!I)
    {
      return value.Shape;
    }
    else if constexpr (I == 1)
    {
      return value.Box;
    }
  }

  inline const BasicShape *ShapeOutside::Value::GetShape() const
  {
    return krys::SwitchOn(
      Contents, [](const ShapeOutside::Shape &shape) -> const BasicShape * { return &shape; },
      [](const ShapeOutside::ShapeBox &) -> const BasicShape * { return nullptr; },
      [](const ShapeOutside::ShapeAndShapeBox &shapeAndShapeBox) -> const BasicShape *
      { return &shapeAndShapeBox.Shape; },
      [](const ShapeOutside::Image &) -> const BasicShape * { return nullptr; });
  }

  inline RefPtr<StyleImage> ShapeOutside::Value::GetImage() const noexcept
  {
    return krys::SwitchOn(
      Contents, [](const ShapeOutside::Shape &) -> RefPtr<StyleImage> { return nullptr; },
      [](const ShapeOutside::ShapeBox &) -> RefPtr<StyleImage> { return nullptr; },
      [](const ShapeOutside::ShapeAndShapeBox &) -> RefPtr<StyleImage> { return nullptr; },
      [](const ShapeOutside::Image &image) -> RefPtr<StyleImage> { return image.Value.Value; });
  }

  CSSBoxType ShapeOutside::Value::EffectiveCSSBox() const noexcept
  {
    return krys::SwitchOn(
      Contents, [](const ShapeOutside::Shape &) { return CSSBoxType::MarginBox; },
      [](const ShapeOutside::ShapeBox &box)
      { return box == CSSBoxType::BoxMissing ? CSSBoxType::MarginBox : box; },
      [](const ShapeOutside::ShapeAndShapeBox &shapeAndShapeBox)
      {
        return shapeAndShapeBox.Box == CSSBoxType::BoxMissing ? CSSBoxType::MarginBox : shapeAndShapeBox.Box;
      },
      [](const ShapeOutside::Image &) { return CSSBoxType::ContentBox; });
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ShapeOutside>
  {
    KRYS_NODISCARD ShapeOutside operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ShapeOutside>
  {
    KRYS_NODISCARD bool CanBlend(const ShapeOutside &, const ShapeOutside &) noexcept;
    KRYS_NODISCARD ShapeOutside Blend(const ShapeOutside &, const ShapeOutside &,
                                      const BlendingContext &) noexcept;
  };
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ShapeOutside::ShapeAndShapeBox, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::ShapeOutside::Image);
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ShapeOutside)
