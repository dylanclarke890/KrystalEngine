#include "Krystal.Booey/DOM/DOMRectList.hpp"

namespace krys::boo::dom
{
  DOMRectList::DOMRectList(const List<FloatQuad> &quads) noexcept
  {
    _items.reserve(quads.size());
    for (const auto &quad : quads)
    {
      _items.push_back(DOMRect::Create(quad.BoundingBox()));
    }
  }

  DOMRectList::DOMRectList(const List<FloatRect> &rects) noexcept
  {
    _items.reserve(rects.size());
    for (const auto &rect : rects)
    {
      _items.push_back(DOMRect::Create(rect));
    }
  }

  DOMRectList::~DOMRectList() noexcept = default;
}
