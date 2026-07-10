#pragma once

#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/Enums/ParserScriptingMode.hpp"
#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <fstream>
#include <ranges>
#include <sstream>
#include <string>

namespace Krys::HTML::Tests
{
  struct TreeConstructionTest
  {
    utf32_string Input;
    DOMString Expected;
    Maybe<DOMString> FragmentContext;
    Maybe<ParserScriptingMode> ScriptingMode;
  };

  enum class TreeConstructionTestSection
  {
    None,
    Data,
    Errors,
    FragmentContext,
    ExpectedOutput
  };

  inline DOMString ToUTF8(const string &s) noexcept
  {
    return DOMString(reinterpret_cast<const char8_t *>(s.data()), s.size());
  };

  template <typename T>
  inline void NormaliseData(T &str) noexcept
  {
    if (!str.empty() && str.back() == U'\n')
    {
      str.pop_back();
    }
  }

  inline void Indent(DOMString &output, size_t depth) noexcept
  {
    output += u8"| ";

    for (size_t i = 0uz; i < depth; ++i)
    {
      output += u8"  ";
    }
  }

  inline void DumpNode(const Node &node, DOMString &output, size_t depth) noexcept
  {
    switch (node.NodeType())
    {
      case NodeType::ELEMENT_NODE:
      {
        auto &element = Downcast<Element>(node);

        Indent(output, depth);

        auto localName = DOMString(element.LocalName().View());
        auto namespaceName = [&] -> DOMString
        {
          if (element.NamespaceURI() == Namespaces::HTML)
          {
            return u8"";
          }
          else if (element.NamespaceURI() == Namespaces::SVG)
          {
            return u8"svg ";
          }
          else if (element.NamespaceURI() == Namespaces::MathML)
          {
            return u8"math ";
          }
          else
          {
            return u8"";
          }
        }();

        output += u8"<" + namespaceName + localName + u8">\n";

        // TODO: handle namespaces and prefixes.
        auto attributeNames = element.GetAttributeNames();
        std::sort(attributeNames.begin(), attributeNames.end(), std::ranges::lexicographical_compare);

        for (auto &name : attributeNames)
        {
          auto value = element.GetAttribute(name);
          if (value.has_value())
          {
            Indent(output, depth + 1uz);
            output += name + u8"=\"" + *value + u8"\"\n";
          }
        }

        if (element.NamespaceURI() == Namespaces::HTML && element.LocalName() == u8"template")
        {
          Indent(output, depth + 1uz);
          output += u8"content\n";

          for (auto &child : ConstChildNodeRange(*Downcast<HTMLTemplateElement>(element).Content()))
          {
            DumpNode(child, output, depth + 2uz);
          }
        }
        break;
      }
      case NodeType::TEXT_NODE:
      {
        Indent(output, depth);
        output += u8'"' + Downcast<HTML::Text>(node).Data() + u8"\"\n";
        break;
      }
      case NodeType::COMMENT_NODE:
      {
        Indent(output, depth);
        output += u8"<!-- " + Downcast<Comment>(node).Data() + u8" -->\n";
        break;
      }
      case NodeType::DOCUMENT_TYPE_NODE:
      {
        auto &documentType = Downcast<DocumentType>(node);

        Krys::HTML::Tests::Indent(output, depth);

        output += u8"<!DOCTYPE ";
        output += documentType.Name();

        if (!documentType.PublicId().empty() || !documentType.SystemId().empty())
        {
          output += u8" \"";
          output += documentType.PublicId();
          output += u8"\" \"";
          output += documentType.SystemId();
          output += u8'"';
        }

        output += u8">\n";
        break;
      }
    }

    if (auto *containerNode = DynamicDowncast<ContainerNode>(node))
    {
      for (auto &child : ConstChildNodeRange(*containerNode))
      {
        DumpNode(child, output, depth + 1uz);
      }
    }
  }

