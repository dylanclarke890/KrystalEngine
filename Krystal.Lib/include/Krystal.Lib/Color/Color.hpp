#pragma once

#include "Krystal.Lib/Color/ColorConversion.hpp"
#include "Krystal.Lib/Color/ColorFlags.hpp"
#include "Krystal.Lib/Color/ColorSpace.hpp"
#include "Krystal.Lib/Color/ColorUtilities.hpp"
#include "Krystal.Lib/Color/DestinationColorSpace.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys
{
  /// @brief Able to represent:
  /// - Special "invalid color" state, treated as transparent black but distinguishable
  /// - 4x 8-bit (0-255) sRGBA, stored inline, no allocation
  /// - 4x float color components + color space, stored in a reference counted sub-object
  class Color
  {
    constexpr static uint64 invalidColorAndFlags = 0;

    uint64 _colorAndFlags {invalidColorAndFlags};

    class OutOfLineComponents : public ThreadSafeRefCounted<OutOfLineComponents>
    {
    private:
      ColorComponents<float, 4> _components;

      OutOfLineComponents(ColorComponents<float, 4> &&components) noexcept
          : _components(Krys::Move(components))
      {
      }

    public:
      KRYS_NODISCARD static Ref<OutOfLineComponents> Create(ColorComponents<float, 4> &&components) noexcept
      {
        return AdoptRef(*new OutOfLineComponents(Krys::Move(components)));
      }

      KRYS_NODISCARD float UnresolvedAlpha() const noexcept
      {
        return _components[3];
      }

      KRYS_NODISCARD float ResolvedAlpha() const noexcept
      {
        return IsNaN(_components[3]) ? 0 : _components[3];
      }

      KRYS_NODISCARD ColorComponents<float, 4> UnresolvedComponents() const noexcept
      {
        return _components;
      }

      KRYS_NODISCARD ColorComponents<float, 4> ResolvedComponents() const noexcept
      {
        return ResolveColorComponents(_components);
      }
    };

  public:
    constexpr static size_t maxNumberOfBitsInPointer = 48uz;
    constexpr static uint64 colorValueMask = (1ULL << maxNumberOfBitsInPointer) - 1uz;
    constexpr static uint64 flagsSize = sizeof(ColorFlagsIncludingPrivate) * 8uz;
    constexpr static uint64 flagsShift = maxNumberOfBitsInPointer;
    constexpr static uint64 colorSpaceSize = sizeof(ColorSpace) * 8uz;
    constexpr static uint64 colorSpaceShift = flagsShift + flagsSize;
    static_assert(flagsSize + colorSpaceSize + maxNumberOfBitsInPointer <= 64uz);

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

    template <IsColorTypeWithComponentType<float> ColorType>
    Color(const ColorType &color, ColorFlags flags = ColorFlags::None) noexcept
    {
      SetOutOfLineComponents(OutOfLineComponents::Create(AsColorComponents(color.Unresolved())),
                             ColorSpaceFor<ColorType>, ToFlagsIncludingPrivate(flags));
    }

    template <IsColorTypeWithComponentType<float> ColorType>
    Color(const Maybe<ColorType> &color, ColorFlags flags = ColorFlags::None) noexcept
    {
      if (color)
      {
        SetOutOfLineComponents(OutOfLineComponents::Create(AsColorComponents(color->Unresolved())),
                               ColorSpaceFor<ColorType>, ToFlagsIncludingPrivate(flags));
      }
    }

    Color(Ref<OutOfLineComponents> &&color, ColorSpace colorSpace,
          ColorFlags flags = ColorFlags::None) noexcept
    {
      SetOutOfLineComponents(Krys::Move(color), colorSpace, ToFlagsIncludingPrivate(flags));
    }

    ~Color() noexcept
    {
      if (IsOutOfLine())
      {
        AsOutOfLine().SubRef();
      }
    }

    Color(const Color &color) noexcept : _colorAndFlags(color._colorAndFlags)
    {
      if (IsOutOfLine())
      {
        AsOutOfLine().AddRef();
      }
    }

    Color &operator=(const Color &color) noexcept
    {
      if (_colorAndFlags == color._colorAndFlags)
      {
        return *this;
      }

      if (IsOutOfLine())
      {
        AsOutOfLine().SubRef();
      }

      _colorAndFlags = color._colorAndFlags;

      if (IsOutOfLine())
      {
        AsOutOfLine().AddRef();
      }

      return *this;
    }

    Color(Color &&other) noexcept
    {
      *this = Krys::Move(other);
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

    KRYS_NODISCARD ColorFlagsIncludingPrivate Flags() const noexcept
    {
      return DecodedFlags(_colorAndFlags);
    }

    KRYS_NODISCARD bool IsOutOfLine() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::OutOfLine);
    }

    KRYS_NODISCARD const OutOfLineComponents &AsOutOfLine() const noexcept
    {
      assert(IsOutOfLine());
      return DecodedOutOfLineComponents(_colorAndFlags);
    }

    KRYS_NODISCARD Maybe<PackedColor::RGBA> TryGetAsPackedInline() const noexcept
    {
      if (IsInline())
      {
        return AsPackedInline();
      }

      return Null;
    }

    KRYS_NODISCARD bool IsInline() const noexcept
    {
      return !HasFlag(Flags(), ColorFlagsIncludingPrivate::OutOfLine);
    }

    KRYS_NODISCARD SRGBA<uint8> AsInline() const noexcept
    {
      assert(IsInline());
      return AsSRGBA(AsPackedInline());
    }

    KRYS_NODISCARD PackedColor::RGBA AsPackedInline() const noexcept
    {
      assert(IsInline());
      return DecodedPackedInlineColor(_colorAndFlags);
    }

    KRYS_NODISCARD Maybe<SRGBA<uint8>> TryGetAsSRGBABytes() const noexcept
    {
      if (IsInline())
      {
        return AsInline();
      }

      return Null;
    }

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::Valid);
    }

    KRYS_NODISCARD bool IsSemantic() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::Semantic);
    }

    KRYS_NODISCARD bool UsesColorFunctionSerialization() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::UseColorFunctionSerialization);
    }

    KRYS_NODISCARD bool IsHashTableDeletedValue() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::HashTableDeletedValue);
    }

    KRYS_NODISCARD bool IsHashTableEmptyValue() const noexcept
    {
      return HasFlag(Flags(), ColorFlagsIncludingPrivate::HashTableEmptyValue);
    }

    KRYS_NODISCARD ColorSpace Space() const noexcept
    {
      return DecodedColorSpace(_colorAndFlags);
    }

    KRYS_NODISCARD bool IsOpaque() const noexcept
    {
      return IsOutOfLine() ? AsOutOfLine().ResolvedAlpha() == 1.0 : AsInline().Resolved().alpha == 255;
    }

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return IsOutOfLine() ? AsOutOfLine().ResolvedAlpha() > 0.0 : AsInline().Resolved().alpha > 0;
    }

    KRYS_NODISCARD uint8 AlphaByte() const noexcept
    {
      return IsOutOfLine() ? ConvertFloatAlphaTo<uint8>(AsOutOfLine().ResolvedAlpha())
                           : AsInline().Resolved().alpha;
    }

    KRYS_NODISCARD float AlphaAsFloat() const noexcept
    {
      return IsOutOfLine() ? AsOutOfLine().ResolvedAlpha()
                           : ConvertByteAlphaTo<float>(AsInline().Resolved().alpha);
    }

    KRYS_NODISCARD double Luminance() const noexcept;

    KRYS_NODISCARD double Lightness() const noexcept; // FIXME: Replace remaining uses with Luminance.

    KRYS_NODISCARD bool AnyComponentIsNone() const noexcept;

    template <typename Functor>
    KRYS_NODISCARD decltype(auto) CallOnUnderlyingType(Functor &&functor) const noexcept
    {
      if (IsOutOfLine())
      {
        return CallWithColorType(AsOutOfLine().UnresolvedComponents(), Space(),
                                 std::forward<Functor>(functor));
      }

      return std::invoke(std::forward<Functor>(functor), AsInline());
    }

    /// @brief Converts the underlying color into ColorType. Potentially lossily if the gamut or precision of
    /// ColorType is smaller than the current underlying type.
    template <typename ColorType>
    KRYS_NODISCARD ColorType ToColorTypeLossy() const noexcept
    {
      return CallOnUnderlyingType([](const auto &underlyingColor)
                                  { return ConvertColor<ColorType>(underlyingColor); });
    }

    /// @brief This acts just like ToColorTypeLossy(), but will carry forward missing components
    /// from the underlying type into any analogous components in ColorType.
    template <typename ColorType>
    KRYS_NODISCARD ColorType ToColorTypeLossyCarryingForwardMissing() const noexcept
    {
      return CallOnUnderlyingType([](const auto &underlyingColor)
                                  { return ConvertColorCarryingForwardMissing<ColorType>(underlyingColor); });
    }

    KRYS_NODISCARD ColorComponents<float, 4> ToResolvedColorComponentsInColorSpace(ColorSpace) const noexcept;

    KRYS_NODISCARD ColorComponents<float, 4>
      ToResolvedColorComponentsInColorSpace(const DestinationColorSpace &) const noexcept;

    KRYS_NODISCARD std::pair<ColorSpace, ColorComponents<float, 4>>
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

    KRYS_NODISCARD Color ColorWithAlpha(float) const noexcept;

    KRYS_NODISCARD Color OpaqueColor() const noexcept
    {
      return ColorWithAlpha(1.0f);
    }

    KRYS_NODISCARD Color SemanticColor() const noexcept;

    KRYS_NODISCARD static bool IsBlackColor(const Color &) noexcept;

    KRYS_NODISCARD static bool IsWhiteColor(const Color &) noexcept;

    void SetColor(SRGBA<uint8> color,
                  ColorFlagsIncludingPrivate flags = ColorFlagsIncludingPrivate::None) noexcept
    {
      flags = flags | ColorFlagsIncludingPrivate::Valid;
      _colorAndFlags = EncodedInlineColor(color) | EncodedColorSpace(ColorSpace::SRGB) | EncodedFlags(flags);
      assert(IsInline());
    }

    void SetOutOfLineComponents(Ref<OutOfLineComponents> &&color, ColorSpace colorSpace,
                                ColorFlagsIncludingPrivate flags = ColorFlagsIncludingPrivate::None) noexcept
    {
      flags = flags | ColorFlagsIncludingPrivate::Valid | ColorFlagsIncludingPrivate::OutOfLine;
      _colorAndFlags =
        EncodedOutOfLineComponents(Krys::Move(color)) | EncodedColorSpace(colorSpace) | EncodedFlags(flags);
      assert(IsOutOfLine());
    }

    // Returns the underlying color converted to pre-Resolved 8-bit sRGBA, useful for debugging purposes.
    struct DebugRGBAData
    {
      uint32 red;
      uint32 green;
      uint32 blue;
      uint32 alpha;
    };

    KRYS_NODISCARD DebugRGBAData DebugRGBA() const noexcept;

    // Out of line and inline colors will always be non-equal.
    friend bool operator==(const Color &a, const Color &b) noexcept;

    friend bool EqualIgnoringSemanticColor(const Color &a, const Color &b) noexcept;

    friend bool OutOfLineComponentsEqual(const Color &, const Color &) noexcept;

    friend bool OutOfLineComponentsEqualIgnoringSemanticColor(const Color &, const Color &) noexcept;

