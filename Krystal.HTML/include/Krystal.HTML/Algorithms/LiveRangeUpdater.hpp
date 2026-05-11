#pragma once

namespace Krys::HTML
{
  class Node;
  class CharacterData;
  class ContainerNode;
  class Text;

  class LiveRangeUpdater
  {
  public:
    static void PreRemove(const Node &node) noexcept;

    static void CharacterDataReplaced(CharacterData &characterData, size_t offset, size_t count,
                                      size_t newDataSize) noexcept;

    static void NodeNormalized(Node &node, Text &removedText, size_t length) noexcept;
    static void MovedBeforeNode(Node &movedBefore) noexcept;

    static void InsertedBeforeNode(Node &insertedBefore, size_t nodesInserted) noexcept;

    static void SplitTextNode(Text &original, Text &newNode, size_t offset) noexcept;
  };
}