  inline DOMString Dump(const Document &document) noexcept
  {
    DOMString output = u8"#document\n";

    for (auto &child : ConstChildNodeRange(document))
    {
      DumpNode(child, output, 0uz);
    }

    if (!output.empty() && output.back() == u8'\n')
    {
      output.pop_back();
    }

    return output;
  }

  inline List<TreeConstructionTest> ParseTreeConstructionTests(std::istream &stream) noexcept
  {
    List<TreeConstructionTest> tests;

    string input;
    string expected;
    Maybe<string> fragmentContext;
    Maybe<ParserScriptingMode> scriptingMode;

    auto FinishParsingTest = [&]()
    {
      NormaliseData(input);
      NormaliseData(expected);

      if (fragmentContext.has_value())
      {
        NormaliseData(*fragmentContext);
      }

      tests.push_back({
        .Input = Krys::Text::ConvertToUTF32(utf8_stringview(ToUTF8(input))),
        .Expected = ToUTF8(expected),
        .FragmentContext = fragmentContext.has_value() ? ToUTF8(*fragmentContext) : Maybe<DOMString>(Null),
        .ScriptingMode = scriptingMode,
      });

      input.clear();
      expected.clear();
      fragmentContext.reset();
      scriptingMode.reset();
    };

    string line;
    TreeConstructionTestSection section = TreeConstructionTestSection::None;
    while (std::getline(stream, line))
    {
      if (!line.empty() && line.back() == '\r')
      {
        line.pop_back();
      }

      if (line == "#data")
      {
        NormaliseData(expected);

        if (!input.empty() && !expected.empty())
        {
          FinishParsingTest();
        }

        section = TreeConstructionTestSection::Data;
        continue;
      }

      if (line == "#errors" || line == "#new-errors" || line == "#errors-new")
      {
        section = TreeConstructionTestSection::Errors;
        continue;
      }

      if (line.starts_with("#document-fragment"))
      {
        section = TreeConstructionTestSection::FragmentContext;
        continue;
      }

      if (line == "#document")
      {
        section = TreeConstructionTestSection::ExpectedOutput;
        expected = "#document\n";
        continue;
      }

      if (line == "#script-on")
      {
        if (!fragmentContext.has_value())
        {
          scriptingMode = ParserScriptingMode::Normal;
        }

        continue;
      }

      if (line == "#script-off")
      {
        if (!fragmentContext.has_value())
        {
          scriptingMode = ParserScriptingMode::Disabled;
        }

        continue;
      }

      if (line[0] == '#')
      {
        section = TreeConstructionTestSection::None;
        continue; // Ignore unknown sections
      }

      switch (section)
      {
        case TreeConstructionTestSection::None:
        {
          continue;
        }
        case TreeConstructionTestSection::Data:
        {
          input += line;
          input += '\n';
          continue;
        }
        case TreeConstructionTestSection::ExpectedOutput:
        {
          expected += line;
          expected += '\n';
          continue;
        }
        case TreeConstructionTestSection::FragmentContext:
        {
          if (!fragmentContext.has_value())
          {
            fragmentContext = line;
          }
          else
          {
            *fragmentContext += '\n';
            *fragmentContext += line;
          }
          continue;
        }
        case TreeConstructionTestSection::Errors:
        {
          // TODO: parse errors
          continue;
        }
      }
    }

    if (section != TreeConstructionTestSection::None)
    {
      FinishParsingTest();
    }

    return tests;
  }

  inline Maybe<List<TreeConstructionTest>> ParseTreeConstructionTests(const string &filePath) noexcept
  {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
      return {};
    }

    char bom[3];

    file.read(bom, 3);

    if (file.gcount() == 3 && static_cast<unsigned char>(bom[0]) == 0xEF
        && static_cast<unsigned char>(bom[1]) == 0xBB && static_cast<unsigned char>(bom[2]) == 0xBF)
    {
      // BOM consumed; continue reading
    }
    else
    {
      file.clear();  // clear eof/fail if we hit it
      file.seekg(0); // rewind to beginning
    }

    return ParseTreeConstructionTests(file);
  }
}
