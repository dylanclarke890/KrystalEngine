#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
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
      RawPtr<NodeList> _list {};
      size_t _index {};

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = RefPtr<Node>;
      using difference_type = std::ptrdiff_t;

      Iterator(RawPtr<NodeList> list, size_t index) : _list(list), _index(index)
      {
      }

      RefPtr<Node> operator*() const
      {
        return _list->Item(_index);
      }

      Iterator &operator++()
      {
        ++_index;
        return *this;
      }

      bool operator==(const Iterator &other) const
      {
        return _list == other._list && _index == other._index;
      }

      bool operator!=(const Iterator &other) const
      {
        return !(*this == other);
      }
      /// @see https://webidl.spec.whatwg.org/#dfn-iterable
    };

  public:
    virtual ~NodeList() noexcept = default;

#pragma region NodeList - https://dom.spec.whatwg.org/#interface-nodelist

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
      return Iterator(this, 0);
    }

    /// @see https://webidl.spec.whatwg.org/#dfn-iterable
    Iterator end() noexcept
    {
      return Iterator(this, Length());
    }

  protected:
    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-indices
    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }

#pragma endregion

#pragma region Type Checks

    KRYS_NODISCARD virtual bool IsLiveNodeList() const noexcept
    {
      return false;
    }

#pragma endregion
  };
}
