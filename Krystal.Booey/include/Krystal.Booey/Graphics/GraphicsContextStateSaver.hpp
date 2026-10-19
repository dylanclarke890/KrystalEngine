#pragma once

#include "Krystal.Booey/Graphics/GraphicsContext.hpp"

namespace krys::boo
{
  /// @brief Saves a graphics context's state for the length of its scope.
  class GraphicsContextStateSaver
  {
    KRYS_NON_COPY_MOVABLE(GraphicsContextStateSaver);

  private:
    GraphicsContext &_context;
    bool _saveAndRestore;

  public:
    GraphicsContextStateSaver(GraphicsContext &context, bool saveAndRestore = true) noexcept
        : _context(context), _saveAndRestore(saveAndRestore)
    {
      if (_saveAndRestore)
      {
        _context.Save();
      }
    }

    ~GraphicsContextStateSaver() noexcept
    {
      if (_saveAndRestore)
      {
        _context.Restore();
      }
    }

    void Save() noexcept
    {
      krys_debug_assert(!_saveAndRestore);
      _context.Save();
      _saveAndRestore = true;
    }

    void Restore() noexcept
    {
      krys_debug_assert(_saveAndRestore);
      _context.Restore();
      _saveAndRestore = false;
    }

    KRYS_NODISCARD GraphicsContext *Context() const noexcept
    {
      return &_context;
    }
  };

  /// @brief Draws into a transparency layer for the length of its scope.
  class TransparencyLayerScope
  {
    KRYS_NON_COPY_MOVABLE(TransparencyLayerScope);

  private:
    GraphicsContext &_context;
    float _alpha;
    bool _beganLayer;

  public:
    TransparencyLayerScope(GraphicsContext &context, float alpha, bool beginLayer = true) noexcept
        : _context(context), _alpha(alpha), _beganLayer(beginLayer)
    {
      if (beginLayer)
      {
        _context.BeginTransparencyLayer(_alpha);
      }
    }

    void BeginLayer(float alpha) noexcept
    {
      _alpha = alpha;
      _context.BeginTransparencyLayer(_alpha);
      _beganLayer = true;
    }

    ~TransparencyLayerScope() noexcept
    {
      if (_beganLayer)
      {
        _context.EndTransparencyLayer();
      }
    }
  };

  /// @brief Checks, in debug builds, that a scope leaves a graphics context's state stack as it found it.
  class GraphicsContextStateStackChecker
  {
    KRYS_NON_COPY_MOVABLE(GraphicsContextStateStackChecker);

  private:
    GraphicsContext &_context;
    uint32 _stackSize;

  public:
    GraphicsContextStateStackChecker(GraphicsContext &context) noexcept
        : _context(context), _stackSize(context.StackSize())
    {
    }

    ~GraphicsContextStateStackChecker() noexcept
    {
      krys_debug_assert_msg(_context.StackSize() == _stackSize, "GraphicsContext stack changed");
    }
  };

  /// @brief Uses an image interpolation quality for the length of its scope.
  class InterpolationQualityMaintainer
  {
    KRYS_NON_COPY_MOVABLE(InterpolationQualityMaintainer);

  private:
    GraphicsContext &_graphicsContext;
    InterpolationQuality _currentInterpolationQuality;
    bool _interpolationQualityChanged;

  public:
    explicit InterpolationQualityMaintainer(GraphicsContext &graphicsContext,
                                            InterpolationQuality interpolationQualityToUse) noexcept
        : _graphicsContext(graphicsContext),
          _currentInterpolationQuality(graphicsContext.ImageInterpolationQuality()),
          _interpolationQualityChanged(interpolationQualityToUse != InterpolationQuality::Default
                                       && _currentInterpolationQuality != interpolationQualityToUse)
    {
      if (_interpolationQualityChanged)
      {
        _graphicsContext.SetImageInterpolationQuality(interpolationQualityToUse);
      }
    }

    explicit InterpolationQualityMaintainer(GraphicsContext &graphicsContext,
                                            Maybe<InterpolationQuality> interpolationQuality) noexcept
        : InterpolationQualityMaintainer(graphicsContext, interpolationQuality
                                                            ? interpolationQuality.value()
                                                            : graphicsContext.ImageInterpolationQuality())
    {
    }

    ~InterpolationQualityMaintainer() noexcept
    {
      if (_interpolationQualityChanged)
      {
        _graphicsContext.SetImageInterpolationQuality(_currentInterpolationQuality);
      }
    }
  };
}
