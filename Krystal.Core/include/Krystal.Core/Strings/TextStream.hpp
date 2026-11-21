/*
 * Copyright (C) 2004, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include <string_view>

namespace krys
{
  /// @brief Writes values as indented, human-readable text: debug dumps, logs and test expectations.
  /// @note Types write themselves by overloading `operator<<(TextStream &, const T &)` next to the type.
  class TextStream
  {
    KRYS_NON_COPYABLE(TextStream);

  public:
    struct FormatNumberRespectingIntegers
    {
      double Value;

      FormatNumberRespectingIntegers(double number) noexcept : Value(number)
      {
      }
    };

    enum class Formatting : uint8
    {
      SVGStyleRect = 1 << 0, // "at (0,0) size 10x10"
      NumberRespectingIntegers = 1 << 1,
      LayoutUnitsAsIntegers = 1 << 2,
    };

    enum class LineMode : bool
    {
      SingleLine,
      MultipleLine,
    };

    struct Repeat
    {
      uint32 Width {0};
      char Character {' '};

      Repeat(uint32 width, char character) noexcept : Width(width), Character(character)
      {
      }
    };

    /// @brief Increases the indent for the length of its scope.
    class IndentScope
    {
      KRYS_NON_COPY_MOVABLE(IndentScope);

    private:
      TextStream &_stream;
      int32 _amount;

    public:
      explicit IndentScope(TextStream &ts, int32 amount = 1) noexcept : _stream(ts), _amount(amount)
      {
        _stream.IncreaseIndent(_amount);
      }

      ~IndentScope() noexcept
      {
        _stream.DecreaseIndent(_amount);
      }
    };

    /// @brief Writes a parenthesised group for the length of its scope.
    class GroupScope
    {
      KRYS_NON_COPY_MOVABLE(GroupScope);

    private:
      TextStream &_stream;

    public:
      explicit GroupScope(TextStream &ts) noexcept : _stream(ts)
      {
        _stream.StartGroup();
      }

      ~GroupScope() noexcept
      {
        _stream.EndGroup();
      }
    };

  private:
    StringBuilder _text;
    int32 _indent {0};
    EnumSet<Formatting> _formattingFlags;
    bool _multiLineMode {true};
    uint32 _containerSizeLimit {0};

  public:
    explicit TextStream(LineMode lineMode = LineMode::MultipleLine, EnumSet<Formatting> formattingFlags = {},
                        uint32 containerSizeLimit = 0) noexcept
        : _formattingFlags(formattingFlags), _multiLineMode(lineMode == LineMode::MultipleLine),
          _containerSizeLimit(containerSizeLimit)
    {
    }

    TextStream(TextStream &&) noexcept = default;

    TextStream &operator=(TextStream &&) noexcept = default;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _text.IsEmpty();
    }

    TextStream &operator<<(bool b) noexcept;

    TextStream &operator<<(char c) noexcept;

    TextStream &operator<<(char8 c) noexcept;

    TextStream &operator<<(int32 i) noexcept;

    TextStream &operator<<(uint32 i) noexcept;

    TextStream &operator<<(long i) noexcept;

    TextStream &operator<<(unsigned long i) noexcept;

    TextStream &operator<<(long long i) noexcept;

    TextStream &operator<<(unsigned long long i) noexcept;

    TextStream &operator<<(float f) noexcept;

    TextStream &operator<<(double d) noexcept;

    TextStream &operator<<(const char *string) noexcept;

    TextStream &operator<<(std::string_view string) noexcept;

    TextStream &operator<<(const char8 *string) noexcept;

    TextStream &operator<<(utf8_stringview string) noexcept;

    TextStream &operator<<(const utf8_string &string) noexcept;

    TextStream &operator<<(const void *p) noexcept;

    // Deprecated. Use the NumberRespectingIntegers FormattingFlag instead.
    TextStream &operator<<(const FormatNumberRespectingIntegers &numberToFormat) noexcept;

    KRYS_NODISCARD EnumSet<Formatting> FormattingFlags() const noexcept
    {
      return _formattingFlags;
    }

    void SetFormattingFlags(EnumSet<Formatting> flags) noexcept
    {
      _formattingFlags = flags;
    }

    KRYS_NODISCARD bool HasFormattingFlag(Formatting flag) const noexcept
    {
      return _formattingFlags.Contains(flag);
    }

    template <typename T>
    void DumpProperty(utf8_stringview name, const T &value) noexcept
    {
      TextStream &ts = *this;
      ts.StartGroup();
      ts << name << ' ' << value;
      ts.EndGroup();
    }

    KRYS_NODISCARD utf8_string Release() noexcept;

    void StartGroup() noexcept;

    void EndGroup() noexcept;

    // Output newline and indent.
    void NextLine() noexcept;

    KRYS_NODISCARD int32 GetIndent() const noexcept
    {
      return _indent;
    }

    void SetIndent(int32 indent) noexcept
    {
      _indent = indent;
    }

    void IncreaseIndent(int32 amount = 1) noexcept
    {
      _indent += amount;
    }

    void DecreaseIndent(int32 amount = 1) noexcept
    {
      _indent -= amount;
      krys_debug_assert(_indent >= 0);
    }

    void WriteIndent() noexcept;

    KRYS_NODISCARD uint32 ContainerSizeLimit() const noexcept
    {
      return _containerSizeLimit;
    }

    // Stream manipulators.
    TextStream &operator<<(TextStream &(*func)(TextStream &)) noexcept
    {
      return (*func)(*this);
    }

    TextStream &operator<<(const Repeat &repeated) noexcept
    {
      _text.AppendRepeated(static_cast<char8>(repeated.Character), repeated.Width);
      return *this;
    }
  };

  /// @brief Writes the stream's current indent.
  inline TextStream &Indent(TextStream &ts) noexcept
  {
    ts.WriteIndent();
    return ts;
  }

  template <typename T>
  struct ValueOrNull
  {
    T *Value;

    explicit ValueOrNull(T *value) noexcept : Value(value)
    {
    }
  };

  template <typename T>
  TextStream &operator<<(TextStream &ts, ValueOrNull<T> item) noexcept
  {
    if (item.Value)
    {
      ts << *item.Value;
    }
    else
    {
      ts << "null";
    }
    return ts;
  }

  template <typename TItem>
  TextStream &operator<<(TextStream &ts, const Maybe<TItem> &item) noexcept
  {
    if (item)
    {
      return ts << *item;
    }

    return ts << "nullopt";
  }

  template <typename TSizedContainer>
  TextStream &StreamSizedContainer(TextStream &ts, const TSizedContainer &sizedContainer) noexcept
  {
    ts << '[';
    uint32 count = 0;
    for (const auto &value : sizedContainer)
    {
      if (count)
      {
        ts << ", ";
      }
      ts << value;
      if (++count == ts.ContainerSizeLimit())
      {
        break;
      }
    }
    if (count != sizedContainer.size())
    {
      ts << ", ...";
    }
    return ts << ']';
  }

  template <typename TItem>
  TextStream &operator<<(TextStream &ts, const List<TItem> &list) noexcept
  {
    return StreamSizedContainer(ts, list);
  }

  template <typename TItem, size_t TExtent>
  TextStream &operator<<(TextStream &ts, Span<TItem, TExtent> items) noexcept
  {
    return StreamSizedContainer(ts, items);
  }

  template <typename T, typename U>
  TextStream &operator<<(TextStream &ts, const Pair<T, U> &pair) noexcept
  {
    return ts << '[' << pair.first << ", " << pair.second << ']';
  }

  template <typename TEnum>
  TextStream &operator<<(TextStream &ts, const EnumSet<TEnum> &options) noexcept
  {
    ts << '[';
    bool needComma = false;
    for (auto option : options)
    {
      if (needComma)
      {
        ts << ", ";
      }
      needComma = true;
      ts << option;
    }
    return ts << ']';
  }

  // Deprecated. Use TextStream::WriteIndent() instead.
  void WriteIndent(TextStream &ts, int32 indent) noexcept;
}
