#include "Krystal.Booey/Rendering/FloatingObjects.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  FloatingObject::FloatingObject(RenderBox &renderer) noexcept
      : _renderer(WeakPtr<RenderBox>::WithRef(renderer))
  {
    UsedFloat type = RenderStyle::GetUsedFloat(renderer);
    krys_debug_assert(type != UsedFloat::None);
    if (type == UsedFloat::Left)
    {
      _type = FloatLeft;
    }
    else if (type == UsedFloat::Right)
    {
      _type = FloatRight;
    }
    if (auto *containingBlock = renderer.ContainingBlock())
    {
      _hasAncestorWithOverflowClip = containingBlock->EffectiveOverflowX() == Overflow::Clip
                                     || containingBlock->EffectiveOverflowY() == Overflow::Clip;
    }
  }

  FloatingObject::FloatingObject(RenderBox &renderer, Type type, const LayoutRect &frameRect,
                                 const LayoutSize &marginOffset, bool shouldPaint, bool isDescendant,
                                 bool overflowClipped) noexcept
      : _renderer(WeakPtr<RenderBox>::WithRef(renderer)), _frameRect(frameRect), _marginOffset(marginOffset),
        _type(type), _paintsFloat(shouldPaint), _isDescendant(isDescendant), _isPlaced(true),
        _hasAncestorWithOverflowClip(overflowClipped)
  {
  }

  UniquePtr<FloatingObject> FloatingObject::Create(RenderBox &renderer) noexcept
  {
    auto object = CreateUnique<FloatingObject>(renderer);
    object->SetIsDescendant(true);
    return object;
  }

  UniquePtr<FloatingObject> FloatingObject::CopyToNewContainer(LayoutSize offset, bool shouldPaint,
                                                               bool isDescendant,
                                                               bool overflowClipped) const noexcept
  {
    krys_debug_assert(Renderer());
    return CreateUnique<FloatingObject>(*Renderer(), GetType(),
                                        LayoutRect(FrameRect().Location() - offset, FrameRect().Size()),
                                        MarginOffset(), shouldPaint, isDescendant, overflowClipped);
  }

  UniquePtr<FloatingObject> FloatingObject::CloneForNewParent() const noexcept
  {
    krys_debug_assert(Renderer());
    auto cloneObject =
      CreateUnique<FloatingObject>(*Renderer(), GetType(), _frameRect, _marginOffset, _paintsFloat,
                                   _isDescendant, _hasAncestorWithOverflowClip);
    cloneObject->_paginationStrut = _paginationStrut;
    cloneObject->_isPlaced = _isPlaced;
    return cloneObject;
  }

  bool FloatingObject::ShouldPaint() const noexcept
  {
    if (!_renderer)
    {
      return false;
    }

    return !_renderer->HasSelfPaintingLayer() && _paintsFloat;
  }

  LayoutSize FloatingObject::TranslationOffsetToAncestor() const noexcept
  {
    krys_debug_assert(Renderer());
    return LocationOffsetOfBorderBox() - Renderer()->LocationOffset();
  }

  FloatingObject *FloatingObjectSet::add(UniquePtr<FloatingObject> floatingObject) noexcept
  {
    const RenderBox *renderer = floatingObject->Renderer();
    if (auto it = _index.find(renderer); it != _index.end())
    {
      return it->second->get();
    }
    _objects.push_back(krys::move(floatingObject));
    auto position = std::prev(_objects.end());
    _index.emplace(renderer, position);
    return position->get();
  }

  void FloatingObjectSet::remove(FloatingObject *floatingObject) noexcept
  {
    for (auto it = _index.begin(); it != _index.end(); ++it)
    {
      if (it->second->get() == floatingObject)
      {
        _objects.erase(it->second);
        _index.erase(it);
        return;
      }
    }
    krys_debug_assert(false);
  }

  namespace
  {
    KRYS_NODISCARD bool RangesIntersect(LayoutUnit floatTop, LayoutUnit floatBottom, LayoutUnit objectTop,
                                        LayoutUnit objectBottom) noexcept
    {
      if (objectTop >= floatBottom || objectBottom < floatTop)
      {
        return false;
      }

      // The top of the object overlaps the float
      if (objectTop >= floatTop)
      {
        return true;
      }

      // The object encloses the float
      if (objectTop < floatTop && objectBottom > floatBottom)
      {
        return true;
      }

      // The bottom of the object overlaps the float
      if (objectBottom > objectTop && objectBottom > floatTop && objectBottom <= floatBottom)
      {
        return true;
      }

      return false;
    }

    // Finds the float of the given type that pushes an offset furthest in, among the floats that intersect a
    // range.
    template <FloatingObject::Type FloatTypeValue>
    class ComputeFloatOffsetAdapter
    {
    public:
      using IntervalType = FloatingObjectInterval;

    protected:
      const RenderBlockFlow &_renderer;
      LayoutUnit _lineTop;
      LayoutUnit _lineBottom;
      LayoutUnit _offset;
      const FloatingObject *_outermostFloat {nullptr};

    public:
      ComputeFloatOffsetAdapter(const RenderBlockFlow &renderer, LayoutUnit lineTop, LayoutUnit lineBottom,
                                LayoutUnit offset) noexcept
          : _renderer(renderer), _lineTop(lineTop), _lineBottom(lineBottom), _offset(offset)
      {
      }

      virtual ~ComputeFloatOffsetAdapter() noexcept = default;

      KRYS_NODISCARD LayoutUnit LowValue() const noexcept
      {
        return _lineTop;
      }
      KRYS_NODISCARD LayoutUnit HighValue() const noexcept
      {
        return _lineBottom;
      }
      void CollectIfNeeded(const IntervalType &interval) noexcept
      {
        const auto &floatingObject = *interval.Data();
        if (floatingObject.GetType() != FloatTypeValue || !floatingObject.Height()
            || !RangesIntersect(interval.Low(), interval.High(), _lineTop, _lineBottom))
        {
          return;
        }

        // All the objects returned from the tree should be already placed.
        krys_debug_assert(floatingObject.IsPlaced());
        // FIXME: Remove floor(). See <https://webkit.org/b/125831>.
        krys_debug_assert(RangesIntersect(_renderer.LogicalTopForFloat(floatingObject).Floor(),
                                          _renderer.LogicalBottomForFloat(floatingObject).Floor(), _lineTop,
                                          _lineBottom));

        bool floatIsNewExtreme = UpdateOffsetIfNeeded(floatingObject);
        if (floatIsNewExtreme)
        {
          _outermostFloat = &floatingObject;
        }
      }

      KRYS_NODISCARD LayoutUnit Offset() const noexcept
      {
        return _offset;
      }

    protected:
      virtual bool UpdateOffsetIfNeeded(const FloatingObject &) noexcept = 0;
    };

    template <FloatingObject::Type FloatTypeValue>
    class ComputeFloatOffsetForFloatLayoutAdapter final : public ComputeFloatOffsetAdapter<FloatTypeValue>
    {
    public:
      using ComputeFloatOffsetAdapter<FloatTypeValue>::ComputeFloatOffsetAdapter;

      KRYS_NODISCARD LayoutUnit HeightRemaining() const noexcept
      {
        return this->_outermostFloat
                 ? this->_renderer.LogicalBottomForFloat(*this->_outermostFloat) - this->_lineTop
                 : 1_lu;
      }

    protected:
      bool UpdateOffsetIfNeeded(const FloatingObject &floatingObject) noexcept final
      {
        if constexpr (FloatTypeValue == FloatingObject::FloatLeft)
        {
          LayoutUnit logicalRight = this->_renderer.LogicalRightForFloat(floatingObject);
          if (logicalRight > this->_offset)
          {
            this->_offset = logicalRight;
            return true;
          }
          return false;
        }
        else
        {
          LayoutUnit logicalLeft = this->_renderer.LogicalLeftForFloat(floatingObject);
          if (logicalLeft < this->_offset)
          {
            this->_offset = logicalLeft;
            return true;
          }
          return false;
        }
      }
    };

    template <FloatingObject::Type FloatTypeValue>
    class ComputeFloatOffsetForLineLayoutAdapter final : public ComputeFloatOffsetAdapter<FloatTypeValue>
    {
    public:
      using ComputeFloatOffsetAdapter<FloatTypeValue>::ComputeFloatOffsetAdapter;

    protected:
      bool UpdateOffsetIfNeeded(const FloatingObject &floatingObject) noexcept final
      {
        if (!floatingObject.Renderer())
        {
          return false;
        }

        if constexpr (FloatTypeValue == FloatingObject::FloatLeft)
        {
          LayoutUnit logicalRight = this->_renderer.LogicalRightForFloat(floatingObject);
          if (auto *shapeOutside = floatingObject.Renderer()->GetShapeOutsideInfo())
          {
            ShapeOutsideDeltas shapeDeltas = shapeOutside->ComputeDeltasForContainingBlockLine(
              this->_renderer, floatingObject, this->_lineTop, this->_lineBottom - this->_lineTop);
            if (!shapeDeltas.IsValid() || !shapeDeltas.LineOverlapsShape())
            {
              return false;
            }

            logicalRight += shapeDeltas.RightMarginBoxDelta();
          }
          if (logicalRight > this->_offset)
          {
            this->_offset = logicalRight;
            return true;
          }

          return false;
        }
        else
        {
          LayoutUnit logicalLeft = this->_renderer.LogicalLeftForFloat(floatingObject);
          if (auto *shapeOutside = floatingObject.Renderer()->GetShapeOutsideInfo())
          {
            ShapeOutsideDeltas shapeDeltas = shapeOutside->ComputeDeltasForContainingBlockLine(
              this->_renderer, floatingObject, this->_lineTop, this->_lineBottom - this->_lineTop);
            if (!shapeDeltas.IsValid() || !shapeDeltas.LineOverlapsShape())
            {
              return false;
            }

            logicalLeft += shapeDeltas.LeftMarginBoxDelta();
          }
          if (logicalLeft < this->_offset)
          {
            this->_offset = logicalLeft;
            return true;
          }

          return false;
        }
      }
    };

    // Finds the lowest bottom of the floats below a block offset.
    class FindNextFloatLogicalBottomAdapter
    {
    public:
      using IntervalType = FloatingObjectInterval;

    private:
      const RenderBlockFlow &_renderer;
      LayoutUnit _belowLogicalHeight;
      Maybe<LayoutUnit> _nextLogicalBottom;
      Maybe<LayoutUnit> _nextShapeLogicalBottom;

    public:
      FindNextFloatLogicalBottomAdapter(const RenderBlockFlow &renderer,
                                        LayoutUnit belowLogicalHeight) noexcept
          : _renderer(renderer), _belowLogicalHeight(belowLogicalHeight)
      {
      }

      KRYS_NODISCARD LayoutUnit LowValue() const noexcept
      {
        return _belowLogicalHeight;
      }
      KRYS_NODISCARD LayoutUnit HighValue() const noexcept
      {
        return LayoutUnit::Max();
      }

      void CollectIfNeeded(const IntervalType &interval) noexcept
      {
        const auto &floatingObject = *interval.Data();
        if (!floatingObject.Height()
            || !RangesIntersect(interval.Low(), interval.High(), _belowLogicalHeight, LayoutUnit::Max()))
        {
          return;
        }

        // All the objects returned from the tree should be already placed.
        krys_debug_assert(floatingObject.IsPlaced());
        // FIXME: Remove floor(). See <https://webkit.org/b/125831>.
        krys_debug_assert(RangesIntersect(_renderer.LogicalTopForFloat(floatingObject).Floor(),
                                          _renderer.LogicalBottomForFloat(floatingObject).Floor(),
                                          _belowLogicalHeight, LayoutUnit::Max()));

        LayoutUnit floatBottom = _renderer.LogicalBottomForFloat(floatingObject);
        if (_nextLogicalBottom && _nextLogicalBottom.value() < floatBottom)
        {
          return;
        }

        if (!floatingObject.Renderer())
        {
          return;
        }

        if (auto *shapeOutside = floatingObject.Renderer()->GetShapeOutsideInfo())
        {
          LayoutUnit shapeBottom = _renderer.LogicalTopForFloat(floatingObject)
                                   + _renderer.MarginBeforeForChild(*floatingObject.Renderer())
                                   + shapeOutside->ShapeLogicalBottom();
          // Use the shapeBottom unless it extends outside of the margin box, in which case it is clipped.
          _nextShapeLogicalBottom = std::min(shapeBottom, floatBottom);
        }
        else
        {
          _nextShapeLogicalBottom = floatBottom;
        }
        _nextLogicalBottom = floatBottom;
      }

      KRYS_NODISCARD LayoutUnit NextLogicalBottom() const noexcept
      {
        return _nextLogicalBottom.value_or(0_lu);
      }
      KRYS_NODISCARD LayoutUnit NextShapeLogicalBottom() const noexcept
      {
        return _nextShapeLogicalBottom.value_or(NextLogicalBottom());
      }
    };
  }

  LayoutUnit FloatingObjects::FindNextFloatLogicalBottomBelow(LayoutUnit logicalHeight) noexcept
  {
    FindNextFloatLogicalBottomAdapter adapter(Renderer(), logicalHeight);
    if (const FloatingObjectTree *placedFloatsTree = this->PlacedFloatsTree())
    {
      placedFloatsTree->allOverlapsWithAdapter(adapter);
    }

    return adapter.NextShapeLogicalBottom();
  }

  LayoutUnit FloatingObjects::FindNextFloatLogicalBottomBelowForBlock(LayoutUnit logicalHeight) noexcept
  {
    FindNextFloatLogicalBottomAdapter adapter(Renderer(), logicalHeight);
    if (const FloatingObjectTree *placedFloatsTree = this->PlacedFloatsTree())
    {
      placedFloatsTree->allOverlapsWithAdapter(adapter);
    }

    return adapter.NextLogicalBottom();
  }

  FloatingObjects::FloatingObjects(const RenderBlockFlow &renderer) noexcept
      : _horizontalWritingMode(renderer.IsHorizontalWritingMode()), _renderer(renderer)
  {
  }

  FloatingObjects::~FloatingObjects() noexcept = default;

  void FloatingObjects::Clear() noexcept
  {
    _set.clear();
    _placedFloatsTree = nullptr;
    _leftObjectsCount = 0;
    _rightObjectsCount = 0;
  }

  void FloatingObjects::IncreaseObjectsCount(FloatingObject::Type type) noexcept
  {
    if (type == FloatingObject::FloatLeft)
    {
      _leftObjectsCount++;
    }
    else
    {
      _rightObjectsCount++;
    }
  }

  void FloatingObjects::DecreaseObjectsCount(FloatingObject::Type type) noexcept
  {
    if (type == FloatingObject::FloatLeft)
    {
      _leftObjectsCount--;
    }
    else
    {
      _rightObjectsCount--;
    }
  }

  FloatingObjectInterval
    FloatingObjects::IntervalForFloatingObject(FloatingObject *floatingObject) const noexcept
  {
    // FIXME: The endpoints of the floating object interval shouldn't need to be
    // floored. See <https://webkit.org/b/125831> for more details.
    if (_horizontalWritingMode)
    {
      return FloatingObjectInterval(floatingObject->FrameRect().Y().Floor(),
                                    floatingObject->FrameRect().MaxY().Floor(), floatingObject);
    }
    return FloatingObjectInterval(floatingObject->FrameRect().X().Floor(),
                                  floatingObject->FrameRect().MaxX().Floor(), floatingObject);
  }

  void FloatingObjects::AddPlacedObject(FloatingObject *floatingObject) noexcept
  {
    krys_debug_assert(!floatingObject->IsInPlacedTree());

    floatingObject->SetIsPlaced(true);
    if (_placedFloatsTree)
    {
      _placedFloatsTree->add(IntervalForFloatingObject(floatingObject));
      floatingObject->SetIsInPlacedTree(true);
    }
  }

  void FloatingObjects::RemovePlacedObject(FloatingObject *floatingObject) noexcept
  {
    krys_debug_assert(floatingObject->IsPlaced());

    if (_placedFloatsTree)
    {
      KRYS_MAYBE_UNUSED auto removed = _placedFloatsTree->remove(IntervalForFloatingObject(floatingObject));
      krys_debug_assert(removed);
      floatingObject->SetIsInPlacedTree(false);
    }
    floatingObject->SetIsPlaced(false);
  }

  FloatingObject *FloatingObjects::Add(UniquePtr<FloatingObject> floatingObject) noexcept
  {
    IncreaseObjectsCount(floatingObject->GetType());
    if (floatingObject->IsPlaced())
    {
      AddPlacedObject(floatingObject.get());
    }
    return _set.add(krys::move(floatingObject));
  }

  void FloatingObjects::Remove(FloatingObject *floatingObject) noexcept
  {
    DecreaseObjectsCount(floatingObject->GetType());
    krys_debug_assert(floatingObject->IsPlaced() || !floatingObject->IsInPlacedTree());
    if (floatingObject->IsPlaced())
    {
      RemovePlacedObject(floatingObject);
    }
    _set.remove(floatingObject);
  }

  void FloatingObjects::ComputePlacedFloatsTree() noexcept
  {
    krys_debug_assert(!_placedFloatsTree);
    if (_set.isEmpty())
    {
      return;
    }

    _placedFloatsTree = CreateUnique<FloatingObjectTree>();
    for (const auto &floatingObject : _set)
    {
      if (floatingObject->IsPlaced())
      {
        _placedFloatsTree->add(IntervalForFloatingObject(floatingObject.get()));
      }
    }
  }

  const FloatingObjectTree *FloatingObjects::PlacedFloatsTree() noexcept
  {
    if (!_placedFloatsTree)
    {
      ComputePlacedFloatsTree();
    }
    return _placedFloatsTree.get();
  }

  LayoutUnit FloatingObjects::LogicalLeftOffsetForPositioningFloat(LayoutUnit fixedOffset,
                                                                   LayoutUnit logicalTop,
                                                                   LayoutUnit *heightRemaining) noexcept
  {
    ComputeFloatOffsetForFloatLayoutAdapter<FloatingObject::FloatLeft> adapter(Renderer(), logicalTop,
                                                                               logicalTop, fixedOffset);
    if (const FloatingObjectTree *placedFloatsTree = this->PlacedFloatsTree())
    {
      placedFloatsTree->allOverlapsWithAdapter(adapter);
    }

    if (heightRemaining)
    {
      *heightRemaining = adapter.HeightRemaining();
    }

    return adapter.Offset();
  }

  LayoutUnit FloatingObjects::LogicalRightOffsetForPositioningFloat(LayoutUnit fixedOffset,
                                                                    LayoutUnit logicalTop,
                                                                    LayoutUnit *heightRemaining) noexcept
  {
    ComputeFloatOffsetForFloatLayoutAdapter<FloatingObject::FloatRight> adapter(Renderer(), logicalTop,
                                                                                logicalTop, fixedOffset);
    if (const FloatingObjectTree *placedFloatsTree = this->PlacedFloatsTree())
    {
      placedFloatsTree->allOverlapsWithAdapter(adapter);
    }

    if (heightRemaining)
    {
      *heightRemaining = adapter.HeightRemaining();
    }

    return std::min(fixedOffset, adapter.Offset());
  }

  LayoutUnit FloatingObjects::LogicalLeftOffset(LayoutUnit fixedOffset, LayoutUnit logicalTop,
                                                LayoutUnit logicalHeight) noexcept
  {
    ComputeFloatOffsetForLineLayoutAdapter<FloatingObject::FloatLeft> adapter(
      Renderer(), logicalTop, logicalTop + logicalHeight, fixedOffset);
    if (const FloatingObjectTree *placedFloatsTree = this->PlacedFloatsTree())
    {
      placedFloatsTree->allOverlapsWithAdapter(adapter);
    }

    return adapter.Offset();
  }

  LayoutUnit FloatingObjects::LogicalRightOffset(LayoutUnit fixedOffset, LayoutUnit logicalTop,
                                                 LayoutUnit logicalHeight) noexcept
  {
    ComputeFloatOffsetForLineLayoutAdapter<FloatingObject::FloatRight> adapter(
      Renderer(), logicalTop, logicalTop + logicalHeight, fixedOffset);
    if (const FloatingObjectTree *placedFloatsTree = this->PlacedFloatsTree())
    {
      placedFloatsTree->allOverlapsWithAdapter(adapter);
    }

    return std::min(fixedOffset, adapter.Offset());
  }

  void FloatingObjects::ShiftFloatsBy(LayoutUnit blockShift) noexcept
  {
    auto shiftX = (_horizontalWritingMode) ? 0_lu : -blockShift;
    auto shiftY = (_horizontalWritingMode) ? blockShift : 0_lu;

    for (const auto &floatBox : _set)
    {
      if (!floatBox->Renderer())
      {
        continue;
      }

      auto isPlaced = floatBox->IsPlaced();
      if (isPlaced)
      {
        RemovePlacedObject(floatBox.get());
      }

      floatBox->_frameRect.Move(shiftX, shiftY);
      floatBox->Renderer()->Move(shiftX, shiftY);

      if (isPlaced)
      {
        AddPlacedObject(floatBox.get());
      }
    }
  }
}
