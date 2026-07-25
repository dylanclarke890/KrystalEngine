#pragma once

#include "Krystal.HTML.Tests/TestParsers/TestParserUtils.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLParseError.hpp"
#include "Krystal.HTML/HTML/Enums/TokenizerState.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.Lib/Core/MagicEnum.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/StringConversion.hpp"

namespace Krys::HTML::Tests
{
  struct HTMLTokenizerTest
  {
    string Name;
    utf8_string Html;
    utf8_string Tokens;
    TokenizerState InitialState {TokenizerState::Data};
    TokenizerState ExpectedState {TokenizerState::Data};
    bool AppendEOF {false};
    bool CDATASectionAllowed {false};
    List<HTMLTokenizerError> Errors {};
  };

  enum class HTMLTokenizerTestSection
  {
    None,
    Name,
    Data,
    InitialState,
    ExpectedState,
    Tokens,
    Errors,
  };

  inline void SerializeHTMLToken(HTMLToken &token, DOMString &output) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::DOCTYPE:
      {
        auto doctype = token.ReleaseDOCTYPEData();
        output += u8"DOCTYPE(";
        output += token.Name();
        output += u8", ";
        output += DOMStringView(doctype->PublicIdentifier);
        output += u8", ";
        output += DOMStringView(doctype->SystemIdentifier);
        output += u8", ";
        output += u8"ForceQuirks=";
        output += doctype->ForceQuirks ? u8"true" : u8"false";
        output += u8")\n";
        break;
      }
      case HTMLTokenType::Character:
      {
        output += u8"Character(";
        output += DOMStringView(token.Data());
        output += u8")\n";
        break;
      }
      case HTMLTokenType::Comment:
      {
        output += u8"Comment(";
        output += DOMStringView(token.Data());
        output += u8")\n";
        break;
      }
      case HTMLTokenType::StartTag:
      case HTMLTokenType::EndTag:
      {
        if (token.Type() == HTMLTokenType::StartTag)
        {
          output += u8"StartTag(";
        }
        else
        {
          output += u8"EndTag(";
        }

        output += token.Name();

        for (const auto &attribute : token.Attributes())
        {
          output += u8", ";
          output += attribute.NameView();
          output += u8"=\"";
          output += attribute.ValueView();
          output += u8"\"";
        }

        if (token.Type() == HTMLTokenType::StartTag)
        {
          output += u8", SelfClosing=";
          output += token.IsSelfClosing() ? u8"true" : u8"false";
        }

        output += u8")\n";
        break;
      }
      case HTMLTokenType::EndOfFile:
      {
        output += u8"EndOfFile\n";
        break;
      }
    }
  }

  KRYS_NODISCARD inline List<HTMLTokenizerTest> ParseHTMLTokenizerTests(std::istream &stream) noexcept
  {
    List<HTMLTokenizerTest> tests;

    string name;
    string html;
    string tokens;
    TokenizerState initialState {TokenizerState::Data};
    TokenizerState expectedState {TokenizerState::Data};
    bool appendEOF {false};
    bool cDATASectionAllowed {false};
    List<HTMLTokenizerError> errors;

    auto FinishParsingTest = [&]()
    {
      NormaliseData(name);
      NormaliseData(html);
      NormaliseData(tokens);

      tests.push_back({
        .Name = name,
        .Html = ToUTF8(html),
        .Tokens = ToUTF8(tokens),
        .InitialState = initialState,
        .ExpectedState = expectedState,
        .AppendEOF = appendEOF,
        .CDATASectionAllowed = cDATASectionAllowed,
        .Errors = Krys::Move(errors),
      });

      html.clear();
      tokens.clear();
      initialState = TokenizerState::Data;
      expectedState = TokenizerState::Data;
      appendEOF = false;
      cDATASectionAllowed = false;
    };

    string line;
    HTMLTokenizerTestSection section = HTMLTokenizerTestSection::None;
    while (std::getline(stream, line))
    {
      if (!line.empty() && line.back() == '\r')
      {
        line.pop_back();
      }

      if (line == "#name")
      {
        NormaliseData(tokens);
        if (!html.empty())
        {
          FinishParsingTest();
        }

        section = HTMLTokenizerTestSection::Name;
        continue;
      }

      if (line == "#data")
      {
        section = HTMLTokenizerTestSection::Data;
        continue;
      }

      if (line == "#errors")
      {
        section = HTMLTokenizerTestSection::Errors;
        continue;
      }

      if (line == "#tokens")
      {
        section = HTMLTokenizerTestSection::Tokens;
        continue;
      }

      if (line == "#initial-state")
      {
        section = HTMLTokenizerTestSection::InitialState;
        continue;
      }

      if (line == "#expected-state")
      {
        section = HTMLTokenizerTestSection::ExpectedState;
        continue;
      }

      if (line == "#append-eof")
      {
        appendEOF = true;
        continue;
      }

      if (line == "#cdata-allowed")
      {
        cDATASectionAllowed = true;
        continue;
      }

      switch (section)
      {
        case HTMLTokenizerTestSection::None:
        {
          continue;
        }
        case HTMLTokenizerTestSection::Name:
        {
          name = line;
          continue;
        }
        case HTMLTokenizerTestSection::Data:
        {
          html += line;
          html += '\n';
          continue;
        }
        case HTMLTokenizerTestSection::Tokens:
        {
          tokens += line;
          tokens += '\n';
          continue;
        }
        case HTMLTokenizerTestSection::InitialState:
        {
          auto state = magic_enum::enum_cast<TokenizerState>(line, magic_enum::case_insensitive);
          if (state.has_value())
          {
            initialState = *state;
          }
          continue;
        }
        case HTMLTokenizerTestSection::ExpectedState:
        {
          auto state = magic_enum::enum_cast<TokenizerState>(line, magic_enum::case_insensitive);
          if (state.has_value())
          {
            expectedState = *state;
          }
          continue;
        }
        case HTMLTokenizerTestSection::Errors:
        {
          // NOTE: errors are in the form "line:column ErrorName"
          if (line.empty())
          {
            continue;
          }
          auto text = stringview(line);

          uint32 line = 0u;
          auto [ptr1, ec1] = std::from_chars(text.data(), text.data() + text.size(), line);
          text.remove_prefix(ptr1 - text.data());

          text.remove_prefix(1uz); // skip ':'

          uint32 column = 0u;
          auto [ptr2, ec2] = std::from_chars(text.data(), text.data() + text.size(), column);
          text.remove_prefix(ptr2 - text.data());

          text.remove_prefix(1uz); // skip ' '

          auto error = magic_enum::enum_cast<HTMLParseError>(text, magic_enum::case_insensitive);
          if (error.has_value())
          {
            errors.push_back({
              .Error = *error,
              .Location =
                {
                  .Line = line,
                  .Column = column,
                },
            });
          }
          continue;
        }
      }
    }

    if (!html.empty())
    {
      FinishParsingTest();
    }

    return tests;
  }

  KRYS_NODISCARD inline Maybe<List<HTMLTokenizerTest>>
    ParseHTMLTokenizerTests(const string &filePath) noexcept
  {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
      return {};
    }

    SkipUTF8FileBOM(file);
    return ParseHTMLTokenizerTests(file);
  }
}