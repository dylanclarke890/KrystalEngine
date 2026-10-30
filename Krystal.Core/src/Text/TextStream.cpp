#include "Krystal.Core/Text/TextStream.hpp"
#include <charconv>
#include <cmath>
#include <cstdio>
#include <string>

namespace krys
{
  namespace
  {
    bool HasFractions(double val) noexcept
    {
      static constexpr double epsilon = 0.0001;
      int32 ival = static_cast<int32>(val);
      double dval = static_cast<double>(ival);
      return std::abs(val - dval) > epsilon;
    }

    // Mimics sprintf("%.[precision]f", ...), as ECMAScript's Number.prototype.toFixed writes it.
    void AppendFixedWidth(StringBuilder &builder, double number, int32 decimalPlaces) noexcept
    {
      if (std::isnan(number))
      {
        builder.Append("NaN");
        return;
      }
      if (std::isinf(number))
      {
        builder.Append(number < 0 ? "-Infinity" : "Infinity");
        return;
      }
      if (std::abs(number) >= 1e21)
      {
        // toFixed falls back to the shortest round-trip form for numbers this large.
        builder.Append(number);
        return;
      }

      // The digits of the magnitude, exact well beyond the requested places, so that ties round half away
      // from zero as toFixed does (std::to_chars would round them to even).
      constexpr int32 extraPlaces = 64;
      char buffer[400];
      auto result = std::to_chars(buffer, buffer + sizeof(buffer), std::abs(number), std::chars_format::fixed,
                                  decimalPlaces + extraPlaces);
      krys_debug_assert(result.ec == std::errc {});
      std::string digits {buffer, static_cast<size_t>(result.ptr - buffer)};

      auto point = digits.find('.');
      bool roundUp = digits[point + 1 + decimalPlaces] >= '5';
      digits.resize(decimalPlaces ? point + 1 + decimalPlaces : point);
      if (roundUp)
      {
        auto i = digits.size();
        while (i-- > 0)
        {
          if (digits[i] == '.')
          {
            continue;
          }
          if (digits[i] != '9')
          {
            ++digits[i];
            break;
          }
          digits[i] = '0';
          if (i == 0)
          {
            digits.insert(digits.begin(), '1');
          }
        }
      }

      // A negative number that rounds to zero is written without its sign.
      if (number < 0 && digits.find_first_not_of("0.") != std::string::npos)
      {
        builder.Append('-');
      }
      builder.Append(std::string_view {digits});
    }
  }

  TextStream &TextStream::operator<<(bool b) noexcept
  {
    return *this << (b ? "1" : "0");
  }

  TextStream &TextStream::operator<<(char c) noexcept
  {
    _text.Append(c);
    return *this;
  }

  TextStream &TextStream::operator<<(char8 c) noexcept
  {
    _text.Append(c);
    return *this;
  }

  TextStream &TextStream::operator<<(int32 i) noexcept
  {
    _text.Append(i);
    return *this;
  }

  TextStream &TextStream::operator<<(uint32 i) noexcept
  {
    _text.Append(i);
    return *this;
  }

  TextStream &TextStream::operator<<(long i) noexcept
  {
    _text.Append(i);
    return *this;
  }

  TextStream &TextStream::operator<<(unsigned long i) noexcept
  {
    _text.Append(i);
    return *this;
  }

  TextStream &TextStream::operator<<(long long i) noexcept
  {
    _text.Append(i);
    return *this;
  }

  TextStream &TextStream::operator<<(unsigned long long i) noexcept
  {
    _text.Append(i);
    return *this;
  }

  TextStream &TextStream::operator<<(float f) noexcept
  {
    if (_formattingFlags.Contains(Formatting::NumberRespectingIntegers))
    {
      return *this << FormatNumberRespectingIntegers(f);
    }
    AppendFixedWidth(_text, f, 2);
    return *this;
  }

  TextStream &TextStream::operator<<(double d) noexcept
  {
    if (_formattingFlags.Contains(Formatting::NumberRespectingIntegers))
    {
      return *this << FormatNumberRespectingIntegers(d);
    }
    AppendFixedWidth(_text, d, 2);
    return *this;
  }

  TextStream &TextStream::operator<<(const char *string) noexcept
  {
    _text.Append(std::string_view {string});
    return *this;
  }

  TextStream &TextStream::operator<<(std::string_view string) noexcept
  {
    _text.Append(string);
    return *this;
  }

  TextStream &TextStream::operator<<(const char8 *string) noexcept
  {
    _text.Append(utf8_stringview {string});
    return *this;
  }

  TextStream &TextStream::operator<<(utf8_stringview string) noexcept
  {
    _text.Append(string);
    return *this;
  }

  TextStream &TextStream::operator<<(const utf8_string &string) noexcept
  {
    _text.Append(string);
    return *this;
  }

  TextStream &TextStream::operator<<(const void *p) noexcept
  {
    char buffer[100];
    std::snprintf(buffer, sizeof(buffer), "%p", p);
    return *this << static_cast<const char *>(buffer);
  }

  TextStream &TextStream::operator<<(const FormatNumberRespectingIntegers &numberToFormat) noexcept
  {
    if (HasFractions(numberToFormat.Value))
    {
      AppendFixedWidth(_text, numberToFormat.Value, 2);
      return *this;
    }
    _text.Append(static_cast<int32>(numberToFormat.Value));
    return *this;
  }

  utf8_string TextStream::Release() noexcept
  {
    utf8_string result = krys::move(_text).ToString();
    _text.Clear();
    return result;
  }

  void TextStream::StartGroup() noexcept
  {
    TextStream &ts = *this;
    if (_multiLineMode)
    {
      ts << '\n';
      ts.WriteIndent();
      ts << '(';
      ts.IncreaseIndent();
    }
    else
    {
      ts << " (";
    }
  }

  void TextStream::EndGroup() noexcept
  {
    TextStream &ts = *this;
    ts << ')';
    if (_multiLineMode)
    {
      ts.DecreaseIndent();
    }
  }

  void TextStream::NextLine() noexcept
  {
    TextStream &ts = *this;
    if (_multiLineMode)
    {
      ts << '\n';
      ts.WriteIndent();
    }
    else
    {
      ts << ' ';
    }
  }

  void TextStream::WriteIndent() noexcept
  {
    if (_multiLineMode)
    {
      krys::WriteIndent(*this, _indent);
    }
  }

  void WriteIndent(TextStream &ts, int32 indent) noexcept
  {
    for (int32 i = 0; i < indent; ++i)
    {
      ts << "  ";
    }
  }
}
