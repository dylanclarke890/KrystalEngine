#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::layout
{
  class InlineInvalidation;

  /// @brief What changed in an inline formatting context since its last layout: why, from which line and
  /// inline item a partial layout can start, the trailing display box of each line after the damage (to tell
  /// when a partial layout can stop), whether the inline item list needs rebuilding, and the layout boxes
  /// removed from the tree, kept alive until after layout.
  class InlineDamage
  {
    KRYS_NON_COPY_MOVABLE(InlineDamage);

  public:
    enum class Reason : uint8
    {
      Append,
      Insert,
      Remove,
      ContentChange,
      StyleChange,
      Pagination,
    };

    // FIXME: Add support for damage range with multiple, different damage types.
    struct LayoutPosition
    {
      size_t LineIndex {0};
      InlineItemPosition InlineItemPos {};
      LayoutUnit PartialContentTop;
    };

    using TrailingDisplayBoxList = List<inline_display::Box>;

  private:
    friend class InlineInvalidation;

    EnumSet<Reason> _damageReasons;
    bool _isInlineItemListDirty {false};
    Maybe<LayoutPosition> _layoutStartPosition;
    TrailingDisplayBoxList _trailingDisplayBoxes;
    List<UniquePtr<Box>> _detachedLayoutBoxes;

  public:
    InlineDamage() noexcept = default;
    ~InlineDamage() noexcept
    {
      _trailingDisplayBoxes.clear();
      _detachedLayoutBoxes.clear();
    }

    KRYS_NODISCARD EnumSet<Reason> Reasons() const noexcept
    {
      return _damageReasons;
    }

    KRYS_NODISCARD Maybe<LayoutPosition> LayoutStartPosition() const noexcept
    {
      return _layoutStartPosition;
    }

    KRYS_NODISCARD Maybe<inline_display::Box> TrailingContentForLine(size_t lineIndex) const noexcept
    {
      if (_trailingDisplayBoxes.empty())
      {
        // Couldn't compute trailing positions for damaged lines.
        return {};
      }
      if (!LayoutStartPosition() || LayoutStartPosition()->LineIndex > lineIndex)
      {
        krys_debug_assert(false);
        return {};
      }
      auto relativeLineIndex = lineIndex - LayoutStartPosition()->LineIndex;
      if (relativeLineIndex >= _trailingDisplayBoxes.size())
      {
        // At the time of the damage, we didn't have this line yet -e.g content insert at a new line.
        return {};
      }
      return _trailingDisplayBoxes[relativeLineIndex];
    }

    void AddDetachedBox(UniquePtr<Box> &&layoutBox) noexcept
    {
      _detachedLayoutBoxes.push_back(krys::move(layoutBox));
    }

    KRYS_NODISCARD bool IsInlineItemListDirty() const noexcept
    {
      return _isInlineItemListDirty;
    }
    void SetInlineItemListClean() noexcept
    {
      _isInlineItemListDirty = false;
    }

    KRYS_NODISCARD bool HasDetachedContent() const noexcept
    {
      return !_detachedLayoutBoxes.empty();
    }

  private:
    void SetDamageReason(Reason reason) noexcept
    {
      _damageReasons.Add(reason);
    }
    void SetLayoutStartPosition(LayoutPosition position) noexcept
    {
      _layoutStartPosition = position;
    }
    void ResetLayoutPosition() noexcept
    {
      _layoutStartPosition = {};
      _trailingDisplayBoxes.clear();
      // Never reset _detachedLayoutBoxes. We need to keep those layout boxes around until after layout.
    }
    void SetTrailingDisplayBoxes(TrailingDisplayBoxList &&trailingDisplayBoxes) noexcept
    {
      _trailingDisplayBoxes = krys::move(trailingDisplayBoxes);
    }
    void SetInlineItemListDirty() noexcept
    {
      _isInlineItemListDirty = true;
    }
  };
}
