#pragma once

#include "Krystal.Booey/SVG/Properties/SVGList.hpp"

namespace krys::boo::svg
{
  /// @brief A list of plain values, such as the strings of an `SVGStringList`.
  template <typename TProperty>
  class SVGPrimitiveList : public SVGList<TProperty>
  {
  protected:
    using Base = SVGList<TProperty>;
    using Base::_items;
    using Base::Base;
    using Base::Size;

    KRYS_NODISCARD TProperty At(uint32 index) const noexcept override
    {
      krys_assert(index < Size());
      return _items.at(index);
    }

    TProperty InsertAt(uint32 index, TProperty &&newItem) noexcept override
    {
      krys_assert(index <= Size());
      _items.insert(_items.begin() + index, krys::move(newItem));
      return At(index);
    }

    TProperty ReplaceAt(uint32 index, TProperty &&newItem) noexcept override
    {
      krys_assert(index < Size());
      _items.at(index) = krys::move(newItem);
      return At(index);
    }

    TProperty RemoveAt(uint32 index) noexcept override
    {
      krys_assert(index < Size());
      TProperty item = At(index);
      _items.erase(_items.begin() + index);
      return item;
    }

    TProperty Append(TProperty &&newItem) noexcept override
    {
      _items.push_back(krys::move(newItem));
      return At(static_cast<uint32>(Size() - 1));
    }
  };
}
