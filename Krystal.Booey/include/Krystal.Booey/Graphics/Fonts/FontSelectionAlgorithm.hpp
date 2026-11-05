#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <algorithm>
#include <compare>
#include <limits>

namespace krys::boo
{
  /// @brief Unclamped, unchecked, signed fixed-point number representing a value used for font variations.
  /// Sixteen bits in total, one sign bit, two fractional bits, smallest positive value is 0.25, maximum value
  /// is 8191.75, and minimum value is -8192.
  class FontSelectionValue
  {
  public:
    using BackingType = int16;

  private:
    enum class RawTag
    {
      RawTag
    };

    constexpr static int FractionalEntropy = 4;
    BackingType _backing {0};

  public:
    constexpr FontSelectionValue() noexcept = default;

    explicit constexpr FontSelectionValue(BackingType x) noexcept : _backing(x)
    {
    }

    /// @brief Explicit because it won't work correctly for values outside the representable range.
    explicit constexpr FontSelectionValue(int x) noexcept
        : _backing(static_cast<BackingType>(x * FractionalEntropy))
    {
      // FIXME: Should we assert the passed in value was in range?
    }

    /// @brief Explicit because it won't work correctly for values outside the representable range and because
    /// precision can be lost.
    explicit constexpr FontSelectionValue(float x) noexcept
        : _backing(static_cast<BackingType>(x * FractionalEntropy))
    {
      // FIXME: Should we assert the passed in value was in range?
    }

    /// @brief Precision can be lost, but value will be clamped to the representable range.
    KRYS_NODISCARD constexpr static FontSelectionValue ClampFloat(float value) noexcept
    {
      return FontSelectionValue {std::max<float>(MinimumValue(), std::min<float>(value, MaximumValue()))};
    }

    /// @brief Since floats have 23 mantissa bits, every value can be represented losslessly.
    constexpr operator float() const noexcept
    {
      return _backing / static_cast<float>(FractionalEntropy);
    }

    KRYS_NODISCARD constexpr static FontSelectionValue MaximumValue() noexcept
    {
      return {std::numeric_limits<BackingType>::max(), RawTag::RawTag};
    }

    KRYS_NODISCARD constexpr static FontSelectionValue MinimumValue() noexcept
    {
      return {std::numeric_limits<BackingType>::min(), RawTag::RawTag};
    }

    KRYS_NODISCARD friend constexpr FontSelectionValue operator+(FontSelectionValue a,
                                                                 FontSelectionValue b) noexcept
    {
      return {a._backing + b._backing, RawTag::RawTag};
    }

    KRYS_NODISCARD friend constexpr FontSelectionValue operator-(FontSelectionValue a,
                                                                 FontSelectionValue b) noexcept
    {
      return {a._backing - b._backing, RawTag::RawTag};
    }

    KRYS_NODISCARD friend constexpr FontSelectionValue operator*(FontSelectionValue a,
                                                                 FontSelectionValue b) noexcept
    {
      return {a._backing * b._backing / FractionalEntropy, RawTag::RawTag};
    }

    KRYS_NODISCARD friend constexpr FontSelectionValue operator/(FontSelectionValue a,
                                                                 FontSelectionValue b) noexcept
    {
      return {a._backing * FractionalEntropy / b._backing, RawTag::RawTag};
    }

    KRYS_NODISCARD friend constexpr FontSelectionValue operator-(FontSelectionValue value) noexcept
    {
      return {-value._backing, RawTag::RawTag};
    }

    KRYS_NODISCARD friend constexpr auto operator<=>(FontSelectionValue,
                                                     FontSelectionValue) noexcept = default;
    KRYS_NODISCARD friend constexpr bool operator==(FontSelectionValue,
                                                    FontSelectionValue) noexcept = default;

    KRYS_NODISCARD constexpr BackingType RawValue() const noexcept
    {
      return _backing;
    }

  private:
    constexpr FontSelectionValue(int rawValue, RawTag) noexcept : _backing(static_cast<BackingType>(rawValue))
    {
    }
  };

  KRYS_NODISCARD constexpr FontSelectionValue ItalicThreshold() noexcept
  {
    return FontSelectionValue {14};
  }

  KRYS_NODISCARD constexpr bool IsItalic(Maybe<FontSelectionValue> slope) noexcept
  {
    return slope && slope.value() >= ItalicThreshold();
  }

  KRYS_NODISCARD constexpr FontSelectionValue NormalItalicValue() noexcept
  {
    return FontSelectionValue {0};
  }

  KRYS_NODISCARD constexpr FontSelectionValue ItalicValue() noexcept
  {
    return FontSelectionValue {14};
  }

  KRYS_NODISCARD constexpr FontSelectionValue BoldThreshold() noexcept
  {
    return FontSelectionValue {600};
  }

