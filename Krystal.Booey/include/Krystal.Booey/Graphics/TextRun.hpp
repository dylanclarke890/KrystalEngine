#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/TabSize.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief A run of UTF-8 text to measure, with the direction, expansion and tab settings it's laid out
  /// with. Offsets into the run are byte offsets.
  /// @attention The run doesn't own its text: the text must outlive it.
  class TextRun final
  {
  private:
    utf8_stringview _text;
    TabSize _tabSize {0};
    // _xpos is the x position relative to the left start of the text line, not relative to the left
    // start of the containing block. In the case of right alignment or center alignment, left start of
    // the text line is not the same as left start of the containing block. This variable is only used
    // to calculate the width of \t
    float _xpos {0};
    float _horizontalGlyphStretch {1};
    float _expansion {0};
    ExpansionBehavior _expansionBehavior;
    text_spacing::SpacingState _textSpacingState;
    bool _allowTabs : 1 {false};
    TextDirection _direction : 1 {TextDirection::LTR};
    bool _directionalOverride : 1 {false}; // Was this direction set by an override character.
    bool _characterScanForCodePath : 1 {true};
    bool _disableSpacing : 1 {false};

  public:
    explicit TextRun(utf8_stringview text, float xpos = 0, float expansion = 0,
                     ExpansionBehavior expansionBehavior = ExpansionBehavior::DefaultBehavior(),
                     TextDirection direction = TextDirection::LTR, bool directionalOverride = false,
                     bool characterScanForCodePath = true) noexcept
        : _text(text), _xpos(xpos), _expansion(expansion), _expansionBehavior(expansionBehavior),
          _direction(direction), _directionalOverride(directionalOverride),
          _characterScanForCodePath(characterScanForCodePath)
    {
    }

    TextRun(const TextRun &) noexcept = default;
    TextRun &operator=(const TextRun &) noexcept = default;

    KRYS_NODISCARD bool operator==(const TextRun &other) const noexcept;

    KRYS_NODISCARD TextRun SubRun(uint32 startOffset, uint32 length) const noexcept
    {
      krys_assert(startOffset + length <= _text.length());
      auto result {*this};
      result.SetText(_text.substr(startOffset, length));
      return result;
    }

    /// @brief The code point that starts at byte @p offset, or U+FFFD if no valid sequence starts there.
    KRYS_NODISCARD char32 CharacterAt(uint32 offset) const noexcept
    {
      krys_assert(offset < _text.length());
      return text::DecodeUTF8(Span<const char8> {_text}.subspan(offset)).CodePoint;
    }

    /// @brief The length of the text in bytes.
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(_text.length());
    }

    void SetText(utf8_stringview text) noexcept
    {
      _text = text;
    }

    KRYS_NODISCARD float HorizontalGlyphStretch() const noexcept
    {
      return _horizontalGlyphStretch;
    }
    void SetHorizontalGlyphStretch(float scale) noexcept
    {
      _horizontalGlyphStretch = scale;
    }

    KRYS_NODISCARD bool AllowTabs() const noexcept
    {
      return _allowTabs;
    }
    KRYS_NODISCARD const boo::TabSize &TabSize() const noexcept
    {
      return _tabSize;
    }
    void SetTabSize(bool allow, const boo::TabSize &size) noexcept
    {
      _allowTabs = allow;
      _tabSize = size;
    }

    KRYS_NODISCARD float XPos() const noexcept
    {
      return _xpos;
    }
    void SetXPos(float xPos) noexcept
    {
      _xpos = xPos;
    }
    KRYS_NODISCARD float Expansion() const noexcept
    {
      return _expansion;
    }
    KRYS_NODISCARD boo::ExpansionBehavior ExpansionBehavior() const noexcept
    {
      return _expansionBehavior;
    }
    KRYS_NODISCARD TextDirection Direction() const noexcept
    {
      return _direction;
    }
    KRYS_NODISCARD bool Rtl() const noexcept
    {
      return _direction == TextDirection::RTL;
    }
    KRYS_NODISCARD bool Ltr() const noexcept
    {
      return _direction == TextDirection::LTR;
    }
    KRYS_NODISCARD bool DirectionalOverride() const noexcept
    {
      return _directionalOverride;
    }
    KRYS_NODISCARD bool CharacterScanForCodePath() const noexcept
    {
      return _characterScanForCodePath;
    }
    KRYS_NODISCARD bool SpacingDisabled() const noexcept
    {
      return _disableSpacing;
    }

    void DisableSpacing() noexcept
    {
      _disableSpacing = true;
    }
    void SetDirection(TextDirection direction) noexcept
    {
      _direction = direction;
    }
    void SetDirectionalOverride(bool override) noexcept
    {
      _directionalOverride = override;
    }
    void SetCharacterScanForCodePath(bool scan) noexcept
    {
      _characterScanForCodePath = scan;
    }

    KRYS_NODISCARD utf8_stringview Text() const noexcept
    {
      return _text;
    }

    void SetTextSpacingState(text_spacing::SpacingState spacingState) noexcept
    {
      _textSpacingState = spacingState;
    }
    KRYS_NODISCARD text_spacing::SpacingState TextSpacingState() const noexcept
    {
      return _textSpacingState;
    }
  };

  inline bool TextRun::operator==(const TextRun &other) const noexcept
  {
    return _text == other._text && _tabSize == other._tabSize && _xpos == other._xpos
           && _horizontalGlyphStretch == other._horizontalGlyphStretch && _expansion == other._expansion
           && _expansionBehavior == other._expansionBehavior && _allowTabs == other._allowTabs
           && _direction == other._direction && _directionalOverride == other._directionalOverride
           && _characterScanForCodePath == other._characterScanForCodePath
           && _disableSpacing == other._disableSpacing && _textSpacingState == other._textSpacingState;
  }
}
