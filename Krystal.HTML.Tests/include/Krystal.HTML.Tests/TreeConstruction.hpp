#pragma once

#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
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

        Indent(output, depth);

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

    return output;
  }

  inline DOMString Dump(const DocumentFragment &fragment) noexcept
  {
    DOMString output = u8"#document-fragment\n";

    for (auto &child : ConstChildNodeRange(fragment))
    {
      DumpNode(child, output, 0uz);
    }

    return output;
  }

  struct TreeConstructionTest
  {
    utf32_string Input;
    DOMString Expected;
    Maybe<bool> ScriptingEnabled;
  };

  inline List<TreeConstructionTest> ParseTreeConstructionTests(std::istream &stream) noexcept
  {
    auto ToUTF8 = [](const string &s) -> DOMString
    {
      return DOMString(reinterpret_cast<const char8_t *>(s.data()), s.size());
    };

    enum class Section
    {
      None,
      Data,
      Errors,
      ExpectedOutput
    };

    List<TreeConstructionTest> tests;
    string input;
    string expected;
    Maybe<bool> scriptingEnabled;

    auto FinishParsingTest = [&]()
    {
      auto data = Krys::Text::ConvertToUTF32(utf8_stringview(ToUTF8(input)));
      if (!data.empty() && data.back() == U'\n')
      {
        data.pop_back(); // Remove the trailing newline from the input
      }

      tests.push_back({
        .Input = Krys::Move(data),
        .Expected = ToUTF8(expected),
        .ScriptingEnabled = scriptingEnabled,
      });

      input.clear();
      expected.clear();
    };

    Section section = Section::None;
    string line;
    while (std::getline(stream, line))
    {
      if (!line.empty() && line.back() == '\r')
      {
        line.pop_back();
      }

      if (line.empty())
      {
        continue;
      }

      if (line == "#data")
      {
        if (!input.empty() && !expected.empty())
        {
          FinishParsingTest();
        }
        section = Section::Data;
        continue;
      }

      if (line == "#errors" || line == "#new-errors" || line == "#errors-new")
      {
        section = Section::Errors;
        continue;
      }

      if (line.starts_with("#document-fragment"))
      {
        section = Section::ExpectedOutput;
        expected = "#document-fragment\n";
        continue;
      }

      if (line == "#document")
      {
        section = Section::ExpectedOutput;
        expected = "#document\n";
        continue;
      }

      if (line == "#script-on")
      {
        scriptingEnabled = true;
        continue;
      }

      if (line == "#script-off")
      {
        scriptingEnabled = false;
        continue;
      }

      if (line[0] == '#')
      {
        section = Section::None;
        continue; // Ignore unknown sections
      }

      if (section == Section::Data)
      {
        input += line;
        input += '\n';
        continue;
      }

      if (section == Section::ExpectedOutput)
      {
        expected += line;
        expected += '\n';
        continue;
      }

      if (section == Section::Errors)
      {
      }
    }

    if (section != Section::None)
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
