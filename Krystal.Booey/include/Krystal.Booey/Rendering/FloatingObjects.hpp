#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/IntervalTree.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <list>

namespace krys::boo
{
  class FloatingObjects;
  class RenderBlockFlow;
  class RenderBox;

  /// @brief A float as its containing block, or a block it intrudes into, sees it: its margin box, and who
  /// paints it.
  class FloatingObject
  {
    friend FloatingObjects;
    KRYS_NON_COPY_MOVABLE(FloatingObject);

  public:
    // Note that Type uses bits so you can use FloatLeftRight as a mask to query for both left and right.
    enum Type : uint8
    {
      FloatLeft = 1,
      FloatRight = 2,
      FloatLeftRight = 3
    };

  private:
    WeakPtr<RenderBox> _renderer;
    LayoutRect _frameRect;
    LayoutUnit _paginationStrut;
    LayoutSize _marginOffset;
    Type _type : 2 {FloatLeft}; // Type (left or right aligned)
    bool _paintsFloat : 1 {true};
    bool _isDescendant : 1 {false};
    bool _isPlaced : 1 {false};
    bool _hasAncestorWithOverflowClip : 1 {false};
#if KRYS_DEBUG
    bool _isInPlacedTree : 1 {false};
#endif

  public:
    KRYS_NODISCARD static UniquePtr<FloatingObject> Create(RenderBox &renderer) noexcept;
    KRYS_NODISCARD UniquePtr<FloatingObject> CopyToNewContainer(LayoutSize offset, bool shouldPaint = false,
                                                                bool isDescendant = false,
                                                                bool overflowClipped = false) const noexcept;
    KRYS_NODISCARD UniquePtr<FloatingObject> CloneForNewParent() const noexcept;

    explicit FloatingObject(RenderBox &renderer) noexcept;
    FloatingObject(RenderBox &renderer, Type type, const LayoutRect &frameRect,
                   const LayoutSize &marginOffset, bool shouldPaint, bool isDescendant,
                   bool overflowClipped) noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }
    KRYS_NODISCARD RenderBox *Renderer() const noexcept
    {
      return _renderer.get();
    }

    KRYS_NODISCARD bool IsPlaced() const noexcept
    {
      return _isPlaced;
    }
    void SetIsPlaced(bool placed = true) noexcept
    {
      _isPlaced = placed;
    }

    KRYS_NODISCARD LayoutUnit X() const noexcept
    {
      krys_debug_assert(IsPlaced());
      return _frameRect.X();
    }
    KRYS_NODISCARD LayoutUnit MaxX() const noexcept
    {
      krys_debug_assert(IsPlaced());
      return _frameRect.MaxX();
    }
    KRYS_NODISCARD LayoutUnit Y() const noexcept
    {
      krys_debug_assert(IsPlaced());
      return _frameRect.Y();
    }
    KRYS_NODISCARD LayoutUnit MaxY() const noexcept
    {
      krys_debug_assert(IsPlaced());
      return _frameRect.MaxY();
    }
    KRYS_NODISCARD LayoutUnit Width() const noexcept
    {
      return _frameRect.Width();
    }
    KRYS_NODISCARD LayoutUnit Height() const noexcept
    {
      return _frameRect.Height();
    }

    void SetX(LayoutUnit x) noexcept
    {
      krys_debug_assert(!IsInPlacedTree());
      _frameRect.SetX(x);
    }
    void SetY(LayoutUnit y) noexcept
    {
      krys_debug_assert(!IsInPlacedTree());
      _frameRect.SetY(y);
    }
    void SetWidth(LayoutUnit width) noexcept
    {
      krys_debug_assert(!IsInPlacedTree());
      _frameRect.SetWidth(width);
    }
    void SetHeight(LayoutUnit height) noexcept
    {
      krys_debug_assert(!IsInPlacedTree());
      _frameRect.SetHeight(height);
    }

