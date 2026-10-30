#include "Krystal.Booey/DOM/TouchList.hpp"

namespace krys::boo::dom
{
  Touch *TouchList::Item(uint32 index) const noexcept
  {
    if (index >= _values.size())
    {
      return nullptr;
    }
    return _values[index].get();
  }
}
