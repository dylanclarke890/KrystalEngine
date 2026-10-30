#include "Krystal.Booey/DOM/FragmentDirectiveParser.hpp"
#include "Krystal.Booey/URL/URLParser.hpp"

namespace krys::boo::dom
{
  namespace
  {
    /// @brief The parts of @p string between each @p separator, empty ones included.
    KRYS_NODISCARD List<utf8_stringview> Split(utf8_stringview string, char8 separator) noexcept
    {
      List<utf8_stringview> parts;
      size_t start = 0;
      while (true)
      {
        auto end = string.find(separator, start);
        if (end == utf8_stringview::npos)
        {
          parts.push_back(string.substr(start));
          return parts;
        }
        parts.push_back(string.substr(start, end - start));
        start = end + 1;
      }
    }
  }

  FragmentDirectiveParser::FragmentDirectiveParser(utf8_stringview fragmentDirective) noexcept
  {
    ParseFragmentDirective(fragmentDirective);

    _fragmentDirective = fragmentDirective;
    _isValid = true;
  }

  // https://wicg.github.io/scroll-to-text-fragment/#parse-a-text-directive
  void FragmentDirectiveParser::ParseFragmentDirective(utf8_stringview fragmentDirective) noexcept
  {
    List<ParsedTextDirective> parsedTextDirectives;
    constexpr utf8_stringview textDirectivePrefix = u8"text=";

    for (auto directive : Split(fragmentDirective, u8'&'))
    {
      if (!directive.starts_with(textDirectivePrefix))
      {
        continue;
      }

      auto textDirective = directive.substr(textDirectivePrefix.size());

      List<utf8_string> tokens;
      bool containsEmptyToken = false;
      for (auto token : Split(textDirective, u8','))
      {
        if (token.empty())
        {
          containsEmptyToken = true;
          break;
        }
        tokens.emplace_back(token);
      }
      if (containsEmptyToken)
      {
        continue;
      }
      if (tokens.size() > 4 || tokens.size() < 1)
      {
        continue;
      }

      ParsedTextDirective parsedTextDirective;

      if (tokens.front().ends_with(u8'-') && tokens.front().size() > 1)
      {
        auto takenFirstToken = krys::move(tokens.front());
        tokens.erase(tokens.begin());
        if (auto prefix = url::URLParser::FormURLDecode(
              utf8_stringview(takenFirstToken).substr(0, takenFirstToken.size() - 1)))
        {
          parsedTextDirective.Prefix = krys::move(*prefix);
        }
      }

      if (tokens.empty())
      {
        continue;
      }

      if (tokens.back().starts_with(u8'-') && tokens.back().size() > 1)
      {
        auto suffixToken = tokens.back().substr(1);
        tokens.pop_back();
        if (auto suffix = url::URLParser::FormURLDecode(suffixToken))
        {
          parsedTextDirective.Suffix = krys::move(*suffix);
        }
      }

      if (tokens.size() != 1 && tokens.size() != 2)
      {
        continue;
      }

      if (auto start = url::URLParser::FormURLDecode(tokens.front()))
      {
        parsedTextDirective.StartText = krys::move(*start);
      }

      if (tokens.size() == 2)
      {
        if (auto end = url::URLParser::FormURLDecode(tokens.back()))
        {
          parsedTextDirective.EndText = krys::move(*end);
        }
      }

      parsedTextDirectives.push_back(krys::move(parsedTextDirective));
    }

    _parsedTextDirectives = krys::move(parsedTextDirectives);
  }
}
