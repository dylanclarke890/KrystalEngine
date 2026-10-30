#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Font;
  class TextSpacingTrim;

  namespace text_spacing
  {
    enum class CharacterClass : uint8
    {
      Undefined = 0,
      Ideograph = 1 << 0,
      NonIdeographLetter = 1 << 1,
      NonIdeographNumeral = 1 << 2,
      FullWidthOpeningPunctuation = 1 << 3,
      FullWidthClosingPunctuation = 1 << 4,
      FullWidthMiddleDotPunctuation = 1 << 5,
      FullWidthColonPunctuation = 1 << 6,
      FullWidthDotPunctuation = 1 << 7
    };

    struct CharactersData
    {
      char32_t PreviousCharacter {0};
      char32_t CurrentCharacter {0};
      char32_t NextCharacter {0};
      CharacterClass PreviousCharacterClass {};
      CharacterClass CurrentCharacterClass {};
      CharacterClass NextCharacterClass {};
    };

    /// @brief The text spacing class of @p character.
    /// @see https://www.w3.org/TR/css-text-4/#text-spacing-classes
    KRYS_NODISCARD CharacterClass GetCharacterClass(char32_t character) noexcept;

    KRYS_NODISCARD bool IsIdeograph(char32_t character) noexcept;

    /// @brief The half width version of @p font if @p textSpacingTrim trims the spacing of the current
    /// character of @p charactersData, else null.
    KRYS_NODISCARD RefPtr<Font> GetHalfWidthFontIfNeeded(const Font &font,
                                                         const TextSpacingTrim &textSpacingTrim,
                                                         CharactersData &charactersData) noexcept;

    struct SpacingState
    {
      CharacterClass LastCharacterClassFromPreviousRun {CharacterClass::Undefined};

      KRYS_NODISCARD bool operator==(const SpacingState &) const noexcept = default;
    };
  }

  class TextSpacingTrim
  {
  public:
    enum class TrimType : uint8
    {
      SpaceAll = 0, // equivalent to None in text-spacing shorthand
      TrimAll,
      Auto
    };

  private:
    TrimType _trim {TrimType::SpaceAll};

  public:
    constexpr TextSpacingTrim() noexcept = default;

    constexpr TextSpacingTrim(TrimType trimType) noexcept : _trim(trimType)
    {
    }

    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return _trim == TrimType::Auto;
    }

    KRYS_NODISCARD constexpr bool IsSpaceAll() const noexcept
    {
      return _trim == TrimType::SpaceAll;
    }

    KRYS_NODISCARD bool ShouldTrimSpacing(const text_spacing::CharactersData &) const noexcept;

    KRYS_NODISCARD constexpr bool operator==(const TextSpacingTrim &) const noexcept = default;

    KRYS_NODISCARD constexpr TrimType Type() const noexcept
    {
      return _trim;
    }
  };

  enum class TextAutospaceType : uint8
  {
    Auto,
    IdeographAlpha,
    IdeographNumeric,
    Normal
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::TextAutospaceType, 4uz);

namespace krys::boo
{
  class TextAutospace
  {
  public:
    using Type = TextAutospaceType;
    using Options = EnumSet<Type>;

  private:
    Options _options {};

  public:
    constexpr TextAutospace() noexcept = default;

    constexpr TextAutospace(Options options) noexcept : _options(options)
    {
    }

    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return _options.Contains(Type::Auto);
    }

    KRYS_NODISCARD constexpr bool IsNoAutospace() const noexcept
    {
      return _options.IsEmpty();
    }

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return _options.Contains(Type::Normal);
    }

    KRYS_NODISCARD constexpr bool HasIdeographAlpha() const noexcept
    {
      return _options.ContainsAny({Type::IdeographAlpha, Type::Normal});
    }

    KRYS_NODISCARD constexpr bool HasIdeographNumeric() const noexcept
    {
      return _options.ContainsAny({Type::IdeographNumeric, Type::Normal});
    }

    KRYS_NODISCARD constexpr Options GetOptions() const noexcept
    {
      return _options;
    }

    KRYS_NODISCARD constexpr bool operator==(const TextAutospace &) const noexcept = default;

    KRYS_NODISCARD bool ShouldApplySpacing(text_spacing::CharacterClass firstCharacterClass,
                                           text_spacing::CharacterClass secondCharacterClass) const noexcept;
    KRYS_NODISCARD bool ShouldApplySpacing(char32_t firstCharacter, char32_t secondCharacter) const noexcept;
    /// @brief The space added between ideographs and non-ideographs: 1/8 of @p font's CJK advance.
    KRYS_NODISCARD static float TextAutospaceSize(const Font &font) noexcept;
  };
}
