#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A graphics layer's transform combined with its parent's, its position and its anchor point.
  class GraphicsLayerTransform
  {
  private:
    FloatPoint3D _anchorPoint;
    FloatPoint _position;
    FloatSize _size;
    bool _flattening;
    bool _dirty;
    mutable bool _childrenDirty;
    TransformationMatrix _local;
    TransformationMatrix _children;
    TransformationMatrix _combined;
    mutable TransformationMatrix _combinedForChildren;

  public:
    GraphicsLayerTransform() noexcept;

    void SetPosition(const FloatPoint &position) noexcept;
    void SetSize(const FloatSize &size) noexcept;
    void SetAnchorPoint(const FloatPoint3D &anchorPoint) noexcept;
    void SetFlattening(bool flattening) noexcept;
    void SetLocalTransform(const TransformationMatrix &transform) noexcept;
    void SetChildrenTransform(const TransformationMatrix &transform) noexcept;

    KRYS_NODISCARD const TransformationMatrix &Combined() const noexcept;
    KRYS_NODISCARD const TransformationMatrix &CombinedForChildren() const noexcept;

    void CombineTransforms(const TransformationMatrix &parentTransform) noexcept;

  private:
    void CombineTransformsForChildren() const noexcept;
  };
}
