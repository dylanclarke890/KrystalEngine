#include "Krystal.Booey/DOM/HighlightRegistry.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD DOMStringAtom AnnotationHighlightKey() noexcept
    {
      return DOMStringAtom(u8"annotationHighlightKey");
    }
  }

  void HighlightRegistry::Set(DOMStringAtom key, Ref<Highlight> &&value) noexcept
  {
    auto [iterator, isNewEntry] = _map.insert_or_assign(key, krys::move(value));
    if (isNewEntry)
    {
      krys_debug_assert(std::ranges::find(_highlightNames, key) == _highlightNames.end());
      _highlightNames.push_back(key);
    }
  }

  Highlight *HighlightRegistry::Get(DOMStringAtom key) const noexcept
  {
    auto found = _map.find(key);
    return found == _map.end() ? nullptr : found->second.get();
  }

  void HighlightRegistry::Clear() noexcept
  {
    _highlightNames.clear();
    auto map = std::exchange(_map, {});
    for (auto &[name, highlight] : map)
    {
      highlight->Clear();
    }
  }

  bool HighlightRegistry::Delete(DOMStringAtom key) noexcept
  {
    if (auto found = std::ranges::find(_highlightNames, key); found != _highlightNames.end())
    {
      _highlightNames.erase(found);
    }
    return _map.erase(key) > 0;
  }

  void HighlightRegistry::AddAnnotationHighlightWithRange(Ref<StaticRange> &&value) noexcept
  {
    if (auto *highlight = Get(AnnotationHighlightKey()))
    {
      (void)highlight->Add(*value);
    }
    else
    {
      List<Ref<AbstractRange>> ranges;
      ranges.push_back(krys::move(value));
      Set(AnnotationHighlightKey(), Highlight::Create(krys::move(ranges)));
    }
  }
}
