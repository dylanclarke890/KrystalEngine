#pragma once

#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <algorithm>
#include <functional>

namespace krys::boo
{
  struct GlyphOverflow;

  /// @brief Caches the widths of short words, sampling words at an interval that shrinks while the cache
  /// hits.
  class WidthCache
  {
  private:
    // Used to optimize small strings as hash table keys. Avoids allocating the string.
    class SmallStringKey
    {
    private:
      static constexpr uint32 Capacity = 16;

      Array<char8, Capacity> _characters {};
      uint32 _length {0};

    public:
      constexpr SmallStringKey() noexcept = default;

      explicit SmallStringKey(utf8_stringview string) noexcept : _length(static_cast<uint32>(string.length()))
      {
        krys_debug_assert(_length <= Capacity);
        std::ranges::copy(string, _characters.begin());
      }

      KRYS_NODISCARD constexpr static uint32 GetCapacity() noexcept
      {
        return Capacity;
      }

      KRYS_NODISCARD utf8_stringview Characters() const noexcept
      {
        return {_characters.data(), _length};
      }

      KRYS_NODISCARD bool operator==(const SmallStringKey &) const noexcept = default;
    };

    struct SmallStringKeyHash
    {
      KRYS_NODISCARD size_t operator()(const SmallStringKey &key) const noexcept
      {
        return std::hash<utf8_stringview> {}(key.Characters());
      }
    };

    using Map = HashMap<SmallStringKey, float, SmallStringKeyHash>;
    using SingleCharMap = HashMap<char32, float>;

    static constexpr int32 MinInterval = -3;   // A cache hit pays for about 3 cache misses.
    static constexpr int32 MaxInterval = 20;   // Sampling at this interval has almost no overhead.
    static constexpr uint32 MaxSize = 500'000; // Just enough to guard against pathological growth.

    int32 _interval {MaxInterval};
    int32 _countdown {_interval};
    SingleCharMap _singleCharMap;
    Map _map;
    bool _hasSeenIdeograph {false};

  public:
    WidthCache() noexcept = default;

    /// @brief The cache entry for @p text, added with @p entry if it's new, or null if @p text isn't cached
    /// (too long, or not sampled).
    KRYS_NODISCARD float *Add(utf8_stringview text, float entry) noexcept
    {
      auto length = text.length();
      // Do not allow length = 0.
      if (!length) KRYS_UNLIKELY
      {
        return nullptr;
      }

      if (length > SmallStringKey::GetCapacity())
      {
        return nullptr;
      }

      if (_countdown > 0)
      {
        --_countdown;
        return nullptr;
      }

      return AddSlowCase(text, entry);
    }

    KRYS_NODISCARD float *Add(const TextRun &run, float entry, bool hasKerningOrLigatures,
                              bool hasWordSpacingOrLetterSpacing, bool hasTextSpacing,
                              GlyphOverflow *glyphOverflow) noexcept
    {
      // The width cache is not really profitable unless we're doing expensive glyph transformations.
      if (!hasKerningOrLigatures)
      {
        return nullptr;
      }
      // Word spacing and letter spacing can change the width of a word.
      if (hasWordSpacingOrLetterSpacing)
      {
        return nullptr;
      }
      // Since this is just a width cache, we don't have enough information to satisfy glyph queries.
      if (glyphOverflow)
      {
        return nullptr;
      }
      // If we allow tabs and a tab occurs inside a word, the width of the word varies based on its position
      // on the line.
      if (run.AllowTabs())
      {
        return nullptr;
      }
      // width calculation with text-spacing depends on context of adjacent characters.
      if (hasTextSpacing && InvalidateCacheForTextSpacing(run))
      {
        return nullptr;
      }

      return Add(run.Text(), entry);
    }

    void Clear() noexcept
    {
      _singleCharMap.clear();
      _map.clear();
    }

  private:
    KRYS_NODISCARD float *AddSlowCase(utf8_stringview text, float entry) noexcept
    {
      bool isNewEntry;
      float *value;
      auto first = text::DecodeUTF8(Span<const char8> {text});
      if (first.Length == text.length())
      {
        auto [iterator, inserted] = _singleCharMap.try_emplace(first.CodePoint, entry);
        isNewEntry = inserted;
        value = &iterator->second;
      }
      else
      {
        auto [iterator, inserted] = _map.try_emplace(SmallStringKey {text}, entry);
        isNewEntry = inserted;
        value = &iterator->second;
      }

      // Cache hit: ramp up by sampling the next few words.
      if (!isNewEntry)
      {
        _interval = MinInterval;
        return value;
      }

      // Cache miss: ramp down by increasing our sampling interval.
      if (_interval < MaxInterval)
      {
        ++_interval;
      }
      _countdown = _interval;

      if ((_singleCharMap.size() + _map.size()) < MaxSize)
      {
        return value;
      }

      // No need to be fancy: we're just trying to avoid pathological growth.
      _singleCharMap.clear();
      _map.clear();
      return nullptr;
    }

    // returns true if cache is/was invalidated
    KRYS_NODISCARD bool InvalidateCacheForTextSpacing(const TextRun &textRun) noexcept
    {
      if (_hasSeenIdeograph)
      {
        return true;
      }

      for (text::CodePointIterator it {textRun.Text()}; !it.AtEnd(); ++it)
      {
        if (text_spacing::IsIdeograph(*it))
        {
          _hasSeenIdeograph = true;
          Clear();
          return true;
        }
      }
      return false;
    }
  };
}
