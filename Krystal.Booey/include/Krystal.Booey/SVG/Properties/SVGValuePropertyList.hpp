#pragma once

#include "Krystal.Booey/SVG/Properties/SVGPropertyList.hpp"

namespace krys::boo::svg
{
  /// @brief A list of value properties (`SVGLength`, `SVGNumber`, `SVGPoint`, `SVGTransform`).
  template <typename TProperty>
  class SVGValuePropertyList : public SVGPropertyList<TProperty>
  {
  public:
    using Base = SVGPropertyList<TProperty>;
    using Base::ClearItems;
    using Base::Items;
    using Base::Size;

    SVGValuePropertyList &operator=(const SVGValuePropertyList &other) noexcept
    {
      ClearItems();
      for (const auto &item : other.Items())
      {
        Append(TProperty::Create(item->Value()));
      }
      return *this;
    }

    // This casting operator returns a List of the underlying ValueType,
    // for example List<float> from SVGNumberList.
    operator List<typename TProperty::ValueType>() const noexcept
    {
      List<typename TProperty::ValueType> values;
      for (const auto &item : Items())
      {
        values.push_back(item->Value());
      }
      return values;
    }

    void Resize(size_t newSize) noexcept
    {
      // Add new items.
      while (Size() < newSize)
      {
        Append(TProperty::Create());
      }

      // Remove existing items.
      while (Size() > newSize)
      {
        RemoveAt(static_cast<uint32>(Size() - 1));
      }
    }

  protected:
    using Base::Append;
    using Base::RemoveAt;

    // Base and default constructor.
    SVGValuePropertyList(SVGPropertyOwner *owner = nullptr,
                         SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept
        : Base(owner, access)
    {
    }

    // Used by SVGAnimatedPropertyList when creating it animVal from baseVal.
    SVGValuePropertyList(const SVGValuePropertyList &other,
                         SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept
        : Base(other.Owner(), access)
    {
      // Clone all items.
      for (const auto &item : other.Items())
      {
        Append(TProperty::Create(item->Value()));
      }
    }
  };
}
