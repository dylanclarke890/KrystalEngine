#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::web
{
  class Node;

  /// @see https://dom.spec.whatwg.org/#interface-nodelist
  class NodeList : public RefCounted<NodeList>, public CanMakeWeakPtr<NodeList>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();

  private:
    /// @see https://webidl.spec.whatwg.org/#dfn-iterable
    class Iterator
    {
    private:
      NodeList *_list {};
      size_t _index {};

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = RefPtr<Node>;
      using difference_type = std::ptrdiff_t;

      Iterator(NodeList *list, size_t index) noexcept : _list(list), _index(index)
      {
      }

      KRYS_NODISCARD RefPtr<Node> operator*() const noexcept
      {
        return _list->Item(_index);
      }

      Iterator &operator++() noexcept
      {
        ++_index;
        return *this;
      }

      bool operator==(const Iterator &other) const noexcept
      {
        return _list == other._list && _index == other._index;
      }

      bool operator!=(const Iterator &other) const noexcept
      {
        return !(*this == other);
      }
    };

  public:
    virtual ~NodeList() noexcept = default;

#pragma region NodeList - https://dom.spec.whatwg.org/#nodelist

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD virtual RefPtr<Node> Item(size_t index) noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD virtual RefPtr<const Node> Item(size_t index) const noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD virtual RefPtr<Node> operator[](size_t index) noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD virtual RefPtr<const Node> operator[](size_t index) const noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-length
    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;

    /// @see https://webidl.spec.whatwg.org/#dfn-iterable
    Iterator begin() noexcept
    {
      return Iterator(this, 0uz);
    }

    /// @see https://webidl.spec.whatwg.org/#dfn-iterable
    Iterator end() noexcept
    {
      return Iterator(this, Length());
    }

#pragma endregion

    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-indices
    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }

  protected:
    KRYS_NODISCARD virtual bool IsLiveNodeList() const noexcept
    {
      return false;
    }
  };
}