  KRYS_NODISCARD constexpr FontSelectionValue BoldWeightValue() noexcept
  {
    return FontSelectionValue {700};
  }

  KRYS_NODISCARD constexpr FontSelectionValue NormalWeightValue() noexcept
  {
    return FontSelectionValue {400};
  }

  KRYS_NODISCARD constexpr FontSelectionValue LightWeightValue() noexcept
  {
    return FontSelectionValue {200};
  }

  KRYS_NODISCARD constexpr bool IsFontWeightBold(FontSelectionValue fontWeight) noexcept
  {
    return fontWeight >= BoldThreshold();
  }

  KRYS_NODISCARD constexpr FontSelectionValue LowerWeightSearchThreshold() noexcept
  {
    return FontSelectionValue {400};
  }

  KRYS_NODISCARD constexpr FontSelectionValue UpperWeightSearchThreshold() noexcept
  {
    return FontSelectionValue {500};
  }

  KRYS_NODISCARD constexpr FontSelectionValue UltraCondensedWidthValue() noexcept
  {
    return FontSelectionValue {50};
  }

  KRYS_NODISCARD constexpr FontSelectionValue ExtraCondensedWidthValue() noexcept
  {
    return FontSelectionValue {62.5f};
  }

  KRYS_NODISCARD constexpr FontSelectionValue CondensedWidthValue() noexcept
  {
    return FontSelectionValue {75};
  }

  KRYS_NODISCARD constexpr FontSelectionValue SemiCondensedWidthValue() noexcept
  {
    return FontSelectionValue {87.5f};
  }

  KRYS_NODISCARD constexpr FontSelectionValue NormalWidthValue() noexcept
  {
    return FontSelectionValue {100};
  }

  KRYS_NODISCARD constexpr FontSelectionValue SemiExpandedWidthValue() noexcept
  {
    return FontSelectionValue {112.5f};
  }

  KRYS_NODISCARD constexpr FontSelectionValue ExpandedWidthValue() noexcept
  {
    return FontSelectionValue {125};
  }

  KRYS_NODISCARD constexpr FontSelectionValue ExtraExpandedWidthValue() noexcept
  {
    return FontSelectionValue {150};
  }

  KRYS_NODISCARD constexpr FontSelectionValue UltraExpandedWidthValue() noexcept
  {
    return FontSelectionValue {200};
  }

  /// @brief An inclusive range of font selection values: [`Minimum`, `Maximum`].
  struct FontSelectionRange
  {
    using Value = FontSelectionValue;

    Value Minimum {1};
    Value Maximum {0};

    constexpr FontSelectionRange(Value a, Value b) noexcept : Minimum(std::min(a, b)), Maximum(std::max(a, b))
    {
    }

    explicit constexpr FontSelectionRange(Value value) noexcept : Minimum(value), Maximum(value)
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const FontSelectionRange &) const noexcept = default;

    KRYS_NODISCARD constexpr bool IsValid() const noexcept
    {
      return Minimum <= Maximum;
    }

    constexpr void Expand(const FontSelectionRange &other) noexcept
    {
      krys_debug_assert(other.IsValid());
      if (!IsValid())
      {
        *this = other;
      }
      else
      {
        Minimum = std::min(Minimum, other.Minimum);
        Maximum = std::max(Maximum, other.Maximum);
      }
      krys_debug_assert(IsValid());
    }

