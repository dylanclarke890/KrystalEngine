#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <fstream>
#include <sstream>
#include <string>

namespace Krys::HTML::Tests
{
  struct TreeConstructionTest
  {
    utf32_string Input;
    DOMString ExpectedDocument;
  };

  inline List<TreeConstructionTest> ParseTreeConstructionTests(std::istream &stream) noexcept
  {
    List<TreeConstructionTest> tests;

    enum class Section
    {
      None,
      Data,
      Errors,
      Document
    };

    std::string line;
    Section section = Section::None;
    std::string currentInput;
    std::string currentDocumentRaw;

    auto toUTF8 = [](const std::string &s) -> DOMString
    {
      return DOMString(reinterpret_cast<const char8_t *>(s.data()), s.size());
    };

    auto finalizeTest = [&]()
    {
      if (!currentInput.empty() && !currentDocumentRaw.empty())
      {
        DOMString data = toUTF8(currentInput);
        tests.push_back({
          .Input = Krys::Text::ConvertToUTF32(utf8_stringview(data)),
          .ExpectedDocument = toUTF8(currentDocumentRaw),
        });
      }
      currentInput.clear();
      currentDocumentRaw.clear();
    };

    while (std::getline(stream, line))
    {
      if (!line.empty() && line.back() == '\r')
        line.pop_back();

      if (line == "#data")
      {
        finalizeTest();
        section = Section::Data;
      }
      else if (line == "#errors" || line == "#new-errors")
      {
        section = Section::Errors;
      }
      else if (line == "#document")
      {
        section = Section::Document;
        currentDocumentRaw = "#document\n";
      }
      else if (!line.empty() && line[0] == '#')
      {
        section = Section::Errors;
      }
      else if (line.empty())
      {
        if (section == Section::Document)
        {
          finalizeTest();
        }
        section = Section::None;
      }
      else if (section == Section::Data)
      {
        currentInput += line;
        currentInput += '\n';
      }
      else if (section == Section::Document)
      {
        currentDocumentRaw += line;
        currentDocumentRaw += '\n';
      }
    }

    if (section == Section::Document)
    {
      finalizeTest();
    }

    return tests;
  }

  inline Maybe<List<TreeConstructionTest>> ParseTreeConstructionTests(const string &filePath) noexcept
  {
    std::ifstream file(filePath);
    if (!file.is_open())
    {
      return {};
    }

    return ParseTreeConstructionTests(file);
  }
}
