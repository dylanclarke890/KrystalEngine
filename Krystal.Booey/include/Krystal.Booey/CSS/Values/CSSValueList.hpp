#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class CSSValueContainingVector : public CSSValue
  {
  private:
    SmallList<const CSSValue *, 4uz> _storage;

  public:
    KRYS_NODISCARD const CSSValue *TryGet(size_t index) const noexcept
    {
      return index < _storage.size() ? _storage[index] : nullptr;
    }

    KRYS_NODISCARD const CSSValue &Get(size_t index) const noexcept
    {
      assert(index < _storage.size());
      return *_storage[index];
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _storage.size();
    }
  };

  class CSSValueList final : public CSSValueContainingVector
  {
  public:
    KRYS_NODISCARD static Ref<CSSValueList> Create(char32 separator, CSSValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateCommaSeparated(CSSValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(Ref<CSSValue> value) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(Ref<CSSValue> first,
                                                                 Ref<CSSValue> second) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(Ref<CSSValue> first, Ref<CSSValue> second,
                                                                 Ref<CSSValue> third) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSpaceSeparated(CSSValueListBuilder &&values) noexcept;

    KRYS_NODISCARD static Ref<CSSValueList> CreateSlashSeparated(CSSValueListBuilder &&values) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSValueList)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSValue &value) noexcept
  {
    return value.IsCSSValueListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()