#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::svg
{
  /// @brief The list interface the SVG DOM's lists share (`SVGLengthList`, `SVGStringList`, ...).
  /// @see https://svgwg.org/svg2-draft/types.html#ListInterfaces
  template <typename TItem>
  class SVGList : public SVGProperty
  {
  protected:
    List<TItem> _items;

  public:
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return NumberOfItems();
    }

    KRYS_NODISCARD uint32 NumberOfItems() const noexcept
    {
      return static_cast<uint32>(_items.size());
    }

    dom::ExceptionOr<void> Clear() noexcept
    {
      auto result = CanAlterList();
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      krys_assert(result.ReleaseValue());

      ClearItems();
      CommitChange();
      return {};
    }

    dom::ExceptionOr<TItem> GetItem(uint32 index) noexcept
    {
      auto result = CanGetItem(index);
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      krys_assert(result.ReleaseValue());

      return At(index);
    }

    dom::ExceptionOr<TItem> Initialize(TItem &&newItem) noexcept
    {
      auto result = CanAlterList();
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      // Spec: Clears all existing current items from the list.
      ClearItems();

      auto item = Append(krys::move(newItem));
      CommitChange();
      return item;
    }

    dom::ExceptionOr<TItem> InsertItemBefore(TItem &&newItem, uint32 index) noexcept
    {
      auto result = CanAlterList();
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      krys_assert(result.ReleaseValue());

      // Spec: If the index is greater than or equal to numberOfItems,
      // then the new item is appended to the end of the list.
      if (index > NumberOfItems())
      {
        index = NumberOfItems();
      }

      auto item = InsertAt(index, krys::move(newItem));
      CommitChange();
      return item;
    }

    dom::ExceptionOr<TItem> ReplaceItem(TItem &&newItem, uint32 index) noexcept
    {
      auto result = CanReplaceItem(index);
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      krys_assert(result.ReleaseValue());

      auto item = ReplaceAt(index, krys::move(newItem));
      CommitChange();
      return item;
    }

    dom::ExceptionOr<TItem> RemoveItem(uint32 index) noexcept
    {
      auto result = CanRemoveItem(index);
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      krys_assert(result.ReleaseValue());

      auto item = RemoveAt(index);
      CommitChange();
      return item;
    }

    dom::ExceptionOr<TItem> AppendItem(TItem &&newItem) noexcept
    {
      auto result = CanAlterList();
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      krys_assert(result.ReleaseValue());

      auto item = Append(krys::move(newItem));
      CommitChange();
      return item;
    }

    dom::ExceptionOr<void> SetItem(uint32 index, TItem &&newItem) noexcept
    {
      auto result = ReplaceItem(krys::move(newItem), index);
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      return {};
    }

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < _items.size();
    }

    // Parsers and animators need to have a direct access to the items.
    KRYS_NODISCARD List<TItem> &Items() noexcept
    {
      return _items;
    }

    KRYS_NODISCARD const List<TItem> &Items() const noexcept
    {
      return _items;
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _items.size();
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _items.empty();
    }

    void ClearItems() noexcept
    {
      DetachItems();
      _items.clear();
    }

  protected:
    using SVGProperty::SVGProperty;

    KRYS_NODISCARD dom::ExceptionOr<bool> CanAlterList() const noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }
      return true;
    }

    KRYS_NODISCARD dom::ExceptionOr<bool> CanGetItem(uint32 index) noexcept
    {
      if (index >= _items.size())
      {
        return dom::Exception {dom::ExceptionCode::IndexSizeError};
      }
      return true;
    }

    KRYS_NODISCARD dom::ExceptionOr<bool> CanReplaceItem(uint32 index) noexcept
    {
      auto result = CanAlterList();
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      krys_assert(result.ReleaseValue());

      if (index >= _items.size())
      {
        return dom::Exception {dom::ExceptionCode::IndexSizeError};
      }
      return true;
    }

    KRYS_NODISCARD dom::ExceptionOr<bool> CanRemoveItem(uint32 index) noexcept
    {
      auto result = CanAlterList();
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      krys_assert(result.ReleaseValue());

      if (index >= _items.size())
      {
        return dom::Exception {dom::ExceptionCode::IndexSizeError};
      }
      return true;
    }

    virtual void DetachItems() noexcept
    {
    }
    KRYS_NODISCARD virtual TItem At(uint32 index) const noexcept = 0;
    virtual TItem InsertAt(uint32 index, TItem &&) noexcept = 0;
    virtual TItem ReplaceAt(uint32 index, TItem &&) noexcept = 0;
    virtual TItem RemoveAt(uint32 index) noexcept = 0;
    virtual TItem Append(TItem &&) noexcept = 0;
  };
}
