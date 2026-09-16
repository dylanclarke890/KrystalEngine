#pragma once

#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class ValueWithSmallList : public Value
  {
  private:
    SmallList<const Value *, 4uz> _storage;

  public:
    KRYS_NODISCARD const Value *TryGet(size_t index) const noexcept
    {
      return index < _storage.size() ? _storage[index] : nullptr;
    }

    KRYS_NODISCARD const Value &Get(size_t index) const noexcept
    {
      assert(index < _storage.size());
      return *_storage[index];
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _storage.size();
    }
  };

  class ValueList final : public ValueWithSmallList
  {
  public:
    KRYS_NODISCARD static Ref<ValueList> Create(char32 separator, ValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateCommaSeparated(ValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> value) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> first, Ref<Value> second) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(Ref<Value> first, Ref<Value> second,
                                                              Ref<Value> third) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSpaceSeparated(ValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<ValueList> CreateSlashSeparated(ValueListBuilder &&values) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ValueList)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsValueListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()