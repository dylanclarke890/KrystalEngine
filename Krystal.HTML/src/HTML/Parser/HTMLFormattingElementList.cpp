#include "Krystal.HTML/HTML/Parser/HTMLFormattingElementList.hpp"

namespace Krys::HTML
{
  void HTMLFormattingElementList::PushMarker() noexcept
  {
    _formattingElements.emplace_back(FormattingListEntry {});
  }

  void HTMLFormattingElementList::PushElement(HTMLStackItem &&item) noexcept
  {
    // NOTE: The spec's algorithm goes as follows:
    // If there are already three elements in the list of active formatting elements after the last marker, if
    // any, or anywhere in the list if there are no markers, that have the same tag name, namespace, and
    // attributes as element, then remove the earliest such element from the list of active formatting
    // elements. For these purposes, the attributes must be compared as they were when the elements were
    // created by the parser; two elements have the same attributes if all their parsed attributes can be
    // paired such that the two attributes in each pair have identical names, namespaces, and values (the
    // order of the attributes does not matter)

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

  RawPtr<HTMLStackItem> HTMLFormattingElementList::FindFromLastMarker(TagName tagName,
                                                                      Namespace tagNamespace) noexcept
  {
    for (auto it = _formattingElements.rbegin(); it != _formattingElements.rend(); ++it)
    {
      if (it->IsMarker())
      {
        return nullptr;
      }

      auto &item = it->Item();
      if (item.TagName() == tagName && item.Namespace() == tagNamespace)
      {
        return &it->Item();
      }
    }

    return nullptr;
  }

  bool HTMLFormattingElementList::Contains(const Element &node) const noexcept
  {
    return std::ranges::any_of(_formattingElements, [&](const FormattingListEntry &entry)
                               { return entry.IsFormattingElement() && &entry.Item().Element() == &node; });
  }

  RawPtr<FormattingListEntry> HTMLFormattingElementList::Find(const Element &node) noexcept
  {
    auto it =
      std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                           { return entry.IsFormattingElement() && &entry.Item().Element() == &node; });

    return it != _formattingElements.end() ? &*it : nullptr;
  }

  void HTMLFormattingElementList::Remove(const Element &node) noexcept
  {
    auto it =
      std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                           { return entry.IsFormattingElement() && &entry.Item().Element() == &node; });

    if (it != _formattingElements.end())
    {
      _formattingElements.erase(it);
    }
  }

  void HTMLFormattingElementList::ClearUpToLastMarker() noexcept
  {
    while (true)
    {
      if (_formattingElements.empty())
      {
        return;
      }

      auto wasMarker = _formattingElements.back().IsMarker();
      _formattingElements.pop_back();

      if (wasMarker)
      {
        return;
      }
    }
  }

#pragma region Bookmark

  HTMLFormattingElementList::Bookmark HTMLFormattingElementList::BookmarkFor(const Element &node) noexcept
  {
    for (size_t i = 0uz; i < _formattingElements.size(); ++i)
    {
      const auto &entry = _formattingElements[i];
      if (entry.IsFormattingElement() && &entry.Item().Element() == &node)
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

  void HTMLFormattingElementList::RemoveAndUpdateBookmark(const Element &node, Bookmark &bookmark) noexcept
  {
    auto it =
      std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                           { return entry.IsFormattingElement() && &entry.Item().Element() == &node; });

    if (it == _formattingElements.end())
    {
      return;
    }

    auto removeIndex = static_cast<size_t>(std::distance(_formattingElements.begin(), it));

    _formattingElements.erase(it);

    if (removeIndex < bookmark.index)
    {
      --bookmark.index;
    }
  }

  void HTMLFormattingElementList::InsertAtBookmark(HTMLStackItem &&newItem, const Bookmark &bookmark) noexcept
  {
    auto insertIt = _formattingElements.begin() + static_cast<ptrdiff_t>(bookmark.index);
    _formattingElements.insert(insertIt, FormattingListEntry(Krys::Move(newItem)));
  }

#pragma endregion
}
