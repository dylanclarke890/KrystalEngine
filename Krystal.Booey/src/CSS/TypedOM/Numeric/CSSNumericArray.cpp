#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSNumericArray.hpp"

namespace krys::boo::css
{
  Ref<CSSNumericArray> CSSNumericArray::Create(List<CSSNumberish> &&numberishes) noexcept
  {
    List<Ref<CSSNumericValue>> values;
    values.reserve(numberishes.size());
    for (auto &numberish : numberishes)
    {
      values.push_back(CSSNumericValue::RectifyNumberish(krys::move(numberish)));
    }
    return AdoptRef(*new CSSNumericArray(krys::move(values)));
  }

  Ref<CSSNumericArray> CSSNumericArray::Create(List<Ref<CSSNumericValue>> &&values) noexcept
  {
    return AdoptRef(*new CSSNumericArray(krys::move(values)));
  }

  CSSNumericArray::CSSNumericArray(List<Ref<CSSNumericValue>> &&values) noexcept : _array(krys::move(values))
  {
  }

  RefPtr<CSSNumericValue> CSSNumericArray::Item(size_t index) noexcept
  {
    if (index >= _array.size())
    {
      return nullptr;
    }
    return _array[index];
  }

  void CSSNumericArray::ForEach(const Func<void(const CSSNumericValue &, bool first)> &function) noexcept
  {
    for (size_t i = 0; i < _array.size(); ++i)
    {
      function(*_array[i], !i);
    }
  }
}
