#pragma once

#include "Krystal.Booey/Graphics/Color/ColorFlags.hpp"
#include "Krystal.Core/Colour/ColourConversion.hpp"
#include "Krystal.Core/Colour/ColourSpace.hpp"
#include "Krystal.Core/Colour/ColourUtilities.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include <bit>
#include <functional>
#include <utility>

namespace krys::boo
{
  class Color;
}

namespace krys
{
  template <>
  struct DefaultHash<krys::boo::Color>;
}

namespace krys::boo
{
  /// @brief Able to represent:
  /// - Special "invalid color" state, treated as transparent black but distinguishable
  /// - 4x 8-bit (0-255) sRGBA, stored inline, no allocation
  /// - 4x float color components + color space, stored in a reference counted sub-object
  class Color
  {
    friend struct krys::DefaultHash<Color>;

    // Out of line and inline colors will always be non-equal.
    friend bool operator==(const Color &a, const Color &b) noexcept;
    friend bool EqualIgnoringSemanticColor(const Color &a, const Color &b) noexcept;
    friend bool OutOfLineComponentsEqual(const Color &, const Color &) noexcept;
    friend bool OutOfLineComponentsEqualIgnoringSemanticColor(const Color &, const Color &) noexcept;

    class OutOfLineComponents : public ThreadSafeRefCounted<OutOfLineComponents>
    {
    private:
      ColourComponents<float, 4> _components;

      explicit OutOfLineComponents(ColourComponents<float, 4> &&components) noexcept
          : _components(krys::move(components))
      {
      }

    public:
      KRYS_NODISCARD static Ref<OutOfLineComponents> Create(ColourComponents<float, 4> &&components) noexcept
      {
        return AdoptRef(*new OutOfLineComponents(krys::move(components)));
      }

      KRYS_NODISCARD float UnresolvedAlpha() const noexcept
      {
        return _components[3];
      }

      KRYS_NODISCARD float ResolvedAlpha() const noexcept
      {
        return IsNaN(_components[3]) ? 0 : _components[3];
      }

      KRYS_NODISCARD ColourComponents<float, 4> UnresolvedComponents() const noexcept
      {
        return _components;
      }

      KRYS_NODISCARD ColourComponents<float, 4> ResolvedComponents() const noexcept
      {
        return ResolveColourComponents(_components);
      }
    };

  private:
    constexpr static uint64 maxNumberOfBitsInPointer = 48;
    constexpr static uint64 colorValueMask = (1ULL << maxNumberOfBitsInPointer) - 1;
    constexpr static uint64 flagsSize = sizeof(ColorFlagsIncludingPrivate) * 8;
    constexpr static uint64 flagsShift = maxNumberOfBitsInPointer;
    constexpr static uint64 colorSpaceSize = sizeof(ColourSpace) * 8;
    constexpr static uint64 colorSpaceShift = flagsShift + flagsSize;
    static_assert(flagsSize + colorSpaceSize + maxNumberOfBitsInPointer <= 64);
    static_assert(sizeof(void *) == 8, "Color packs a 48-bit pointer and assumes a 64-bit address space.");

    constexpr static uint64 invalidColorAndFlags = 0;

    uint64 _colorAndFlags {invalidColorAndFlags};

  public:
    constexpr static auto transparentBlack = SRGBA<uint8> {};
    constexpr static auto black = SRGBA<uint8> {0, 0, 0};
    constexpr static auto white = SRGBA<uint8> {255, 255, 255};
    constexpr static auto darkGray = SRGBA<uint8> {128, 128, 128};
    constexpr static auto gray = SRGBA<uint8> {160, 160, 160};
    constexpr static auto lightGray = SRGBA<uint8> {192, 192, 192};
    constexpr static auto cyan = SRGBA<uint8> {0, 255, 255};
    constexpr static auto yellow = SRGBA<uint8> {255, 255, 0};
    constexpr static auto red = SRGBA<uint8> {255, 0, 0};
    constexpr static auto magenta = SRGBA<uint8> {255, 0, 255};
    constexpr static auto blue = SRGBA<uint8> {0, 0, 255};
    constexpr static auto green = SRGBA<uint8> {0, 255, 0};
    constexpr static auto darkGreen = SRGBA<uint8> {0, 128, 0};
    constexpr static auto orange = SRGBA<uint8> {255, 128, 0};
    constexpr static auto purple = SRGBA<uint8> {128, 0, 255};
    constexpr static auto gold = SRGBA<uint8> {255, 215, 0};

