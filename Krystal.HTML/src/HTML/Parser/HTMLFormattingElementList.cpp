#include "Krystal.HTML/HTML/Parser/HTMLFormattingElementList.hpp"

namespace Krys::HTML
{
  void HTMLFormattingElementList::PushElement(HTMLStackItem &&item) noexcept
  {
    assert(item.IsElement());

    auto lastMarkerIt = std::find_if(_formattingElements.rbegin(), _formattingElements.rend(),
                                     [](const auto &entry) { return entry.IsMarker(); });
    auto lastMarkerIndex =
      (lastMarkerIt != _formattingElements.rend())
        ? static_cast<size_t>(std::distance(lastMarkerIt, _formattingElements.rend())) - 1uz
        : 0uz;

    SmallList<decltype(_formattingElements.begin())> identicalFormattingElements;
    for (auto it = _formattingElements.begin() + lastMarkerIndex; it != _formattingElements.end(); ++it)
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
      if (item.Attributes().size() != entryAttrs.size())
      {
        continue;
      }

      if (entryAttrs.empty() && item.Attributes().empty())
      {
        identicalFormattingElements.push_back(it);
        continue;
      }

      if (std::ranges::all_of(entryAttrs,
                              [&](const auto &attr)
                              {
                                return std::ranges::any_of(
                                  item.Attributes(), [&](const auto &a)
                                  { return a.Name == attr.Name && a.Value == attr.Value; });
                              }))
      {
        identicalFormattingElements.push_back(it);
      }
    }

    if (identicalFormattingElements.size() == 3)
    {
      _formattingElements.erase(identicalFormattingElements.front());
    }

    _formattingElements.emplace_back(FormattingListEntry(Krys::Move(item)));
  }

  void HTMLFormattingElementList::PushMarker() noexcept
  {
    _formattingElements.emplace_back(FormattingListEntry {});
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

  RawPtr<HTMLStackItem> HTMLFormattingElementList::FindFromLastMarker(TagName tagName,
                                                                      Namespace tagNamespace) noexcept
  {
    for (auto it = _formattingElements.rbegin(); it != _formattingElements.rend(); ++it)
    {
      if (it->IsMarker())
      {
        break;
      }

      auto &item = it->Item();

      if (item.TagName() == tagName && item.Namespace() == tagNamespace)
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

  void HTMLFormattingElementList::Insert(HTMLStackItem &&newItem, const Bookmark &bookmark) noexcept
  {
    size_t insertIndex = bookmark.index;
    auto insertIt = std::next(_formattingElements.begin(), static_cast<ptrdiff_t>(insertIndex));
    _formattingElements.insert(insertIt, FormattingListEntry(Krys::Move(newItem)));
  }
}
