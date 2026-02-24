#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  enum class NodeListType : uint8
  {
    Live,
    ChildNode,
    Static,
    Empty,
  };

  enum NodeListFlag : uint8
  {
    None = 0,
    ContainsOnlyElements = 1 << 0
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::NodeListType, 4);
KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::NodeListFlag, 2);

namespace Krys::HTML
{
  class Element;
  class Node;

  class NodeList : public RefCounted<NodeList>
  {
  private:
    NodeListType _type : BitCount<NodeListType>();
    NodeListFlag _flags : BitCount<NodeListFlag>();

  protected:
    explicit NodeList(NodeListType type, NodeListFlag flags) noexcept : _type(type), _flags(flags)
    {
    }

  public:
    virtual ~NodeList() noexcept = default;

    KRYS_NODISCARD virtual RawPtr<Node> Item(size_t index) const noexcept = 0;

    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }

#pragma region Type Checks

    KRYS_NODISCARD bool IsLiveNodeList() const noexcept
    {
      return _type == NodeListType::Live;
    }

    KRYS_NODISCARD bool IsChildNodeList() const noexcept
    {
      return _type == NodeListType::ChildNode;
    }

    KRYS_NODISCARD bool IsStaticNodeList() const noexcept
    {
      return _type == NodeListType::Static && !HasFlag(_flags, NodeListFlag::ContainsOnlyElements);
    }

    KRYS_NODISCARD bool IsStaticElementList() const noexcept
    {
      return _type == NodeListType::Static && HasFlag(_flags, NodeListFlag::ContainsOnlyElements);
    }

    KRYS_NODISCARD bool IsEmptyNodeList() const noexcept
    {
      return _type == NodeListType::Empty;
    }

#pragma endregion
  };
}