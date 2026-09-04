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

      KRYS_NODISCARD ColorComponents<float, 4> resolvedComponents() const noexcept
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

    // TODO: merge these into their declarations.

    inline Color::Color(SRGBA<uint8> color, ColorFlags flags)
    {
      SetColor(color, ToFlagsIncludingPrivate(flags));
    }

    inline Color::Color(Maybe<SRGBA<uint8>> color, ColorFlags flags)
    {
      if (color)
      {
        SetColor(*color, ToFlagsIncludingPrivate(flags));
      }
    }

    template <IsColorTypeWithComponentType<float> ColorType>
    inline Color::Color(const ColorType &color, ColorFlags flags)
    {
      SetOutOfLineComponents(OutOfLineComponents::Create(AsColorComponents(color.Unresolved())),
                             ColorSpaceFor<ColorType>, ToFlagsIncludingPrivate(flags));
    }

    template <IsColorTypeWithComponentType<float> ColorType>
    inline Color::Color(const Maybe<ColorType> &color, ColorFlags flags)
    {
      if (color)
        SetOutOfLineComponents(OutOfLineComponents::Create(AsColorComponents(color->Unresolved())),
                               ColorSpaceFor<ColorType>, ToFlagsIncludingPrivate(flags));
    }

    inline Color::Color(Ref<OutOfLineComponents> &&outOfLineComponents, ColorSpace colorSpace,
                        ColorFlags flags)
    {
      SetOutOfLineComponents(WTF::move(outOfLineComponents), colorSpace, ToFlagsIncludingPrivate(flags));
    }

    inline Color::Color(const Color &other) : _colorAndFlags(other._colorAndFlags)
    {
      if (IsOutOfLine())
        AsOutOfLine().ref();
    }

    Color(SRGBA<uint8>, ColorFlags flags = ColorFlags::None) noexcept;

    Color(Maybe<SRGBA<uint8>>, ColorFlags flags = ColorFlags::None) noexcept;

    template <IsColorTypeWithComponentType<float> ColorType>
    Color(const ColorType &, ColorFlags flags = ColorFlags::None) noexcept;

    template <IsColorTypeWithComponentType<float> ColorType>
    Color(const Maybe<ColorType> &, ColorFlags flags = ColorFlags::None) noexcept;

    Color(Ref<OutOfLineComponents> &&, ColorSpace, ColorFlags flags = ColorFlags::None) noexcept;

    ~Color() noexcept;

    Color(const Color &) noexcept;
    Color &operator=(const Color &) noexcept;

    Color(Color &&) noexcept;
    Color &operator=(Color &&) noexcept;

    KRYS_NODISCARD bool IsValid() const noexcept;

    KRYS_NODISCARD bool IsSemantic() const noexcept;

    KRYS_NODISCARD bool UsesColorFunctionSerialization() const noexcept;

    KRYS_NODISCARD ColorSpace colorSpace() const noexcept;

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
    KRYS_NODISCARD decltype(auto) CallOnUnderlyingType(Functor &&) const noexcept;

    /// @brief Converts the underlying color into ColorType. Potentially lossily if the gamut or precision of
    /// ColorType is smaller than the current underlying type.
    template <typename ColorType>
    KRYS_NODISCARD ColorType ToColorTypeLossy() const noexcept;

    /// @brief This acts just like ToColorTypeLossy(), but will carry forward missing components
    /// from the underlying type into any analogous components in ColorType.
    template <typename ColorType>
    KRYS_NODISCARD ColorType ToColorTypeLossyCarryingForwardMissing() const noexcept;

    KRYS_NODISCARD ColorComponents<float, 4> ToResolvedColorComponentsInColorSpace(ColorSpace) const noexcept;

    KRYS_NODISCARD ColorComponents<float, 4>
      ToResolvedColorComponentsInColorSpace(const DestinationColorSpace &) const noexcept;

    KRYS_NODISCARD std::pair<ColorSpace, ColorComponents<float, 4>>
      ColorSpaceAndResolvedColorComponents() const noexcept;

    KRYS_NODISCARD Color Lightened() const noexcept;

    KRYS_NODISCARD Color Darkened() const noexcept;

    KRYS_NODISCARD Color InvertedColorWithAlpha(Maybe<float> alpha) const noexcept;

    KRYS_NODISCARD Color InvertedColorWithAlpha(float alpha) const noexcept;

    KRYS_NODISCARD Color ColorWithAlphaMultipliedBy(Maybe<float>) const noexcept;

    KRYS_NODISCARD Color ColorWithAlphaMultipliedBy(float) const noexcept;

    KRYS_NODISCARD Color ColorWithAlpha(Maybe<float>) const noexcept;

    KRYS_NODISCARD Color ColorWithAlpha(float) const noexcept;

    KRYS_NODISCARD Color OpaqueColor() const noexcept
    {
      return ColorWithAlpha(1.0f);
    }

    KRYS_NODISCARD Color SemanticColor() const noexcept;

    // Returns the underlying color if its type is inline.
    KRYS_NODISCARD Maybe<PackedColor::RGBA> TryGetAsPackedInline() const noexcept;

    KRYS_NODISCARD Maybe<SRGBA<uint8>> TryGetAsSRGBABytes() const noexcept;

    KRYS_NODISCARD static bool IsBlackColor(const Color &) noexcept;

    KRYS_NODISCARD static bool IsWhiteColor(const Color &) noexcept;

    // Out of line and inline colors will always be non-equal.
    friend bool operator==(const Color &a, const Color &b) noexcept;

    friend bool EqualIgnoringSemanticColor(const Color &a, const Color &b) noexcept;

    friend bool OutOfLineComponentsEqual(const Color &, const Color &) noexcept;

    friend bool OutOfLineComponentsEqualIgnoringSemanticColor(const Color &, const Color &) noexcept;

    // Returns the underlying color converted to pre-Resolved 8-bit sRGBA, useful for debugging purposes.
    struct DebugRGBAData
    {
      uint32 red;
      uint32 green;
      uint32 blue;
      uint32 alpha;
    };

    KRYS_NODISCARD DebugRGBAData DebugRGBA() const noexcept;

    KRYS_NODISCARD static ColorFlagsIncludingPrivate ToFlagsIncludingPrivate(ColorFlags flags) noexcept
    {
      return static_cast<ColorFlagsIncludingPrivate>(ToUnderlying(flags));
    }

    KRYS_NODISCARD ColorFlagsIncludingPrivate Flags() const noexcept;

    KRYS_NODISCARD bool IsOutOfLine() const noexcept;

    KRYS_NODISCARD bool IsInline() const noexcept;

    void SetColor(SRGBA<uint8>, ColorFlagsIncludingPrivate flags = ColorFlagsIncludingPrivate::None) noexcept;

    void SetOutOfLineComponents(Ref<OutOfLineComponents> &&, ColorSpace,
                                ColorFlagsIncludingPrivate flags = ColorFlagsIncludingPrivate::None) noexcept;

    SRGBA<uint8> AsInline() const;

    PackedColor::RGBA AsPackedInline() const;

    const OutOfLineComponents &AsOutOfLine() const;

    KRYS_NODISCARD static uint64 EncodedFlags(ColorFlagsIncludingPrivate flags) noexcept;

    KRYS_NODISCARD static uint64 EncodedColorSpace(ColorSpace) noexcept;

    KRYS_NODISCARD static uint64 EncodedInlineColor(SRGBA<uint8>) noexcept;

    KRYS_NODISCARD static uint64 EncodedPackedInlineColor(PackedColor::RGBA) noexcept;

    KRYS_NODISCARD static uint64 EncodedOutOfLineComponents(Ref<OutOfLineComponents> &&) noexcept;

    KRYS_NODISCARD static ColorFlagsIncludingPrivate DecodedFlags(uint64) noexcept;

    KRYS_NODISCARD static ColorSpace DecodedColorSpace(uint64) noexcept;

    KRYS_NODISCARD static SRGBA<uint8> DecodedInlineColor(uint64) noexcept;

    KRYS_NODISCARD static PackedColor::RGBA DecodedPackedInlineColor(uint64) noexcept;

    KRYS_NODISCARD static OutOfLineComponents &DecodedOutOfLineComponents(uint64) noexcept;
  };

  // One or both must be out of line colors.
  KRYS_NODISCARD bool OutOfLineComponentsEqual(const Color &a, const Color &b) noexcept
  {
    if (a.IsOutOfLine() && b.IsOutOfLine())
    {
      return a.AsOutOfLine().UnresolvedComponents() == b.AsOutOfLine().UnresolvedComponents()
             && a.colorSpace() == b.colorSpace() && a.Flags() == b.Flags();
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
             && a.colorSpace() == b.colorSpace() && aFlags == bFlags;
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

  inline Color::Color(Color &&other)
  {
    *this = WTF::move(other);
  }

  inline Color &Color::operator=(const Color &other)
  {
    if (_colorAndFlags == other._colorAndFlags)
      return *this;

    if (IsOutOfLine())
      AsOutOfLine().deref();

    _colorAndFlags = other._colorAndFlags;

    if (IsOutOfLine())
      AsOutOfLine().ref();

    return *this;
  }

  inline Color &Color::operator=(Color &&other)
  {
    if (this == &other)
      return *this;

    if (IsOutOfLine())
      AsOutOfLine().deref();

    _colorAndFlags = std::exchange(other._colorAndFlags, invalidColorAndFlags);
    return *this;
  }

  inline bool Color::isHashTableDeletedValue() const
  {
    return Flags().contains(ColorFlagsIncludingPrivate::HashTableDeletedValue);
  }

  inline bool Color::isHashTableEmptyValue() const
  {
    return Flags().contains(ColorFlagsIncludingPrivate::HashTableEmptyValue);
  }

  inline Color::~Color()
  {
    if (IsOutOfLine())
      AsOutOfLine().deref();
  }

  inline bool Color::IsValid() const
  {
    return Flags().contains(ColorFlagsIncludingPrivate::Valid);
  }

  inline bool Color::IsSemantic() const
  {
    return Flags().contains(ColorFlagsIncludingPrivate::Semantic);
  }

  inline bool Color::UsesColorFunctionSerialization() const
  {
    return Flags().contains(ColorFlagsIncludingPrivate::UseColorFunctionSerialization);
  }

  inline ColorSpace Color::colorSpace() const
  {
    return DecodedColorSpace(_colorAndFlags);
  }

  template <typename Functor>
  decltype(auto) Color::CallOnUnderlyingType(Functor &&functor) const
  {
    if (IsOutOfLine())
      return callWithColorType(AsOutOfLine().UnresolvedComponents(), colorSpace(),
                               std::forward<Functor>(functor));
    return std::invoke(std::forward<Functor>(functor), AsInline());
  }

  template <typename ColorType>
  ColorType Color::ToColorTypeLossy() const
  {
    return CallOnUnderlyingType([](const auto &underlyingColor)
                                { return convertColor<ColorType>(underlyingColor); });
  }

  template <typename ColorType>
  ColorType Color::ToColorTypeLossyCarryingForwardMissing() const
  {
    return CallOnUnderlyingType([](const auto &underlyingColor)
                                { return convertColorCarryingForwardMissing<ColorType>(underlyingColor); });
  }

  inline Color Color::InvertedColorWithAlpha(Maybe<float> alpha) const
  {
    return alpha ? InvertedColorWithAlpha(alpha.value()) : *this;
  }

  inline Color Color::ColorWithAlphaMultipliedBy(float amount) const
  {
    return ColorWithAlpha(amount * AlphaAsFloat());
  }

  inline Color Color::ColorWithAlphaMultipliedBy(Maybe<float> alpha) const
  {
    return alpha ? ColorWithAlphaMultipliedBy(alpha.value()) : *this;
  }

  inline Color Color::ColorWithAlpha(Maybe<float> alpha) const
  {
    return alpha ? ColorWithAlpha(alpha.value()) : *this;
  }

  inline OptionSet<Color::ColorFlagsIncludingPrivate> Color::Flags() const
  {
    return DecodedFlags(_colorAndFlags);
  }

  inline bool Color::IsOutOfLine() const
  {
    return Flags().contains(ColorFlagsIncludingPrivate::OutOfLine);
  }

  inline bool Color::IsInline() const
  {
    return !Flags().contains(ColorFlagsIncludingPrivate::OutOfLine);
  }

  inline const Color::OutOfLineComponents &Color::AsOutOfLine() const
  {
    assert(IsOutOfLine());
    return DecodedOutOfLineComponents(_colorAndFlags);
  }

  inline Ref<Color::OutOfLineComponents> Color::protectedAsOutOfLine() const
  {
    assert(IsOutOfLine());
    return DecodedOutOfLineComponents(_colorAndFlags);
  }

  inline SRGBA<uint8> Color::AsInline() const
  {
    assert(IsInline());
    return asSRGBA(AsPackedInline());
  }

  inline PackedColor::RGBA Color::AsPackedInline() const
  {
    assert(IsInline());
    return DecodedPackedInlineColor(_colorAndFlags);
  }

  inline Maybe<PackedColor::RGBA> Color::TryGetAsPackedInline() const
  {
    if (IsInline())
      return AsPackedInline();
    return std::nullopt;
  }

  inline Maybe<SRGBA<uint8>> Color::TryGetAsSRGBABytes() const
  {
    if (IsInline())
      return AsInline();
    return std::nullopt;
  }

  inline uint64 Color::EncodedFlags(OptionSet<ColorFlagsIncludingPrivate> flags)
  {
    return static_cast<uint64>(flags.toRaw()) << flagsShift;
  }

  inline uint64 Color::EncodedColorSpace(ColorSpace colorSpace)
  {
    return static_cast<uint64>(colorSpace) << colorSpaceShift;
  }

  inline uint64 Color::EncodedInlineColor(SRGBA<uint8> color)
  {
    return EncodedPackedInlineColor(PackedColor::RGBA {color});
  }

  inline uint64 Color::EncodedPackedInlineColor(PackedColor::RGBA color)
  {
    return color.value;
  }

  inline uint64 Color::EncodedOutOfLineComponents(Ref<OutOfLineComponents> &&outOfLineComponents)
  {
#if CPU(ADDRESS64)
    return std::bit_cast<uint64>(&outOfLineComponents.leakRef());
#else
    return std::bit_cast<uint32_t>(&outOfLineComponents.leakRef());
#endif
  }

  inline OptionSet<Color::ColorFlagsIncludingPrivate> Color::DecodedFlags(uint64 value)
  {
    return OptionSet<Color::ColorFlagsIncludingPrivate>::fromRaw(static_cast<uint8>(value >> flagsShift));
  }

  inline ColorSpace Color::DecodedColorSpace(uint64 value)
  {
    return static_cast<ColorSpace>(static_cast<uint8>(value >> colorSpaceShift));
  }

  inline SRGBA<uint8> Color::DecodedInlineColor(uint64 value)
  {
    return asSRGBA(DecodedPackedInlineColor(value));
  }

  inline PackedColor::RGBA Color::DecodedPackedInlineColor(uint64 value)
  {
    return PackedColor::RGBA {static_cast<uint32_t>(value & colorValueMask)};
  }

  inline Color::OutOfLineComponents &Color::DecodedOutOfLineComponents(uint64 value)
  {
#if CPU(ADDRESS64)
    return *std::bit_cast<OutOfLineComponents *>(value & colorValueMask);
#else
    return *std::bit_cast<OutOfLineComponents *>(static_cast<uint32_t>(value & colorValueMask));
#endif
  }

  inline void Color::SetColor(SRGBA<uint8> color, OptionSet<ColorFlagsIncludingPrivate> flags)
  {
    flags.add({ColorFlagsIncludingPrivate::Valid});
    _colorAndFlags = EncodedInlineColor(color) | EncodedColorSpace(ColorSpace::SRGB) | EncodedFlags(flags);
    assert(IsInline());
  }

  inline void Color::SetOutOfLineComponents(Ref<OutOfLineComponents> &&color, ColorSpace colorSpace,
                                            OptionSet<ColorFlagsIncludingPrivate> flags)
  {
    flags.add({ColorFlagsIncludingPrivate::Valid, ColorFlagsIncludingPrivate::OutOfLine});
    _colorAndFlags =
      EncodedOutOfLineComponents(WTF::move(color)) | EncodedColorSpace(colorSpace) | EncodedFlags(flags);
    assert(IsOutOfLine());
  }

}
}