    void SetMarginOffset(LayoutSize offset) noexcept
    {
      krys_debug_assert(!IsInPlacedTree());
      _marginOffset = offset;
    }

    KRYS_NODISCARD const LayoutRect &FrameRect() const noexcept
    {
      krys_debug_assert(IsPlaced());
      return _frameRect;
    }
    void SetFrameRect(const LayoutRect &frameRect) noexcept
    {
      krys_debug_assert(!IsInPlacedTree());
      _frameRect = frameRect;
    }

    KRYS_NODISCARD LayoutUnit PaginationStrut() const noexcept
    {
      return _paginationStrut;
    }
    void SetPaginationStrut(LayoutUnit strut) noexcept
    {
      _paginationStrut = strut;
    }

    KRYS_NODISCARD bool IsInPlacedTree() const noexcept
    {
#if KRYS_DEBUG
      return _isInPlacedTree;
#else
      return false;
#endif
    }
    void SetIsInPlacedTree(KRYS_MAYBE_UNUSED bool value) noexcept
    {
#if KRYS_DEBUG
      _isInPlacedTree = value;
#endif
    }

    KRYS_NODISCARD bool ShouldPaint() const noexcept;

    KRYS_NODISCARD bool PaintsFloat() const noexcept
    {
      return _paintsFloat;
    }
    void SetPaintsFloat(bool paintsFloat) noexcept
    {
      _paintsFloat = paintsFloat;
    }

    KRYS_NODISCARD bool HasAncestorWithOverflowClip() const noexcept
    {
      return _hasAncestorWithOverflowClip;
    }

    KRYS_NODISCARD bool IsDescendant() const noexcept
    {
      return _isDescendant;
    }
    void SetIsDescendant(bool isDescendant) noexcept
    {
      _isDescendant = isDescendant;
    }