    /// @brief Returns the underlying color converted to pre-resolved 8-bit sRGBA, useful for debugging
    /// purposes.
    struct DebugRGBAData
    {
      uint32 Red;
      uint32 Green;
      uint32 Blue;
      uint32 Alpha;
    };

    Color() noexcept = default;

    Color(SRGBA<uint8> color, ColorFlags flags = ColorFlags::None) noexcept
    {
      SetColor(color, ToFlagsIncludingPrivate(flags));
    }

    Color(Maybe<SRGBA<uint8>> color, ColorFlags flags = ColorFlags::None) noexcept
    {
      if (color)
      {
        SetColor(*color, ToFlagsIncludingPrivate(flags));
      }
    }

    template <IsColourTypeWithComponentType<float> ColorType>
    Color(const ColorType &color, ColorFlags flags = ColorFlags::None) noexcept
    {
      SetOutOfLineComponents(OutOfLineComponents::Create(AsColourComponents(color.Unresolved())),
                             ColourSpaceFor<ColorType>, ToFlagsIncludingPrivate(flags));
    }

    template <IsColourTypeWithComponentType<float> ColorType>
    Color(const Maybe<ColorType> &color, ColorFlags flags = ColorFlags::None) noexcept
    {
      if (color)
      {
        SetOutOfLineComponents(OutOfLineComponents::Create(AsColourComponents(color->Unresolved())),
                               ColourSpaceFor<ColorType>, ToFlagsIncludingPrivate(flags));
      }
    }

    explicit Color(HashTableEmptyValueTag) noexcept
        : _colorAndFlags(EncodedFlags(ColorFlagsIncludingPrivate::HashTableEmptyValue))
    {
    }

    explicit Color(HashTableDeletedValueTag) noexcept
        : _colorAndFlags(EncodedFlags(ColorFlagsIncludingPrivate::HashTableDeletedValue))
    {
    }

    Color(const Color &other) noexcept : _colorAndFlags(other._colorAndFlags)
    {
      if (IsOutOfLine())
      {
        AsOutOfLine().AddRef();
      }
    }

    Color(Color &&other) noexcept : _colorAndFlags(std::exchange(other._colorAndFlags, invalidColorAndFlags))
    {
    }

    Color &operator=(const Color &other) noexcept
    {
      if (_colorAndFlags == other._colorAndFlags)
      {
        return *this;
      }

      if (IsOutOfLine())
      {
        AsOutOfLine().SubRef();
      }

      _colorAndFlags = other._colorAndFlags;

      if (IsOutOfLine())
      {
        AsOutOfLine().AddRef();
      }

      return *this;
    }

    Color &operator=(Color &&other) noexcept
    {
      if (this == &other)
      {
        return *this;
      }

      if (IsOutOfLine())
      {
        AsOutOfLine().SubRef();
      }

      _colorAndFlags = std::exchange(other._colorAndFlags, invalidColorAndFlags);
      return *this;
    }

    ~Color() noexcept
    {
      if (IsOutOfLine())
      {
        AsOutOfLine().SubRef();
      }
    }

