#pragma once

#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueListValue.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <algorithm>

namespace krys::boo::style
{
  /// @brief The values of a group of coordinating list properties (e.g. the background layers).
  // https://drafts.csswg.org/css-values-4/#coordinating-list-property
  template <typename T>
  struct CoordinatedValueList
  {
    using Container = List<T>;
    using value_type = T;

  private:
    class Data : public RefCounted<Data>
    {
    public:
      Container Values;
      uint32 UsedLength {1};

      explicit Data(Container &&container) noexcept : Values {krys::move(container)}
      {
        krys_assert(!Values.empty());
      }

      KRYS_NODISCARD static Ref<Data> Create(Container &&container) noexcept
      {
        return AdoptRef(*new Data(krys::move(container)));
      }

      KRYS_NODISCARD Ref<Data> Clone() const noexcept
      {
        Container clone;
        clone.reserve(Values.size());
        for (const auto &item : Values)
        {
          clone.push_back(value_type::Clone(item));
        }
        return AdoptRef(*new Data(krys::move(clone)));
      }

      KRYS_NODISCARD bool operator==(const Data &other) const noexcept
      {
        return Values == other.Values;
      }
    };

    Ref<Data> _data;

    explicit CoordinatedValueList(Container &&container) noexcept
        : _data {Data::Create(krys::move(container))}
    {
    }

  public:
    template <css::ValueId ValueId>
    CoordinatedValueList(css::KeywordValueConstant<ValueId> keyword) noexcept
        : CoordinatedValueList {Container {value_type {keyword}}}
    {
    }

    CoordinatedValueList(T &&value) noexcept : CoordinatedValueList {MakeContainer(krys::move(value))}
    {
    }

    CoordinatedValueList(std::initializer_list<T> values) noexcept : CoordinatedValueList {Container {values}}
    {
    }

    CoordinatedValueList &Access() noexcept KRYS_LIFETIME_BOUND
    {
      if (!_data->HasOneRef())
      {
        Ref oldData = _data;
        _data = oldData->Clone();
      }
      return *this;
    }

    // Must be called after modifying the list or its values, before any of the Used*() functions are called.
    void PrepareForUse() noexcept;

    void Append(T &&value) noexcept
    {
      _data->Values.push_back(krys::move(value));
    }

    KRYS_NODISCARD value_type &UsedFirst() noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values.front();
    }

    KRYS_NODISCARD const value_type &UsedFirst() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values.front();
    }

    KRYS_NODISCARD value_type &ComputedFirst() noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values.front();
    }

    KRYS_NODISCARD const value_type &ComputedFirst() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values.front();
    }

    KRYS_NODISCARD value_type &UsedLast() noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values[_data->UsedLength - 1];
    }

    KRYS_NODISCARD const value_type &UsedLast() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values[_data->UsedLength - 1];
    }

    KRYS_NODISCARD value_type &ComputedLast() noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values.back();
    }

    KRYS_NODISCARD const value_type &ComputedLast() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values.back();
    }

    KRYS_NODISCARD T &operator[](size_t i) noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values[i];
    }

    KRYS_NODISCARD const T &operator[](size_t i) const noexcept KRYS_LIFETIME_BOUND
    {
      return _data->Values[i];
    }

    KRYS_NODISCARD uint32 UsedLength() const noexcept
    {
      return _data->UsedLength;
    }

    KRYS_NODISCARD uint32 ComputedLength() const noexcept
    {
      return static_cast<uint32>(_data->Values.size());
    }

    KRYS_NODISCARD Span<const T> UsedValues() const noexcept KRYS_LIFETIME_BOUND
    {
      return Span<const T> {_data->Values}.first(_data->UsedLength);
    }

    KRYS_NODISCARD Span<const T> ComputedValues() const noexcept KRYS_LIFETIME_BOUND
    {
      return Span<const T> {_data->Values};
    }

    KRYS_NODISCARD bool IsInitial() const noexcept
    {
      return _data->Values.size() == 1 && _data->Values[0].IsInitial()
             && AllNonBasePropertiesAreUnsetOrFilled(_data->Values[0]);
    }

    KRYS_NODISCARD bool operator==(const CoordinatedValueList &other) const noexcept
    {
      return _data.get() == other._data.get() || *_data == *other._data;
    }

  private:
    KRYS_NODISCARD static Container MakeContainer(T &&value) noexcept
    {
      Container container;
      container.push_back(krys::move(value));
      return container;
    }

    void RemoveEmptyValues() noexcept;
    void FillUnsetProperties() noexcept;
    void ComputeUsedLength() noexcept;
  };

  template <typename T>
  void CoordinatedValueList<T>::PrepareForUse() noexcept
  {
    RemoveEmptyValues();
    FillUnsetProperties();
    ComputeUsedLength();
  }

  template <typename T>
  void CoordinatedValueList<T>::RemoveEmptyValues() noexcept
  {
    for (size_t i = 1; i < _data->Values.size(); ++i)
    {
      if (AllPropertiesAreUnsetOrFilled(_data->Values[i]))
      {
        _data->Values.erase(_data->Values.begin() + static_cast<ptrdiff_t>(i), _data->Values.end());
        break;
      }
    }
  }

  template <typename T>
  void CoordinatedValueList<T>::FillUnsetProperties() noexcept
  {
    auto fillUnsetProperty = [this]<css::PropertyId propertyId>()
    {
      using PropertyAccessor = CoordinatedValueListPropertyAccessor<propertyId>;

      size_t i = 0;
      for (; i < _data->Values.size() && PropertyAccessor {_data->Values[i]}.IsSet(); ++i)
      {
      }
      if (i)
      {
        for (size_t j = 0; i < _data->Values.size(); ++i, ++j)
        {
          PropertyAccessor {_data->Values[i]}.Fill(
            typename PropertyAccessor::Type {PropertyAccessor {_data->Values[j]}.Get()});
        }
      }
    };

    EachCoordinatedValueListProperties<T>(fillUnsetProperty);
  }

  template <typename T>
  void CoordinatedValueList<T>::ComputeUsedLength() noexcept
  {
    // The length of the coordinated value list is determined by the number of items
    // specified in one particular coordinating list property, the coordinating list
    // base property. (In the case of backgrounds, this is the background-image property.).

    using BasePropertyAccessor = CoordinatedValueListPropertyConstAccessor<T::BaseProperty.value>;

    uint32 result = 0;
    for (auto &value : _data->Values)
    {
      if (!BasePropertyAccessor {value}.IsSet())
      {
        break;
      }
      ++result;
    }
    _data->UsedLength = std::max(1u, result);
  }
}
