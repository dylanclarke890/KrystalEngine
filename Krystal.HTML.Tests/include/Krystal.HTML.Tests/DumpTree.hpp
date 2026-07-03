#pragma once

#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Text.hpp"

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
        output += u8"<" + DOMString(element.LocalName().View()) + u8">\n";

        auto attributeNames = element.GetAttributeNames();
        for (auto &name : attributeNames)
        {
          auto value = element.GetAttribute(name);
          if (value.has_value())
          {
            Indent(output, depth + 1uz);
            output += name + u8"=\"" + *value + u8"\"\n";
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
}