    KRYS_NODISCARD LayoutSize LocationOffsetOfBorderBox() const noexcept
    {
      krys_debug_assert(IsPlaced());
      return LayoutSize(_frameRect.Location().X() + _marginOffset.Width(),
                        _frameRect.Location().Y() + _marginOffset.Height());
    }
    KRYS_NODISCARD LayoutSize MarginOffset() const noexcept
    {
      krys_debug_assert(IsPlaced());
      return _marginOffset;
    }
    KRYS_NODISCARD LayoutSize TranslationOffsetToAncestor() const noexcept;
  };

  /// @brief The floating objects of a block, in insertion order, unique by renderer.
  class FloatingObjectSet
  {
    KRYS_NON_COPY_MOVABLE(FloatingObjectSet);

  private:
    using Storage = std::list<UniquePtr<FloatingObject>>;

    Storage _objects;
    // Keyed by the renderer a floating object was added with.
    HashMap<const RenderBox *, Storage::iterator> _index;

  public:
    using const_iterator = Storage::const_iterator;

    FloatingObjectSet() noexcept = default;

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return _objects.cbegin();
    }
    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return _objects.cend();
    }
    KRYS_NODISCARD auto rbegin() const noexcept
    {
      return _objects.crbegin();
    }
    KRYS_NODISCARD auto rend() const noexcept
    {
      return _objects.crend();
    }
    KRYS_NODISCARD size_t size() const noexcept
    {
      return _objects.size();
    }
    KRYS_NODISCARD bool isEmpty() const noexcept
    {
      return _objects.empty();
    }
    KRYS_NODISCARD const UniquePtr<FloatingObject> &last() const noexcept
    {
      return _objects.back();
    }

    /// @brief The floating object of @p renderer, or null.
    KRYS_NODISCARD FloatingObject *find(const RenderBox &renderer) const noexcept
    {
      auto it = _index.find(&renderer);
      return it != _index.end() ? it->second->get() : nullptr;
    }
    KRYS_NODISCARD bool contains(const RenderBox &renderer) const noexcept
    {
      return _index.contains(&renderer);
    }

    /// @brief Adds @p floatingObject, unless its renderer already has one.
    /// @return The renderer's floating object.
    FloatingObject *add(UniquePtr<FloatingObject> floatingObject) noexcept;
    void remove(FloatingObject *floatingObject) noexcept;
    void clear() noexcept
    {
      _index.clear();
      _objects.clear();
    }
  };

  using FloatingObjectInterval = Interval<LayoutUnit, FloatingObject *>;
  using FloatingObjectTree = IntervalTree<LayoutUnit, FloatingObject *>;

  /// @brief The floats of a block flow, and the tree of the placed ones by block range.
  // FIXME: This is really the same thing as FloatingObjectSet.
  // Change clients to use that set directly, and replace the moveAllToFloatInfoMap function with a takeSet
  // function.
  class FloatingObjects
  {
    KRYS_NON_COPY_MOVABLE(FloatingObjects);

  private:
    FloatingObjectSet _set;
    UniquePtr<FloatingObjectTree> _placedFloatsTree;
    uint32 _leftObjectsCount {0};
    uint32 _rightObjectsCount {0};
    bool _horizontalWritingMode {false};
    const RenderBlockFlow &_renderer;

  public:
    explicit FloatingObjects(const RenderBlockFlow &renderer) noexcept;
    ~FloatingObjects() noexcept;

    void Clear() noexcept;
    FloatingObject *Add(UniquePtr<FloatingObject> floatingObject) noexcept;
    void Remove(FloatingObject *floatingObject) noexcept;
    void AddPlacedObject(FloatingObject *floatingObject) noexcept;
    void RemovePlacedObject(FloatingObject *floatingObject) noexcept;
    void SetHorizontalWritingMode(bool b = true) noexcept
    {
      _horizontalWritingMode = b;
    }

    KRYS_NODISCARD bool HasLeftObjects() const noexcept
    {
      return _leftObjectsCount > 0;
    }
    KRYS_NODISCARD bool HasRightObjects() const noexcept
    {
      return _rightObjectsCount > 0;
    }
    KRYS_NODISCARD const FloatingObjectSet &Set() const noexcept
    {
      return _set;
    }

    KRYS_NODISCARD LayoutUnit LogicalLeftOffset(LayoutUnit fixedOffset, LayoutUnit logicalTop,
                                                LayoutUnit logicalHeight) noexcept;
    KRYS_NODISCARD LayoutUnit LogicalRightOffset(LayoutUnit fixedOffset, LayoutUnit logicalTop,
                                                 LayoutUnit logicalHeight) noexcept;

    KRYS_NODISCARD LayoutUnit LogicalLeftOffsetForPositioningFloat(LayoutUnit fixedOffset,
                                                                   LayoutUnit logicalTop,
                                                                   LayoutUnit *heightRemaining) noexcept;
    KRYS_NODISCARD LayoutUnit LogicalRightOffsetForPositioningFloat(LayoutUnit fixedOffset,
                                                                    LayoutUnit logicalTop,
                                                                    LayoutUnit *heightRemaining) noexcept;

    KRYS_NODISCARD LayoutUnit FindNextFloatLogicalBottomBelow(LayoutUnit logicalHeight) noexcept;
    KRYS_NODISCARD LayoutUnit FindNextFloatLogicalBottomBelowForBlock(LayoutUnit logicalHeight) noexcept;

    void ShiftFloatsBy(LayoutUnit blockShift) noexcept;

  private:
    KRYS_NODISCARD const RenderBlockFlow &Renderer() const noexcept
    {
      return _renderer;
    }
    void ComputePlacedFloatsTree() noexcept;
    KRYS_NODISCARD const FloatingObjectTree *PlacedFloatsTree() noexcept;
    void IncreaseObjectsCount(FloatingObject::Type type) noexcept;
    void DecreaseObjectsCount(FloatingObject::Type type) noexcept;
    KRYS_NODISCARD FloatingObjectInterval
      IntervalForFloatingObject(FloatingObject *floatingObject) const noexcept;
  };
}
