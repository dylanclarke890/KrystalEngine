#pragma once

#include "Krystal.Booey/Graphics/ContentsFormat.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/LayerTreeAsTextOptions.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FloatPoint;
  class FloatRect;
  class GraphicsContext;
  class GraphicsLayer;

  enum class AnimatedProperty : uint8
  {
    Invalid,
    Translate,
    Scale,
    Rotate,
    Transform,
    Opacity,
    BackgroundColor,
    Filter,
    WebkitBackdropFilter,
  };

  enum class GraphicsLayerPaintingPhase : uint8
  {
    Background,
    Foreground,
    Mask,
    ClipPath,
    OverflowContents,
    CompositedScroll,
    ChildClippingMask,
  };

  enum class PlatformLayerTreeAsTextFlags : uint8
  {
    Debug,
    IgnoreChildren,
    IncludeModels,
  };

  // See PaintBehavior.
  enum class GraphicsLayerPaintBehavior : uint8
  {
    DefaultAsynchronousImageDecode,
    ForceSynchronousImageDecode,
  };

  /// @brief What a graphics layer asks of what it shows: its contents, its transform, and the scales and
  /// notifications of the compositor that owns it.
  class GraphicsLayerClient
  {
  public:
    virtual ~GraphicsLayerClient() noexcept = default;

    virtual void TiledBackingUsageChanged(const GraphicsLayer *, bool /*usingTiledBacking*/) noexcept
    {
    }

    // Callback for when hardware-accelerated animation started.
    virtual void NotifyAnimationStarted(const GraphicsLayer *, const utf8_string & /*animationKey*/,
                                        MonotonicTime::duration /*time*/) noexcept
    {
    }
    virtual void NotifyAnimationEnded(const GraphicsLayer *, const utf8_string & /*animationKey*/) noexcept
    {
    }

    // Notification that a layer property changed that requires a subsequent call to flushCompositingState()
    // to appear on the screen.
    virtual void NotifyFlushRequired(const GraphicsLayer *) noexcept
    {
    }

    // Notification that this layer requires a flush on the next display refresh.
    virtual void NotifySubsequentFlushRequired(const GraphicsLayer *) noexcept
    {
    }

    virtual void PaintContents(const GraphicsLayer &, GraphicsContext &, const FloatRect & /* inClip */,
                               EnumSet<GraphicsLayerPaintBehavior>) noexcept
    {
    }
    virtual void DidChangePlatformLayerForLayer(const GraphicsLayer *) noexcept
    {
    }

    // Provides current transform (taking transform-origin and animations into account). Input matrix has been
    // initialized to identity already. Returns false if the layer has no transform.
    virtual bool GetCurrentTransform(const GraphicsLayer *, TransformationMatrix &) const noexcept
    {
      return false;
    }

    // Allows the client to modify a layer position used during the visibleRect calculation, for example to
    // ignore scroll overhang.
    virtual void CustomPositionForVisibleRectComputation(const GraphicsLayer *, FloatPoint &) const noexcept
    {
    }

    // Multiplier for backing store size, related to high DPI.
    KRYS_NODISCARD virtual float DeviceScaleFactor() const noexcept
    {
      return 1;
    }
    // Page scale factor.
    KRYS_NODISCARD virtual float PageScaleFactor() const noexcept
    {
      return 1;
    }
    KRYS_NODISCARD virtual float ZoomedOutPageScaleFactor() const noexcept
    {
      return 0;
    }

    KRYS_NODISCARD virtual FloatSize EnclosingFrameViewVisibleSize() const noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual Maybe<float> CustomContentsScale(const GraphicsLayer &) const noexcept
    {
      return null;
    }

    KRYS_NODISCARD virtual float ContentsScaleMultiplierForNewTiles(const GraphicsLayer *) const noexcept
    {
      return 1;
    }
    KRYS_NODISCARD virtual bool PaintsOpaquelyAtNonIntegralScales(const GraphicsLayer *) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsFlushingLayers() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsTrackingRepaints() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool ShouldSkipLayerInDump(const GraphicsLayer *,
                                                      EnumSet<LayerTreeAsTextOptions>) const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool ShouldDumpPropertyForLayer(const GraphicsLayer *, utf8_stringview,
                                                           EnumSet<LayerTreeAsTextOptions>) const noexcept
    {
      return true;
    }

    KRYS_NODISCARD virtual bool ShouldAggressivelyRetainTiles(const GraphicsLayer *) const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool ShouldTemporarilyRetainTileCohorts(const GraphicsLayer *) const noexcept
    {
      return true;
    }

    KRYS_NODISCARD virtual bool UseGiantTiles() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool CssUnprefixedBackdropFilterEnabled() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool NeedsPixelAligment() const noexcept
    {
      return false;
    }

    virtual void DumpProperties(const GraphicsLayer *, TextStream &,
                                EnumSet<LayerTreeAsTextOptions>) const noexcept
    {
    }

    virtual void LogFilledVisibleFreshTile(uint32) noexcept
    {
    }

    KRYS_NODISCARD virtual TransformationMatrix TransformMatrixForProperty(AnimatedProperty) const noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual bool LayerNeedsPlatformContext(const GraphicsLayer &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool BackdropRootIsOpaque(const GraphicsLayer *) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual EnumSet<ContentsFormat> ScreenContentsFormats() const noexcept
    {
      return {};
    }

#if KRYS_DEBUG
    // RenderLayerBacking overrides this to verify that it is not currently painting contents. An ASSERT
    // fails, if it is. This is executed in GraphicsLayer construction and destruction to verify that we don't
    // create or destroy GraphicsLayers while painting.
    virtual void VerifyNotPainting() noexcept
    {
    }
#endif
  };

  TextStream &operator<<(TextStream &ts, GraphicsLayerPaintingPhase phase) noexcept;
}
