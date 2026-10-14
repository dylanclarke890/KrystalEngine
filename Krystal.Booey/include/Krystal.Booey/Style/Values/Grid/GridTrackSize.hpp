#pragma once

#include "Krystal.Booey/Style/Values/Grid/GridTrackBreadth.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  struct GridTrackMinMaxParameters
  {
    GridTrackBreadth Min;
    GridTrackBreadth Max;

    KRYS_NODISCARD bool operator==(const GridTrackMinMaxParameters &) const noexcept = default;
  };
  using GridTrackMinMax = css::FunctionNotation<css::ValueId::Minmax, GridTrackMinMaxParameters>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const GridTrackMinMaxParameters &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Min;
    }
    else if constexpr (I == 1)
    {
      return value.Max;
    }
  }

  struct GridTrackFitContentLength : LengthWrapperBase<LengthPercentage<css::NonNegative>>
  {
    using Base::Base;
  };
  using GridTrackFitContent = css::FunctionNotation<css::ValueId::FitContent, GridTrackFitContentLength>;

  // This class represents a <track-size> from the spec. Although there are 3 different types of
  // <track-size> there is always an equivalent minmax() representation that could represent any of
  // them. The only special case is fit-content(argument) which is similar to minmax(auto,
  // max-content) except that the track size is clamped at argument if it is greater than the auto
  // minimum. At the GridTrackSize level we don't need to worry about clamping so we treat that case
  // exactly as auto.
  //
  // We're using a separate attribute to store fit-content argument even though we could directly use
  // _maxTrackBreadth. The reason why we don't do it is because the maxTrackBreadth() call is a hot
  // spot, so adding a conditional statement there (to distinguish between fit-content and any other
  // case) was causing a severe performance drop.

  // <track-size> = <track-breadth> | minmax( <inflexible-breadth> , <track-breadth> ) | fit-content(
  // <length-percentage [0,∞]> ) https://www.w3.org/TR/css-grid-2/#typedef-track-size
  struct GridTrackSize
  {
    using Breadth = GridTrackBreadth;
    using MinMax = GridTrackMinMax;
    using FitContent = GridTrackFitContent;

    GridTrackSize(css::keywords::Auto keyword) noexcept
        : _type(Type::Breadth), _minTrackBreadth(keyword), _maxTrackBreadth(keyword)
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : _type(Type::Breadth), _minTrackBreadth(literal), _maxTrackBreadth(literal)
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
        : _type(Type::Breadth), _minTrackBreadth(literal), _maxTrackBreadth(literal)
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(css::ValueLiteral<css::FlexUnit::Fr> literal) noexcept
        : _type(Type::Breadth), _minTrackBreadth(literal), _maxTrackBreadth(literal)
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(const Breadth &breadth) noexcept
        : _type(Type::Breadth), _minTrackBreadth(breadth), _maxTrackBreadth(breadth)
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(Breadth &&breadth) noexcept
        : _type(Type::Breadth), _minTrackBreadth(breadth), _maxTrackBreadth(krys::move(breadth))
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(const MinMax &minMax) noexcept
        : _type(Type::MinMax), _minTrackBreadth(minMax->Min), _maxTrackBreadth(minMax->Max)
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(MinMax &&minMax) noexcept
        : _type(Type::MinMax), _minTrackBreadth(krys::move(minMax->Min)),
          _maxTrackBreadth(krys::move(minMax->Max))
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(const FitContent &fitContent) noexcept
        : _type(Type::FitContent), _fitContentTrackLength(*fitContent)
    {
      CacheMinMaxTrackBreadthTypes();
    }

    GridTrackSize(FitContent &&fitContent) noexcept
        : _type(Type::FitContent), _fitContentTrackLength(krys::move(*fitContent))
    {
      CacheMinMaxTrackBreadthTypes();
    }

    KRYS_NODISCARD const GridTrackFitContentLength &FitContentTrackLength() const noexcept
    {
      krys_debug_assert(_type == Type::FitContent);
      return _fitContentTrackLength;
    }

    KRYS_NODISCARD const GridTrackBreadth &MinTrackBreadth() const noexcept
    {
      return _minTrackBreadth;
    }
    KRYS_NODISCARD const GridTrackBreadth &MaxTrackBreadth() const noexcept
    {
      return _maxTrackBreadth;
    }

    KRYS_NODISCARD bool IsBreadth() const noexcept
    {
      return _type == Type::Breadth;
    }
    KRYS_NODISCARD bool IsMinMax() const noexcept
    {
      return _type == Type::MinMax;
    }
    KRYS_NODISCARD bool IsFitContent() const noexcept
    {
      return _type == Type::FitContent;
    }

    KRYS_NODISCARD bool IsContentSized() const noexcept
    {
      return _minTrackBreadth.IsContentSized() || _maxTrackBreadth.IsContentSized();
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...) const;

    bool operator==(const GridTrackSize &other) const noexcept
    {
      return _type == other._type && _minTrackBreadth == other._minTrackBreadth
             && _maxTrackBreadth == other._maxTrackBreadth
             && _fitContentTrackLength == other._fitContentTrackLength;
    }

    void CacheMinMaxTrackBreadthTypes() noexcept
    {
      _minTrackBreadthIsAuto = MinTrackBreadth().IsLength() && MinTrackBreadth().Length().IsAuto();
      _minTrackBreadthIsMinContent =
        MinTrackBreadth().IsLength() && MinTrackBreadth().Length().IsMinContent();
      _minTrackBreadthIsMaxContent =
        MinTrackBreadth().IsLength() && MinTrackBreadth().Length().IsMaxContent();
      _maxTrackBreadthIsMaxContent =
        MaxTrackBreadth().IsLength() && MaxTrackBreadth().Length().IsMaxContent();
      _maxTrackBreadthIsMinContent =
        MaxTrackBreadth().IsLength() && MaxTrackBreadth().Length().IsMinContent();
      _maxTrackBreadthIsAuto = MaxTrackBreadth().IsLength() && MaxTrackBreadth().Length().IsAuto();
      _maxTrackBreadthIsFixed = MaxTrackBreadth().IsLength() && MaxTrackBreadth().Length().IsSpecified();

      // These values depend on the above ones so keep them here.
      _minTrackBreadthIsIntrinsic = _minTrackBreadthIsMaxContent || _minTrackBreadthIsMinContent
                                    || _minTrackBreadthIsAuto || IsFitContent();
      _maxTrackBreadthIsIntrinsic = _maxTrackBreadthIsMaxContent || _maxTrackBreadthIsMinContent
                                    || _maxTrackBreadthIsAuto || IsFitContent();
    }

    KRYS_NODISCARD bool HasIntrinsicMinTrackBreadth() const noexcept
    {
      return _minTrackBreadthIsIntrinsic;
    }
    KRYS_NODISCARD bool HasIntrinsicMaxTrackBreadth() const noexcept
    {
      return _maxTrackBreadthIsIntrinsic;
    }
    KRYS_NODISCARD bool HasMinOrMaxContentMinTrackBreadth() const noexcept
    {
      return _minTrackBreadthIsMaxContent || _minTrackBreadthIsMinContent;
    }
    KRYS_NODISCARD bool HasAutoMinTrackBreadth() const noexcept
    {
      return _minTrackBreadthIsAuto;
    }
    KRYS_NODISCARD bool HasAutoMaxTrackBreadth() const noexcept
    {
      return _maxTrackBreadthIsAuto;
    }
    KRYS_NODISCARD bool HasMaxContentMaxTrackBreadth() const noexcept
    {
      return _maxTrackBreadthIsMaxContent;
    }
    KRYS_NODISCARD bool HasMaxContentOrAutoMaxTrackBreadth() const noexcept
    {
      return _maxTrackBreadthIsMaxContent || _maxTrackBreadthIsAuto;
    }
    KRYS_NODISCARD bool HasMinContentMaxTrackBreadth() const noexcept
    {
      return _maxTrackBreadthIsMinContent;
    }
    KRYS_NODISCARD bool HasMinOrMaxContentMaxTrackBreadth() const noexcept
    {
      return _maxTrackBreadthIsMaxContent || _maxTrackBreadthIsMinContent;
    }
    KRYS_NODISCARD bool HasMaxContentMinTrackBreadth() const noexcept
    {
      return _minTrackBreadthIsMaxContent;
    }
    KRYS_NODISCARD bool HasMinContentMinTrackBreadth() const noexcept
    {
      return _minTrackBreadthIsMinContent;
    }
    KRYS_NODISCARD bool HasMaxContentMinTrackBreadthAndMaxContentMaxTrackBreadth() const noexcept
    {
      return _minTrackBreadthIsMaxContent && _maxTrackBreadthIsMaxContent;
    }
    KRYS_NODISCARD bool HasAutoOrMinContentMinTrackBreadthAndIntrinsicMaxTrackBreadth() const noexcept
    {
      return (_minTrackBreadthIsMinContent || _minTrackBreadthIsAuto) && _maxTrackBreadthIsIntrinsic;
    }
    KRYS_NODISCARD bool HasFixedMaxTrackBreadth() const noexcept
    {
      return _maxTrackBreadthIsFixed;
    }

  private:
    friend struct Blending<GridTrackSize>;

    enum class Type : uint8
    {
      Breadth,
      MinMax,
      FitContent
    };
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    Type _type {Type::Breadth};
    GridTrackBreadth _minTrackBreadth {css::keywords::Auto {}};
    GridTrackBreadth _maxTrackBreadth {css::keywords::Auto {}};
    GridTrackFitContentLength _fitContentTrackLength {0_css_px};

    bool _minTrackBreadthIsAuto : 1;
    bool _maxTrackBreadthIsAuto : 1;
    bool _minTrackBreadthIsMaxContent : 1;
    bool _minTrackBreadthIsMinContent : 1;
    bool _maxTrackBreadthIsMaxContent : 1;
    bool _maxTrackBreadthIsMinContent : 1;
    bool _minTrackBreadthIsIntrinsic : 1;
    bool _maxTrackBreadthIsIntrinsic : 1;
    bool _maxTrackBreadthIsFixed : 1;
  };

  template <typename... F>
  decltype(auto) GridTrackSize::SwitchOn(F &&...f) const
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    switch (_type)
    {
      case Type::Breadth:    return visitor(_minTrackBreadth);

      case Type::FitContent: return visitor(GridTrackSize::FitContent {.Parameters = _fitContentTrackLength});

      case Type::MinMax:
        if (_minTrackBreadth.IsAuto() && _maxTrackBreadth.IsFlex())
        {
          return visitor(_maxTrackBreadth.GetFlex());
        }

        return visitor(GridTrackSize::MinMax {.Parameters = {
                                                .Min = _minTrackBreadth,
                                                .Max = _maxTrackBreadth,
                                              }});
    }

    krys_unreachable();
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<GridTrackSize>
  {
    KRYS_NODISCARD GridTrackSize operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<GridTrackSize>
  {
    KRYS_NODISCARD GridTrackSize Blend(const GridTrackSize &, const GridTrackSize &,
                                       const BlendingContext &) noexcept;
  };
}

DEFINE_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::GridTrackMinMaxParameters, 2)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridTrackFitContentLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridTrackSize)