    KRYS_NODISCARD bool IsHashTableDeletedValue() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::HashTableDeletedValue);
    }

    KRYS_NODISCARD bool IsHashTableEmptyValue() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::HashTableEmptyValue);
    }

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::Valid);
    }

    KRYS_NODISCARD bool IsSemantic() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::Semantic);
    }

    KRYS_NODISCARD bool UsesColorFunctionSerialisation() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::UseColorFunctionSerialisation);
    }

    /// @note Named `Space` rather than `ColourSpace` so it doesn't hide the `ColourSpace` type.
    KRYS_NODISCARD ColourSpace Space() const noexcept
    {
      return DecodedColorSpace(_colorAndFlags);
    }

    KRYS_NODISCARD bool IsOpaque() const noexcept
    {
      return IsOutOfLine() ? AsOutOfLine().ResolvedAlpha() == 1.0f : AsInline().Resolved().Alpha == 255;
    }

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return IsOutOfLine() ? AsOutOfLine().ResolvedAlpha() > 0.0f : AsInline().Resolved().Alpha > 0;
    }

    KRYS_NODISCARD uint8 AlphaByte() const noexcept
    {
      return IsOutOfLine() ? ConvertFloatAlphaTo<uint8>(AsOutOfLine().ResolvedAlpha())
                           : AsInline().Resolved().Alpha;
    }

    KRYS_NODISCARD float AlphaAsFloat() const noexcept
    {
      return IsOutOfLine() ? AsOutOfLine().ResolvedAlpha()
                           : ConvertByteAlphaTo<float>(AsInline().Resolved().Alpha);
    }

    KRYS_NODISCARD double Luminance() const noexcept;

    // FIXME: Replace remaining uses with Luminance.
    KRYS_NODISCARD double Lightness() const noexcept;

    KRYS_NODISCARD bool AnyComponentIsNone() const noexcept;

    template <typename Functor>
    KRYS_NODISCARD decltype(auto) CallOnUnderlyingType(Functor &&functor) const noexcept
    {
      if (IsOutOfLine())
      {
        return CallWithColourType(AsOutOfLine().UnresolvedComponents(), Space(),
                                  std::forward<Functor>(functor));
      }

      return std::invoke(std::forward<Functor>(functor), AsInline());
    }

    /// @brief Converts the underlying color into ColorType, potentially lossily if the gamut or precision of
    /// ColorType is smaller than the current underlying type.
    template <typename ColorType>
    KRYS_NODISCARD ColorType ToColorTypeLossy() const noexcept
    {
      return CallOnUnderlyingType([](const auto &underlyingColor)
                                  { return ConvertColour<ColorType>(underlyingColor); });
    }

    /// @brief Acts just like ToColorTypeLossy(), but carries forward missing components from the underlying
    /// type into any analogous components in ColorType.
    template <typename ColorType>
    KRYS_NODISCARD ColorType ToColorTypeLossyCarryingForwardMissing() const noexcept
    {
      return CallOnUnderlyingType(
        [](const auto &underlyingColor)
        { return ConvertColourCarryingForwardMissing<ColorType>(underlyingColor); });
    }

    KRYS_NODISCARD ColourComponents<float, 4>
      ToResolvedColorComponentsInColorSpace(ColourSpace) const noexcept;

    KRYS_NODISCARD Pair<ColourSpace, ColourComponents<float, 4>>
      ColorSpaceAndResolvedColorComponents() const noexcept;

    KRYS_NODISCARD Color Lightened() const noexcept;

    KRYS_NODISCARD Color Darkened() const noexcept;

    KRYS_NODISCARD Color InvertedColorWithAlpha(Maybe<float> alpha) const noexcept
    {
      return alpha ? InvertedColorWithAlpha(alpha.value()) : *this;
    }

    KRYS_NODISCARD Color InvertedColorWithAlpha(float alpha) const noexcept;

    KRYS_NODISCARD Color ColorWithAlphaMultipliedBy(Maybe<float> alpha) const noexcept
    {
      return alpha ? ColorWithAlphaMultipliedBy(alpha.value()) : *this;
    }

    KRYS_NODISCARD Color ColorWithAlphaMultipliedBy(float amount) const noexcept
    {
      return ColorWithAlpha(amount * AlphaAsFloat());
    }

    KRYS_NODISCARD Color ColorWithAlpha(Maybe<float> alpha) const noexcept
    {
      return alpha ? ColorWithAlpha(alpha.value()) : *this;
    }

    KRYS_NODISCARD Color ColorWithAlpha(float alpha) const noexcept;

    KRYS_NODISCARD Color OpaqueColor() const noexcept
    {
      return ColorWithAlpha(1.0f);
    }

    KRYS_NODISCARD Color SemanticColor() const noexcept;

    /// @brief Returns the underlying color if its type is inline.
    KRYS_NODISCARD Maybe<PackedColour::RGBA> TryGetAsPackedInline() const noexcept
    {
      if (IsInline())
      {
        return AsPackedInline();
      }

      return null;
    }

    KRYS_NODISCARD Maybe<SRGBA<uint8>> TryGetAsSRGBABytes() const noexcept
    {
      if (IsInline())
      {
        return AsInline();
      }

      return null;
    }

    KRYS_NODISCARD static bool IsBlackColor(const Color &) noexcept;

    KRYS_NODISCARD static bool IsWhiteColor(const Color &) noexcept;

    KRYS_NODISCARD DebugRGBAData DebugRGBA() const noexcept;

    KRYS_NODISCARD utf8_string DebugDescription() const noexcept;

  private:
    Color(Ref<OutOfLineComponents> &&color, ColourSpace colorSpace,
          ColorFlags flags = ColorFlags::None) noexcept
    {
      SetOutOfLineComponents(krys::move(color), colorSpace, ToFlagsIncludingPrivate(flags));
    }

    KRYS_NODISCARD static ColorFlagsIncludingPrivate ToFlagsIncludingPrivate(ColorFlags flags) noexcept
    {
      return static_cast<ColorFlagsIncludingPrivate>(ToUnderlying(flags));
    }

    KRYS_NODISCARD ColorFlagsIncludingPrivate Flags() const noexcept
    {
      return DecodedFlags(_colorAndFlags);
    }

    KRYS_NODISCARD bool IsOutOfLine() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::OutOfLine);
    }

    KRYS_NODISCARD bool IsInline() const noexcept
    {
      return !HasFlag(Flags(), ColorFlagsIncludingPrivate::OutOfLine);
    }

    void SetColor(SRGBA<uint8> color,
                  ColorFlagsIncludingPrivate flags = ColorFlagsIncludingPrivate::None) noexcept
    {
      flags |= ColorFlagsIncludingPrivate::Valid;
      _colorAndFlags = EncodedInlineColor(color) | EncodedColorSpace(ColourSpace::SRGB) | EncodedFlags(flags);
      krys_assert(IsInline());
    }

    void SetOutOfLineComponents(Ref<OutOfLineComponents> &&color, ColourSpace colorSpace,
                                ColorFlagsIncludingPrivate flags = ColorFlagsIncludingPrivate::None) noexcept
    {
      flags |= ColorFlagsIncludingPrivate::Valid | ColorFlagsIncludingPrivate::OutOfLine;
      _colorAndFlags =
        EncodedOutOfLineComponents(krys::move(color)) | EncodedColorSpace(colorSpace) | EncodedFlags(flags);
      krys_assert(IsOutOfLine());
    }

    KRYS_NODISCARD SRGBA<uint8> AsInline() const noexcept
    {
      krys_assert(IsInline());
      return AsSRGBA(AsPackedInline());
    }

    KRYS_NODISCARD PackedColour::RGBA AsPackedInline() const noexcept
    {
      krys_assert(IsInline());
      return DecodedPackedInlineColor(_colorAndFlags);
    }

    KRYS_NODISCARD OutOfLineComponents &AsOutOfLine() const noexcept
    {
      krys_assert(IsOutOfLine());
      return DecodedOutOfLineComponents(_colorAndFlags);
    }

    KRYS_NODISCARD Ref<OutOfLineComponents> ProtectedAsOutOfLine() const noexcept
    {
      return ShareRef(AsOutOfLine());
    }

    KRYS_NODISCARD static uint64 EncodedFlags(ColorFlagsIncludingPrivate flags) noexcept
    {
      return static_cast<uint64>(flags) << flagsShift;
    }

    KRYS_NODISCARD static uint64 EncodedColorSpace(ColourSpace colorSpace) noexcept
    {
      return static_cast<uint64>(colorSpace) << colorSpaceShift;
    }

    KRYS_NODISCARD static uint64 EncodedInlineColor(SRGBA<uint8> color) noexcept
    {
      return EncodedPackedInlineColor(PackedColour::RGBA {color});
    }

    KRYS_NODISCARD static uint64 EncodedPackedInlineColor(PackedColour::RGBA color) noexcept
    {
      return color.Value;
    }

    KRYS_NODISCARD static uint64
      EncodedOutOfLineComponents(Ref<OutOfLineComponents> &&outOfLineComponents) noexcept
    {
      return std::bit_cast<uint64>(outOfLineComponents.release());
    }

    KRYS_NODISCARD static ColorFlagsIncludingPrivate DecodedFlags(uint64 value) noexcept
    {
      return static_cast<ColorFlagsIncludingPrivate>(static_cast<uint8>(value >> flagsShift));
    }

    KRYS_NODISCARD static ColourSpace DecodedColorSpace(uint64 value) noexcept
    {
      return static_cast<ColourSpace>(static_cast<uint8>(value >> colorSpaceShift));
    }

    KRYS_NODISCARD static SRGBA<uint8> DecodedInlineColor(uint64 value) noexcept
    {
      return AsSRGBA(DecodedPackedInlineColor(value));
    }

    KRYS_NODISCARD static PackedColour::RGBA DecodedPackedInlineColor(uint64 value) noexcept
    {
      return PackedColour::RGBA {static_cast<uint32>(value & colorValueMask)};
    }

    KRYS_NODISCARD static OutOfLineComponents &DecodedOutOfLineComponents(uint64 value) noexcept
    {
      return *std::bit_cast<OutOfLineComponents *>(value & colorValueMask);
    }
  };

  /// @note One or both must be out of line colors.
  KRYS_NODISCARD inline bool OutOfLineComponentsEqual(const Color &a, const Color &b) noexcept
  {
    if (a.IsOutOfLine() && b.IsOutOfLine())
    {
      return a.AsOutOfLine().UnresolvedComponents() == b.AsOutOfLine().UnresolvedComponents()
             && a.Space() == b.Space() && a.Flags() == b.Flags();
    }

    krys_assert(a.IsOutOfLine() || b.IsOutOfLine());
    return false;
  }

  /// @note One or both must be out of line colors.
  KRYS_NODISCARD inline bool OutOfLineComponentsEqualIgnoringSemanticColor(const Color &a,
                                                                           const Color &b) noexcept
  {
    if (a.IsOutOfLine() && b.IsOutOfLine())
    {
      auto aFlags = a.Flags() & ~ColorFlagsIncludingPrivate::Semantic;
      auto bFlags = b.Flags() & ~ColorFlagsIncludingPrivate::Semantic;
      return a.AsOutOfLine().UnresolvedComponents() == b.AsOutOfLine().UnresolvedComponents()
             && a.Space() == b.Space() && aFlags == bFlags;
    }

    krys_assert(a.IsOutOfLine() || b.IsOutOfLine());
    return false;
  }

  KRYS_NODISCARD inline bool EqualIgnoringSemanticColor(const Color &a, const Color &b) noexcept
  {
    if (a.IsOutOfLine() || b.IsOutOfLine())
    {
      return OutOfLineComponentsEqualIgnoringSemanticColor(a, b);
    }

    auto aFlags = a.Flags() & ~ColorFlagsIncludingPrivate::Semantic;
    auto bFlags = b.Flags() & ~ColorFlagsIncludingPrivate::Semantic;
    return a.AsPackedInline().Value == b.AsPackedInline().Value && aFlags == bFlags;
  }

  KRYS_NODISCARD inline bool operator==(const Color &a, const Color &b) noexcept
  {
    if (a.IsOutOfLine() || b.IsOutOfLine())
    {
      return OutOfLineComponentsEqual(a, b);
    }

    return a._colorAndFlags == b._colorAndFlags;
  }
}

namespace krys
{
  template <>
  struct DefaultHash<krys::boo::Color>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const krys::boo::Color &color) noexcept
    {
      if (color.IsOutOfLine())
      {
        auto [c1, c2, c3, alpha] = color.AsOutOfLine().UnresolvedComponents();
        return krys::Hash::Combine(c1, c2, c3, alpha, color.Space(), color.Flags());
      }

      return krys::Hash::Combine(color.AsPackedInline().Value, color.Flags());
    }

    KRYS_NODISCARD static bool Equal(const krys::boo::Color &a, const krys::boo::Color &b) noexcept
    {
      return a == b;
    }
  };
}