    KRYS_NODISCARD constexpr bool Includes(Value target) const noexcept
    {
      return target >= Minimum && target <= Maximum;
    }
  };

  struct FontSelectionRequest
  {
    using Value = FontSelectionValue;

    Value Weight;
    Value Width;

    // FIXME: We are using an optional here to be able to distinguish between an explicit or implicit slope
    // (for "italic" and "oblique") and the "normal" value which has no slope. The "italic" and "oblique"
    // values can be distinguished by looking at the "fontStyleAxis" on the FontDescription. We should come up
    // with a tri-state member so that it's a lot clearer whether we're dealing with a "normal", "italic" or
    // explicit "oblique" font style. See webkit.org/b/187774.
    Maybe<Value> Slope;

    KRYS_NODISCARD constexpr bool operator==(const FontSelectionRequest &) const noexcept = default;
  };

  struct FontSelectionCapabilities
  {
    using Range = FontSelectionRange;

    Range Weight {NormalWeightValue()};
    Range Width {NormalWidthValue()};
    Range Slope {NormalItalicValue()};

    KRYS_NODISCARD constexpr bool operator==(const FontSelectionCapabilities &) const noexcept = default;

    constexpr void Expand(const FontSelectionCapabilities &capabilities) noexcept
    {
      Weight.Expand(capabilities.Weight);
      Width.Expand(capabilities.Width);
      Slope.Expand(capabilities.Slope);
    }
  };

  struct FontSelectionSpecifiedCapabilities
  {
    using Capabilities = FontSelectionCapabilities;
    using Range = FontSelectionRange;
    using OptionalRange = Maybe<Range>;

    OptionalRange Weight;
    OptionalRange Width;
    OptionalRange Slope;

    KRYS_NODISCARD constexpr Capabilities ComputeFontSelectionCapabilities() const noexcept
    {
      return {ComputeWeight(), ComputeWidth(), ComputeSlope()};
    }

    KRYS_NODISCARD constexpr bool
      operator==(const FontSelectionSpecifiedCapabilities &) const noexcept = default;

    constexpr FontSelectionSpecifiedCapabilities &operator=(const Capabilities &other) noexcept
    {
      Weight = other.Weight;
      Width = other.Width;
      Slope = other.Slope;
      return *this;
    }

    KRYS_NODISCARD constexpr Range ComputeWeight() const noexcept
    {
      return Weight.value_or(Range {NormalWeightValue()});
    }

    KRYS_NODISCARD constexpr Range ComputeWidth() const noexcept
    {
      return Width.value_or(Range {NormalWidthValue()});
    }

    KRYS_NODISCARD constexpr Range ComputeSlope() const noexcept
    {
      return Slope.value_or(Range {NormalItalicValue()});
    }
  };

  /// @brief Picks the capabilities that best match a request, following the CSS font matching algorithm
  /// (width, then style, then weight).
  class FontSelectionAlgorithm
  {
  public:
    using Capabilities = FontSelectionCapabilities;

    struct DistanceResult
    {
      FontSelectionValue Distance;
      FontSelectionValue Value;
    };

  private:
    using DistanceFunction = DistanceResult (FontSelectionAlgorithm::*)(Capabilities) const;
    using CapabilitiesRange = FontSelectionRange Capabilities::*;

    FontSelectionRequest _request;
    Capabilities _capabilitiesBounds;
    const List<Capabilities> &_capabilities;

  public:
    FontSelectionAlgorithm() = delete;
    FontSelectionAlgorithm(FontSelectionRequest, const List<Capabilities> &,
                           Maybe<Capabilities> capabilitiesBounds = std::nullopt) noexcept;

    KRYS_NODISCARD DistanceResult WidthDistance(Capabilities) const noexcept;
    KRYS_NODISCARD DistanceResult StyleDistance(Capabilities) const noexcept;
    KRYS_NODISCARD DistanceResult WeightDistance(Capabilities) const noexcept;

    KRYS_NODISCARD size_t IndexOfBestCapabilities() noexcept;

  private:
    KRYS_NODISCARD FontSelectionValue BestValue(Span<const bool> eliminated, DistanceFunction) const noexcept;
    void FilterCapability(Span<bool> eliminated, DistanceFunction, CapabilitiesRange) noexcept;
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::FontSelectionValue>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontSelectionValue &value) noexcept
    {
      return DefaultHash<short>::Hash(value.RawValue());
    }

    KRYS_NODISCARD static bool Equal(const boo::FontSelectionValue &a,
                                     const boo::FontSelectionValue &b) noexcept
    {
      return a == b;
    }
  };

  template <>
  struct DefaultHash<boo::FontSelectionRange>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontSelectionRange &range) noexcept
    {
      return krys::Hash::Combine(range.Minimum, range.Maximum);
    }

    KRYS_NODISCARD static bool Equal(const boo::FontSelectionRange &a,
                                     const boo::FontSelectionRange &b) noexcept
    {
      return a == b;
    }
  };

  template <>
  struct DefaultHash<boo::FontSelectionRequest>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontSelectionRequest &request) noexcept
    {
      // An absent slope hashes apart from every present one.
      return krys::Hash::Combine(request.Weight, request.Width, request.Slope.has_value(),
                                 request.Slope.value_or(boo::FontSelectionValue {}));
    }

    KRYS_NODISCARD static bool Equal(const boo::FontSelectionRequest &a,
                                     const boo::FontSelectionRequest &b) noexcept
    {
      return a == b;
    }
  };

  template <>
  struct DefaultHash<boo::FontSelectionSpecifiedCapabilities>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontSelectionSpecifiedCapabilities &capabilities) noexcept
    {
      auto range = [](const boo::FontSelectionSpecifiedCapabilities::OptionalRange &value)
      {
        return value ? krys::Hash::Combine(true, *value) : DefaultHash<bool>::Hash(false);
      };
      return krys::Hash::Combine(range(capabilities.Weight), range(capabilities.Width),
                                 range(capabilities.Slope));
    }

    KRYS_NODISCARD static bool Equal(const boo::FontSelectionSpecifiedCapabilities &a,
                                     const boo::FontSelectionSpecifiedCapabilities &b) noexcept
    {
      return a == b;
    }
  };
}
