#include "Krystal.HTML/HTML/Parser/HTMLFormattingElementList.hpp"

namespace Krys::HTML
{
  void HTMLFormattingElementList::PushElement(HTMLStackItem &&item) noexcept
  {
    assert(item.IsElement());

    auto lastMarkerIt = std::find_if(_formattingElements.rbegin(), _formattingElements.rend(),
                                     [](const auto &entry) { return entry.IsMarker(); });
    if (lastMarkerIt != _formattingElements.rend())
    {
      lastMarkerIt = std::next(lastMarkerIt);
    }

    // TODO: test that this correctly positions the begin iterator to the item after the last marker, or the
    // beginning of the list if there is no marker.
    auto begin =
      (lastMarkerIt != _formattingElements.rend()) ? lastMarkerIt.base() : _formattingElements.begin();

    auto &itemAttrs = item.Attributes();
    SmallList<decltype(_formattingElements.begin())> matchingElements;
    for (auto it = begin; it != _formattingElements.end(); ++it)
    {
      if (!it->IsFormattingElement())
      {
        continue;
      }

      auto &entryItem = it->Item();
      if (entryItem.Name() != item.Name() || entryItem.Namespace() != item.Namespace())
      {
        continue;
      }

      auto &entryAttrs = entryItem.Attributes();
      if (itemAttrs.size() != entryAttrs.size())
      {
        continue;
      }

      if (!std::ranges::all_of(entryAttrs,
                               [&](const auto &attr)
                               {
                                 return std::ranges::any_of(
                                   itemAttrs, [&](const auto &a)
                                   { return a.Name == attr.Name && a.Value == attr.Value; });
                               }))
      {
        continue;
      }

      matchingElements.push_back(it);
    }

    if (matchingElements.size() == 3)
    {
      _formattingElements.erase(matchingElements.front());
    }

    _formattingElements.push_back(FormattingListEntry(Krys::Move(item)));
  }

  void HTMLFormattingElementList::Reconstruct() noexcept
  {
    if (_formattingElements.empty())
    {
      return;
    }


  }

  void HTMLFormattingElementList::PushMarker() noexcept
  {
    _formattingElements.push_back(FormattingListEntry {});
  }

  void HTMLFormattingElementList::ClearUpToLastMarker() noexcept
  {
    auto it = std::find_if(_formattingElements.rbegin(), _formattingElements.rend(),
                           [](const FormattingListEntry &entry) { return entry.IsMarker(); });

    if (it != _formattingElements.rend())
    {
      _formattingElements.erase(it.base(), _formattingElements.end());
    }
    else
    {
      _formattingElements.clear();
    }
  }
}