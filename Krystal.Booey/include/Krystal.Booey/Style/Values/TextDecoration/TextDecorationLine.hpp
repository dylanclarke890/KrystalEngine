#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::style
{
  // text-decoration-line = none | [ underline || overline || line-through || blink ] | spelling-error |
  // grammar-error https://www.w3.org/TR/css-text-decor-4/#text-decoration-line-property

  // We are representing TextDecorationLine in 5 bits.
  // 1 bit is used for defining the Type (SingleValue or Flags)
  // 4 bits are used for defining the Value
  // Values for SingleValue: None, SpellingError, GrammarError
  // Values for Flags: Any combination of Underline, Overline, LineThrough, Blink
  // Therefore, we are packing its content with the following layout:
  // Bits 7-5 : Reserved
  // Bit 4    : Type (SingleValue or Flags)
  // Bits 3-0 : When Type=1 (Underline=0x1, Overline=0x2, LineThrough=0x4, Blink=0x8)
  //          : When Type=0 (None = 0, SpellingError = 1, GrammarError = 2)
  struct TextDecorationLine
  {
    enum class Type : uint8
    {
      SingleValue = 0,
      Flags = 1 << 4
    };

    static constexpr uint8 TypeMask = 1 << 4; // 0001 0000
    static constexpr uint8 ValuesMask = 0x0F;

    // Values when Type is SingleValue
    enum class SingleValue : uint8
    {
      None = 0,
      SpellingError,
      GrammarError
    };

    // Ordinal, for `EnumSet`: each flag's bit is `1 << value`.
    enum class Flag : uint8
    {
      Underline,
      Overline,
      LineThrough,
      Blink,
    };

    // Values when Type is Flags
    static constexpr uint8 UnderlineBit = 1 << ToUnderlying(TextDecorationLine::Flag::Underline);
    static constexpr uint8 OverlineBit = 1 << ToUnderlying(TextDecorationLine::Flag::Overline);
    static constexpr uint8 LineThroughBit = 1 << ToUnderlying(TextDecorationLine::Flag::LineThrough);
    static constexpr uint8 BlinkBit = 1 << ToUnderlying(TextDecorationLine::Flag::Blink);

    static constexpr uint8 SingleValueNone =
      static_cast<uint8>(Type::SingleValue) | static_cast<uint8>(SingleValue::None);
    static constexpr uint8 SingleValueSpellingError =
      static_cast<uint8>(Type::SingleValue) | static_cast<uint8>(SingleValue::SpellingError);
    static constexpr uint8 SingleValueGrammarError =
      static_cast<uint8>(Type::SingleValue) | static_cast<uint8>(SingleValue::GrammarError);

    constexpr TextDecorationLine() noexcept = default;

    constexpr TextDecorationLine(css::keywords::None) noexcept : _packed(SingleValueNone)
    {
    }

    constexpr TextDecorationLine(css::keywords::SpellingError) noexcept : _packed(SingleValueSpellingError)
    {
    }

    constexpr TextDecorationLine(css::keywords::GrammarError) noexcept : _packed(SingleValueGrammarError)
    {
    }

    constexpr TextDecorationLine(EnumSet<TextDecorationLine::Flag> flags) noexcept
        : _packed(flags.IsEmpty() ? SingleValueNone : PackFlags(flags))
    {
    }

    constexpr TextDecorationLine(TextDecorationLine::Flag flag) noexcept
        : TextDecorationLine(EnumSet<TextDecorationLine::Flag> {flag})
    {
    }

    KRYS_NODISCARD constexpr static TextDecorationLine FromRaw(uint8 rawValue) noexcept
    {
      return TextDecorationLine(rawValue);
    }
    KRYS_NODISCARD constexpr uint8 ToRaw() const noexcept
    {
      return _packed;
    }

    KRYS_NODISCARD constexpr Type GetType() const noexcept
    {
      return static_cast<Type>(_packed & TypeMask);
    }
    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      return _packed == SingleValueNone;
    }
    KRYS_NODISCARD constexpr bool IsSpellingError() const noexcept
    {
      return _packed == SingleValueSpellingError;
    }
    KRYS_NODISCARD constexpr bool IsGrammarError() const noexcept
    {
      return _packed == SingleValueGrammarError;
    }
    KRYS_NODISCARD constexpr bool IsFlags() const noexcept
    {
      return GetType() == Type::Flags;
    }

    KRYS_NODISCARD constexpr bool HasUnderline() const noexcept
    {
      return (IsFlags()) && (_packed & UnderlineBit);
    }

    KRYS_NODISCARD constexpr bool HasOverline() const noexcept
    {
      return (IsFlags()) && (_packed & OverlineBit);
    }

    KRYS_NODISCARD constexpr bool HasLineThrough() const noexcept
    {
      return (IsFlags()) && (_packed & LineThroughBit);
    }

    KRYS_NODISCARD constexpr bool HasBlink() const noexcept
    {
      return (IsFlags()) && (_packed & BlinkBit);
    }

    KRYS_NODISCARD constexpr bool ContainsAny(EnumSet<TextDecorationLine::Flag> options) const noexcept
    {
      if (!IsFlags())
      {
        return false;
      }
      return (_packed & PackFlags(options));
    }

    KRYS_NODISCARD constexpr bool Contains(TextDecorationLine::Flag option) const noexcept
    {
      if (!IsFlags())
      {
        return false;
      }
      return (_packed & PackFlagValue(option));
    }

    constexpr void Remove(TextDecorationLine::Flag option) noexcept
    {
      if (GetType() == Type::Flags)
      {
        _packed &= ~PackFlagValue(option);
        // If none flags are set we should represent this as Type::None
        if (!(_packed & ValuesMask))
        {
          SetNone();
        }
      }
    }

    KRYS_NODISCARD uint8 AddOrReplaceIfNotNone(const TextDecorationLine &) noexcept;

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (GetType())
      {
        case Type::Flags: return visitor(UnpackFlags());
        case Type::SingleValue:
        {
          if (IsNone())
          {
            return visitor(css::keywords::None {});
          }
          if (IsSpellingError())
          {
            return visitor(css::keywords::SpellingError {});
          }
          krys_debug_assert(IsGrammarError());
          return visitor(css::keywords::GrammarError {});
        }
      }
      krys_debug_assert_msg(false, "Unreachable");
      return visitor(css::keywords::None {});
    }

    constexpr void SetNone() noexcept
    {
      _packed = SingleValueNone;
    }
    constexpr void SetSpellingError() noexcept
    {
      _packed = SingleValueSpellingError;
    }
    constexpr void SetGrammarError() noexcept
    {
      _packed = SingleValueGrammarError;
    }
    constexpr void SetFlags(EnumSet<TextDecorationLine::Flag> flags) noexcept
    {
      if (IsFlags())
      {
        _packed |= PackFlags(flags);
      }
      else
      {
        _packed = PackFlags(flags);
      }
    }

    KRYS_NODISCARD constexpr operator bool() const noexcept
    {
      return !IsNone();
    }
    KRYS_NODISCARD constexpr bool operator==(const TextDecorationLine &) const noexcept = default;

    KRYS_NODISCARD constexpr static uint8 PackFlags(EnumSet<TextDecorationLine::Flag> flags) noexcept
    {
      uint8 result = static_cast<uint8>(Type::Flags);
      if (flags.Contains(TextDecorationLine::Flag::Underline))
      {
        result |= UnderlineBit;
      }
      if (flags.Contains(TextDecorationLine::Flag::Overline))
      {
        result |= OverlineBit;
      }
      if (flags.Contains(TextDecorationLine::Flag::LineThrough))
      {
        result |= LineThroughBit;
      }
      if (flags.Contains(TextDecorationLine::Flag::Blink))
      {
        result |= BlinkBit;
      }
      return result;
    }

  private:
    constexpr TextDecorationLine(uint8 rawValue) noexcept : _packed(rawValue)
    {
    }

    // Returns only the value bits, not to be confused with "toRaw", which returns the whole packed raw
    // representation
    KRYS_NODISCARD constexpr uint8 RawValue() const noexcept
    {
      return _packed & ValuesMask;
    }

    // Note that this function packs only the 'Value' bit, ignoring the Type. This is useful for bitwise
    // operations.
    KRYS_NODISCARD constexpr static uint8 PackFlagValue(TextDecorationLine::Flag flag) noexcept
    {
      switch (flag)
      {
        case TextDecorationLine::Flag::Underline:   return UnderlineBit;
        case TextDecorationLine::Flag::Overline:    return OverlineBit;
        case TextDecorationLine::Flag::LineThrough: return LineThroughBit;
        case TextDecorationLine::Flag::Blink:       return BlinkBit;
      }
      krys_debug_assert_msg(false, "Unreachable");
      return 0;
    }

    KRYS_NODISCARD constexpr EnumSet<TextDecorationLine::Flag> UnpackFlags() const noexcept
    {
      krys_debug_assert(IsFlags());
      EnumSet<TextDecorationLine::Flag> flags;
      if (_packed & UnderlineBit)
      {
        flags.Add(TextDecorationLine::Flag::Underline);
      }
      if (_packed & OverlineBit)
      {
        flags.Add(TextDecorationLine::Flag::Overline);
      }
      if (_packed & LineThroughBit)
      {
        flags.Add(TextDecorationLine::Flag::LineThrough);
      }
      if (_packed & BlinkBit)
      {
        flags.Add(TextDecorationLine::Flag::Blink);
      }
      return flags;
    }

    uint8 _packed {0};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TextDecorationLine>
  {
    KRYS_NODISCARD TextDecorationLine operator()(BuilderState &, const css::Value &) noexcept;
  };

  template <>
  struct CSSValueCreation<EnumSet<TextDecorationLine::Flag>>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const EnumSet<TextDecorationLine::Flag> &) noexcept;
  };

  // MARK: Serialization

  template <>
  struct Serialiser<EnumSet<TextDecorationLine::Flag>>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const EnumSet<TextDecorationLine::Flag> &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextDecorationLine)
