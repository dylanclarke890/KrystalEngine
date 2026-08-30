#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CSSValueList)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::CSSValue &value) noexcept
  {
    return value.IsCSSValueListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()