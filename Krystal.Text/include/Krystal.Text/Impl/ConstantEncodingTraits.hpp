#pragma once

#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/EBCO.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/DecodeResult.hpp"
#include "Krystal.Text/EncodeResult.hpp"
#include "Krystal.Text/Impl/SchemeHandler.hpp"
#include "Krystal.Text/IsBidirectionalEncoding.hpp"
#include "Krystal.Text/IsCodePointsReplaceable.hpp"
#include "Krystal.Text/IsCodeUnitsReplaceable.hpp"
#include "Krystal.Text/IsFullRangeRepresentable.hpp"
#include "Krystal.Text/IsIgnorableErrorHandler.hpp"
#include "Krystal.Text/IsUnicodeEncoding.hpp"
#include "Krystal.Text/State.hpp"
#include <cstddef>
#include <optional>

namespace Krys::Impl
{
  template <typename TSuper, bool = HasMaybeReplacementCodeUnits<typename TSuper::encoding_type>>
  class MaybeReplacementCodeUnitsES
  {
  };

  template <typename TSuper>
  class MaybeReplacementCodeUnitsES<TSuper, true>
  {
  private:
    const TSuper &Super() const noexcept
    {
      return static_cast<const TSuper &>(*this);
    }

  public:
    constexpr auto MaybeReplacementCodeUnits() const noexcept
    {
      using TOriginalCodeUnit = code_unit_t<typename TSuper::encoding_type>;
      using TCodeUnit = typename TSuper::code_unit;

      decltype(auto) maybeOriginal = this->Super().Base().MaybeReplacementCodeUnits();
      if constexpr (SameType<TOriginalCodeUnit, TCodeUnit>)
      {
        return maybeOriginal;
      }
      else
      {
        using TOriginalSpan = Span<const TOriginalCodeUnit>;
        using TTransformedSpan = Span<const TCodeUnit>;
        using TMaybeTransformedSpan = std::optional<TTransformedSpan>;

        if (!maybeOriginal)
        {
          return TMaybeTransformedSpan(std::nullopt);
        }

        decltype(auto) original = *std::forward<decltype(maybeOriginal)>(maybeOriginal);
        TOriginalSpan guaranteedCodeUnitView(original);

        // transform into proper type...
        auto transformedPtr = reinterpret_cast<const TCodeUnit *>(guaranteedCodeUnitView.data());
        auto transformedSize =
          (guaranteedCodeUnitView.size() * sizeof(TOriginalCodeUnit)) / sizeof(const TCodeUnit);

        return TMaybeTransformedSpan(std::in_place, transformedPtr, transformedSize);
      }
    }
  };

  template <typename TSuper, bool = HasMaybeReplacementCodePoints<typename TSuper::encoding_type>>
  class MaybeReplacementCodePointsES
  {
  };

  template <typename TSuper>
  class MaybeReplacementCodePointsES<TSuper, true>
  {
  private:
    const TSuper &Super() const noexcept
    {
      return static_cast<const TSuper &>(*this);
    }

  public:
    constexpr auto MaybeReplacementCodePoints() const noexcept
    {
      using TOriginalCodePoint = code_point_t<typename TSuper::encoding_type>;
      using TCodePoint = typename TSuper::code_point;

      decltype(auto) maybeOriginal = this->Super().Base().MaybeReplacementCodePoints();
      if constexpr (SameType<TOriginalCodePoint, TCodePoint>)
      {
        return maybeOriginal;
      }
      else
      {
        using TOriginalSpan = Span<const TOriginalCodePoint>;
        using TTransformedSpan = Span<const TCodePoint>;
        using TMaybeTransformedSpan = std::optional<TTransformedSpan>;

        if (!maybeOriginal)
        {
          return TMaybeTransformedSpan(std::nullopt);
        }

        decltype(auto) original = *std::forward<decltype(maybeOriginal)>(maybeOriginal);
        TOriginalSpan guaranteedCodePointView(original);

        // transform into proper type...
        auto transformedPtr = reinterpret_cast<const TCodePoint *>(guaranteedCodePointView.data());
        auto transformedSize =
          (guaranteedCodePointView.size() * sizeof(TOriginalCodePoint)) / sizeof(const TCodePoint);

        return TMaybeTransformedSpan(std::in_place, transformedPtr, transformedSize);
      }
    }
  };

  template <typename TSuper, typename TEncoding, typename = void>
  class IsUnicodeEncodingES
  {
  };

  template <typename TSuper, typename TEncoding>
  class IsUnicodeEncodingES<TSuper, TEncoding, enable_if_t<HasIsUnicodeEncoding<TEncoding>>>
  {
  public:
    using IsUnicodeEncoding = IntegralConstant<bool, IsUnicodeEncoding<TEncoding>>;
  };
}
