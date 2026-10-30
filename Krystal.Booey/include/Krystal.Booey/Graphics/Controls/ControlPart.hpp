#pragma once

#include "Krystal.Booey/Core/StyleAppearance.hpp"
#include "Krystal.Booey/Graphics/Controls/ControlFactory.hpp"
#include "Krystal.Booey/Graphics/Controls/PlatformControl.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FloatRect;
  class FloatRoundedRect;
  class GraphicsContext;

  /// @brief A form control a theme draws, of one appearance, with what it needs to draw it (a slider's thumb
  /// position, a progress bar's value).
  class ControlPart : public RefCounted<ControlPart>
  {
  protected:
    const StyleAppearance _type;

    mutable UniquePtr<PlatformControl> _platformControl;
    RefPtr<ControlFactory> _controlFactory;
    RefPtr<ControlFactory> _overrideControlFactory;

  public:
    virtual ~ControlPart() noexcept = default;

    KRYS_NODISCARD StyleAppearance GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD ControlFactory &GetControlFactory() const noexcept;
    void SetOverrideControlFactory(RefPtr<ControlFactory> &&controlFactory) noexcept
    {
      _overrideControlFactory = krys::move(controlFactory);
    }

    KRYS_NODISCARD FloatSize SizeForBounds(const FloatRect &bounds, const ControlStyle &) noexcept;
    KRYS_NODISCARD FloatRect RectForBounds(const FloatRect &bounds, const ControlStyle &) noexcept;
    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) const noexcept;

  protected:
    explicit ControlPart(StyleAppearance) noexcept;

    KRYS_NODISCARD PlatformControl *GetPlatformControl() const noexcept;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl> CreatePlatformControl() noexcept = 0;
  };
}

#define KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(PartName)                                              \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::PartName##Part)                                          \
    KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept                           \
    {                                                                                                        \
      return part.GetType() == krys::boo::StyleAppearance::PartName;                                         \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
