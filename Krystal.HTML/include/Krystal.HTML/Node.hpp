#pragma once

#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  enum class NodeType : uint8
  {
    ELEMENT_NODE = 1,
    ATTRIBUTE_NODE = 2,
    TEXT_NODE = 3,
    CDATA_SECTION_NODE = 4,
    PROCESSING_INSTRUCTION_NODE = 7,
    COMMENT_NODE = 8,
    DOCUMENT_NODE = 9,
    DOCUMENT_TYPE_NODE = 10,
    DOCUMENT_FRAGMENT_NODE = 11,
  };

  enum class DeprecatedNodeType : uint8
  {
    ENTITY_REFERENCE_NODE = 5,
    ENTITY_NODE = 6,
    NOTATION_NODE = 12,
  };

  enum class DocumentPosition : uint8
  {
    DOCUMENT_POSITION_EQUIVALENT = 0x00,
    DOCUMENT_POSITION_DISCONNECTED = 0x01,
    DOCUMENT_POSITION_PRECEDING = 0x02,
    DOCUMENT_POSITION_FOLLOWING = 0x04,
    DOCUMENT_POSITION_CONTAINS = 0x08,
    DOCUMENT_POSITION_CONTAINED_BY = 0x10,
    DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20,
  };
}

namespace Krys::HTML
{
  class ContainerNode;

  class Node : public EventTarget
  {
    friend class Document;

  public:
  };
}