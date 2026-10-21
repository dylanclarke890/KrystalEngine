#include "Krystal.Booey/Graphics/GraphicsLayerKeyframeValueList.hpp"

namespace krys::boo
{
  void GraphicsLayerKeyframeValueList::Insert(UniquePtr<const GraphicsLayerAnimationValue> value) noexcept
  {
    for (size_t i = 0; i < _values.size(); ++i)
    {
      const auto *curValue = _values[i].get();
      if (curValue->KeyTime() == value->KeyTime())
      {
        krys_debug_assert(false);
        // insert after
        _values.insert(_values.begin() + static_cast<ptrdiff_t>(i + 1), krys::move(value));
        return;
      }
      if (curValue->KeyTime() > value->KeyTime())
      {
        // insert before
        _values.insert(_values.begin() + static_cast<ptrdiff_t>(i), krys::move(value));
        return;
      }
    }

    _values.push_back(krys::move(value));
  }
}
