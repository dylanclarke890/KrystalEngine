#pragma once

#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRangeName.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-animation-range-[start|end]> = [ normal | <length-percentage> | <timeline-range-name>
  //   <length-percentage>? ]
  // https://drafts.csswg.org/scroll-animations-1/#propdef-animation-range-start
  // https://drafts.csswg.org/scroll-animations-1/#propdef-animation-range-end

  enum class SingleAnimationRangeType : bool
  {
    Start,
    End
  };

  struct SingleAnimationRangeLength : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;

    KRYS_NODISCARD static SingleAnimationRangeLength DefaultValue(SingleAnimationRangeType) noexcept;
    KRYS_NODISCARD bool IsDefault(SingleAnimationRangeType) const noexcept;
  };

  template <SingleAnimationRangeType type>
  struct SingleAnimationRangeEdge
  {
    using Base = SingleAnimationRangeEdge<type>;
    using Name = SingleAnimationRangeName;
    using Offset = SingleAnimationRangeLength;

    SingleAnimationRangeEdge(Offset &&offset) noexcept
        : SingleAnimationRangeEdge {Name::Omitted, krys::move(offset)}
    {
    }
    SingleAnimationRangeEdge(css::keywords::Normal) noexcept
        : SingleAnimationRangeEdge {Name::Normal, Maybe<Offset> {}}
    {
    }
    SingleAnimationRangeEdge(css::keywords::Cover, Maybe<Offset> &&offset = std::nullopt) noexcept
        : SingleAnimationRangeEdge {Name::Cover, krys::move(offset)}
    {
    }
    SingleAnimationRangeEdge(css::keywords::Contain, Maybe<Offset> &&offset = std::nullopt) noexcept
        : SingleAnimationRangeEdge {Name::Contain, krys::move(offset)}
    {
    }
    SingleAnimationRangeEdge(css::keywords::Entry, Maybe<Offset> &&offset = std::nullopt) noexcept
        : SingleAnimationRangeEdge {Name::Entry, krys::move(offset)}
    {
    }
    SingleAnimationRangeEdge(css::keywords::Exit, Maybe<Offset> &&offset = std::nullopt) noexcept
        : SingleAnimationRangeEdge {Name::Exit, krys::move(offset)}
    {
    }
    SingleAnimationRangeEdge(css::keywords::EntryCrossing, Maybe<Offset> &&offset = std::nullopt) noexcept
        : SingleAnimationRangeEdge {Name::EntryCrossing, krys::move(offset)}
    {
    }
    SingleAnimationRangeEdge(css::keywords::ExitCrossing, Maybe<Offset> &&offset = std::nullopt) noexcept
        : SingleAnimationRangeEdge {Name::ExitCrossing, krys::move(offset)}
    {
    }

    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return _name == Name::Normal;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      auto visitPredefinedNamedRange = [&](auto keyword)
      {
        if (_offset.IsDefault(type))
        {
          return visitor(keyword);
        }
        return visitor(css::SpaceSeparatedTuple {keyword, _offset});
      };

      switch (_name)
      {
        case Name::Normal:        return visitor(css::keywords::Normal {});
        case Name::Omitted:       return visitor(_offset);
        case Name::Cover:         return visitPredefinedNamedRange(css::keywords::Cover {});
        case Name::Contain:       return visitPredefinedNamedRange(css::keywords::Contain {});
        case Name::Entry:         return visitPredefinedNamedRange(css::keywords::Entry {});
        case Name::Exit:          return visitPredefinedNamedRange(css::keywords::Exit {});
        case Name::EntryCrossing: return visitPredefinedNamedRange(css::keywords::EntryCrossing {});
        case Name::ExitCrossing:  return visitPredefinedNamedRange(css::keywords::ExitCrossing {});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD Name GetName() const noexcept
    {
      return _name;
    }
    KRYS_NODISCARD const Offset &GetOffset() const noexcept
    {
      return _offset;
    }

    KRYS_NODISCARD bool HasDefaultOffset() const noexcept
    {
      return _offset.IsDefault(type);
    }

    KRYS_NODISCARD bool operator==(const SingleAnimationRangeEdge<type> &) const noexcept = default;

  protected:
    SingleAnimationRangeEdge(Name name, Offset &&offset) noexcept : _name {name}, _offset {krys::move(offset)}
    {
    }

    SingleAnimationRangeEdge(Name name, Maybe<Offset> &&offset) noexcept
        : _name {name}, _offset {offset ? *offset : Offset::DefaultValue(type)}
    {
    }

    Name _name {Name::Normal};
    Offset _offset;
  };

  struct SingleAnimationRangeStart : SingleAnimationRangeEdge<SingleAnimationRangeType::Start>
  {
    using Base::Base;

    // FIXME: `toTimelineRangeValue()` comes with the Typed OM (`CSSNumericValue`) and Web Animations.
  };

  struct SingleAnimationRangeEnd : SingleAnimationRangeEdge<SingleAnimationRangeType::End>
  {
    using Base::Base;

    // FIXME: `toTimelineRangeValue()` comes with the Typed OM (`CSSNumericValue`) and Web Animations.
  };

  struct SingleAnimationRange
  {
    SingleAnimationRangeStart Start;
    SingleAnimationRangeEnd End;

    KRYS_NODISCARD bool operator==(const SingleAnimationRange &) const noexcept = default;

    KRYS_NODISCARD static SingleAnimationRange DefaultForScrollTimeline() noexcept;
    KRYS_NODISCARD static SingleAnimationRange DefaultForViewTimeline() noexcept;

    KRYS_NODISCARD bool IsDefault() const noexcept
    {
      return Start.IsNormal() && End.IsNormal();
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SingleAnimationRangeStart>
  {
    KRYS_NODISCARD SingleAnimationRangeStart operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueConversion<SingleAnimationRangeEnd>
  {
    KRYS_NODISCARD SingleAnimationRangeEnd operator()(BuilderState &, const css::Value &) noexcept;
  };

  template <>
  struct DeprecatedCSSValueConversion<SingleAnimationRangeStart>
  {
    KRYS_NODISCARD Maybe<SingleAnimationRangeStart> operator()(const RefPtr<dom::Element> &,
                                                               const css::Value &) noexcept;
  };
  template <>
  struct DeprecatedCSSValueConversion<SingleAnimationRangeEnd>
  {
    KRYS_NODISCARD Maybe<SingleAnimationRangeEnd> operator()(const RefPtr<dom::Element> &,
                                                             const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleAnimationRangeLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleAnimationRangeStart)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleAnimationRangeEnd)