#pragma region Codec Helpers

    KRYS_NODISCARD static ColorFlagsIncludingPrivate ToFlagsIncludingPrivate(ColorFlags flags) noexcept
    {
      return static_cast<ColorFlagsIncludingPrivate>(ToUnderlying(flags));
    }

    KRYS_NODISCARD static uint64 EncodedFlags(ColorFlagsIncludingPrivate flags) noexcept
    {
      return static_cast<uint64>(flags) << flagsShift;
    }

    KRYS_NODISCARD static uint64 EncodedColorSpace(ColorSpace colorSpace) noexcept
    {
      return static_cast<uint64>(colorSpace) << colorSpaceShift;
    }

    KRYS_NODISCARD static uint64 EncodedInlineColor(SRGBA<uint8> color) noexcept
    {
      return EncodedPackedInlineColor(PackedColor::RGBA {color});
    }

    KRYS_NODISCARD static uint64 EncodedPackedInlineColor(PackedColor::RGBA color) noexcept
    {
      return color.value;
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

    KRYS_NODISCARD static ColorSpace DecodedColorSpace(uint64 value) noexcept
    {
      return static_cast<ColorSpace>(static_cast<uint8>(value >> colorSpaceShift));
    }

    KRYS_NODISCARD static SRGBA<uint8> DecodedInlineColor(uint64 value) noexcept
    {
      return AsSRGBA(DecodedPackedInlineColor(value));
    }

    KRYS_NODISCARD static PackedColor::RGBA DecodedPackedInlineColor(uint64 value) noexcept
    {
      return PackedColor::RGBA {static_cast<uint32>(value & colorValueMask)};
    }

    KRYS_NODISCARD static OutOfLineComponents &DecodedOutOfLineComponents(uint64 value) noexcept
    {
      return *std::bit_cast<OutOfLineComponents *>(value & colorValueMask);
    }

#pragma endregion
  };

  // One or both must be out of line colors.
  KRYS_NODISCARD bool OutOfLineComponentsEqual(const Color &a, const Color &b) noexcept
  {
    if (a.IsOutOfLine() && b.IsOutOfLine())
    {
      return a.AsOutOfLine().UnresolvedComponents() == b.AsOutOfLine().UnresolvedComponents()
             && a.Space() == b.Space() && a.Flags() == b.Flags();
    }

    assert(a.IsOutOfLine() || b.IsOutOfLine());
    return false;
  }

  KRYS_NODISCARD bool OutOfLineComponentsEqualIgnoringSemanticColor(const Color &a, const Color &b) noexcept
  {
    if (a.IsOutOfLine() && b.IsOutOfLine())
    {
      auto aFlags = ToUnderlying(a.Flags()) - ToUnderlying(ColorFlagsIncludingPrivate::Semantic);
      auto bFlags = ToUnderlying(b.Flags()) - ToUnderlying(ColorFlagsIncludingPrivate::Semantic);
      return a.AsOutOfLine().UnresolvedComponents() == b.AsOutOfLine().UnresolvedComponents()
             && a.Space() == b.Space() && aFlags == bFlags;
    }

    assert(a.IsOutOfLine() || b.IsOutOfLine());
    return false;
  }

  KRYS_NODISCARD inline bool EqualIgnoringSemanticColor(const Color &a, const Color &b) noexcept
  {
    if (a.IsOutOfLine() || b.IsOutOfLine())
    {
      return OutOfLineComponentsEqualIgnoringSemanticColor(a, b);
    }

    auto aFlags = ToUnderlying(a.Flags()) - ToUnderlying(ColorFlagsIncludingPrivate::Semantic);
    auto bFlags = ToUnderlying(b.Flags()) - ToUnderlying(ColorFlagsIncludingPrivate::Semantic);
    return a.AsPackedInline().value == b.AsPackedInline().value && aFlags == bFlags;
  }

  KRYS_NODISCARD bool operator==(const Color &a, const Color &b) noexcept
  {
    if (a.IsOutOfLine() || b.IsOutOfLine())
    {
      return OutOfLineComponentsEqual(a, b);
    }

    return a._colorAndFlags == b._colorAndFlags;
  }
}