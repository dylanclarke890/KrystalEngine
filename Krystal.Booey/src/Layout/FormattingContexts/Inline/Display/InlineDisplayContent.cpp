#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"

namespace krys::boo::inline_display
{
  namespace
  {
    template <typename T>
    void InsertList(List<T> &list, size_t index, List<T> &&other) noexcept
    {
      list.insert(list.begin() + static_cast<ptrdiff_t>(index), std::make_move_iterator(other.begin()),
                  std::make_move_iterator(other.end()));
    }

    template <typename T>
    void RemoveAt(List<T> &list, size_t index, size_t count) noexcept
    {
      auto first = list.begin() + static_cast<ptrdiff_t>(index);
      list.erase(first, first + static_cast<ptrdiff_t>(count));
    }
  }

  void Content::Clear() noexcept
  {
    LineList.clear();
    BoxList.clear();
  }

  void Content::Set(Content &&newContent) noexcept
  {
    LineList = krys::move(newContent.LineList);
    BoxList = krys::move(newContent.BoxList);
  }

  void Content::Append(Content &&newContent) noexcept
  {
    InsertList(LineList, LineList.size(), krys::move(newContent.LineList));
    InsertList(BoxList, BoxList.size(), krys::move(newContent.BoxList));
  }

  void Content::Insert(Content &&newContent, size_t lineIndex, size_t boxIndex) noexcept
  {
    InsertList(LineList, lineIndex, krys::move(newContent.LineList));
    InsertList(BoxList, boxIndex, krys::move(newContent.BoxList));
  }

  void Content::Remove(size_t firstLineIndex, size_t numberOfLines, size_t firstBoxIndex,
                       size_t numberOfBoxes) noexcept
  {
    RemoveAt(LineList, firstLineIndex, numberOfLines);
    RemoveAt(BoxList, firstBoxIndex, numberOfBoxes);
  }
}
