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
    for (auto &it = begin; it != _formattingElements.end(); ++it)
    {
      if (!it->IsFormattingElement())
      {
        continue;
      }

      auto &entryItem = it->Item();
      if (entryItem.TagName() != item.TagName() || entryItem.Namespace() != item.Namespace())
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

  RawPtr<HTMLStackItem> HTMLFormattingElementList::FindFromLastMarker(TagName name) noexcept
  {
    for (auto it = _formattingElements.rbegin(); it != _formattingElements.rend(); ++it)
    {
      if (it->IsMarker())
      {
        break;
      }

      if (it->IsFormattingElement() && it->Item().TagName() == name)
      {
        return &it->Item();
      }
    }

    return nullptr;
  }

  void HTMLFormattingElementList::Remove(const ContainerNode &node) noexcept
  {
    auto it = std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                                   { return entry.IsFormattingElement() && &entry.Item().Node() == &node; });
    if (it != _formattingElements.end())
    {
      _formattingElements.erase(it);
    }
  }

  RawPtr<FormattingListEntry> HTMLFormattingElementList::Find(const ContainerNode &node) noexcept
  {
    auto it = std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                                   { return entry.IsFormattingElement() && &entry.Item().Node() == &node; });
    return it != _formattingElements.end() ? &*it : nullptr;
  }

  HTMLFormattingElementList::Bookmark
    HTMLFormattingElementList::BookmarkFor(const ContainerNode &node) noexcept
  {
    for (size_t i = 0uz; i < _formattingElements.size(); ++i)
    {
      const auto &entry = _formattingElements[i];
      if (entry.IsFormattingElement() && &entry.Item().Node() == &node)
      {
        return Bookmark {i};
      }
    }

    assert(false && "node not found in formatting list");
    return Bookmark {0uz};
  }

  void HTMLFormattingElementList::MoveBookmarkAfter(Bookmark &bookmark,
                                                    const FormattingListEntry &entry) noexcept
  {
    for (size_t i = 0uz; i < _formattingElements.size(); ++i)
    {
      if (&_formattingElements[i] == &entry)
      {
        bookmark.index = i + 1uz;
        return;
      }
    }
  }

  void HTMLFormattingElementList::RemoveAndUpdateBookmark(const ContainerNode &node,
                                                          Bookmark &bookmark) noexcept
  {
    auto it = std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                                   { return entry.IsFormattingElement() && &entry.Item().Node() == &node; });

    if (it == _formattingElements.end())
    {
      return;
    }

    auto removeIndex = static_cast<size_t>(std::distance(_formattingElements.begin(), it));

    _formattingElements.erase(it);

    if (removeIndex < bookmark.index)
    {
      bookmark.index -= 1uz;
    }
  }

  void HTMLFormattingElementList::SwapTo(const ContainerNode &oldElement, HTMLStackItem &&newItem,
                                         const Bookmark &bookmark) noexcept
  {
    auto it =
      std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                           { return entry.IsFormattingElement() && &entry.Item().Node() == &oldElement; });

    size_t insertIndex = bookmark.index;

    if (it != _formattingElements.end())
    {
      auto removeIndex = static_cast<size_t>(std::distance(_formattingElements.begin(), it));
      _formattingElements.erase(it);

      if (removeIndex < insertIndex)
      {
        --insertIndex;
      }
    }

    auto insertIt = std::next(_formattingElements.begin(), static_cast<ptrdiff_t>(insertIndex));
    _formattingElements.insert(insertIt, FormattingListEntry(Krys::Move(newItem)));
  }
}
