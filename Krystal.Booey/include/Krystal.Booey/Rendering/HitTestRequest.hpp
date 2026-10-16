#pragma once

#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  /// @brief What a hit test is for, and so what it may change: a pointer press or release, a move, a touch.
  class HitTestRequest
  {
  public:
    enum class Type : uint8
    {
      ReadOnly,
      Active,
      Move,
      Release,
      IgnoreCSSPointerEventsProperty,
      IgnoreClipping,
      SVGClipContent,
      TouchEvent,
      DisallowUserAgentShadowContent,
      DisallowUserAgentShadowContentExceptForImageOverlays,
      AllowFrameScrollbars,
      AllowChildFrameContent,
      AllowVisibleChildFrameContentOnly,
      ChildFrameHitTest,
      AccessibilityHitTest,
      // Collect a list of nodes instead of just one. Used for elementsFromPoint and rect-based tests.
      CollectMultipleElements,
      // When using list-based testing, continue hit testing even after a hit has been found.
      IncludeAllElementsUnderPoint,
      PenEvent,
      ForFixedContainerSampling,
      SkipTransformToRootFrameCoordinates,
    };

    static constexpr EnumSet<Type> DefaultTypes {Type::ReadOnly, Type::Active,
                                                 Type::DisallowUserAgentShadowContent};

  private:
    EnumSet<Type> _type;
    HitTestSource _source {HitTestSource::User};

  public:
    static void AssertConsistentType(EnumSet<Type> type) noexcept
    {
      krys_debug_assert(!type.ContainsAll(
        {Type::DisallowUserAgentShadowContentExceptForImageOverlays, Type::DisallowUserAgentShadowContent}));
      krys_debug_assert(!type.Contains(Type::IncludeAllElementsUnderPoint)
                        || type.Contains(Type::CollectMultipleElements));
      (void)type;
    }

    explicit HitTestRequest(HitTestSource source, EnumSet<Type> type = DefaultTypes) noexcept
        : _type(type), _source(source)
    {
      AssertConsistentType(type);
    }

    // FIXME: This constructor should be phased out in favor of the `HitTestSource` version above, such that
    // all call sites must consider whether the hit test request is user-triggered or bindings-triggered.
    HitTestRequest(EnumSet<Type> type = DefaultTypes) noexcept : _type(type)
    {
      AssertConsistentType(type);
    }

    KRYS_NODISCARD bool ReadOnly() const noexcept
    {
      return _type.Contains(Type::ReadOnly);
    }

    KRYS_NODISCARD bool Active() const noexcept
    {
      return _type.Contains(Type::Active);
    }

    KRYS_NODISCARD bool Move() const noexcept
    {
      return _type.Contains(Type::Move);
    }

    KRYS_NODISCARD bool Release() const noexcept
    {
      return _type.Contains(Type::Release);
    }

    KRYS_NODISCARD bool IgnoreCSSPointerEventsProperty() const noexcept
    {
      return _type.Contains(Type::IgnoreCSSPointerEventsProperty);
    }

    KRYS_NODISCARD bool IgnoreClipping() const noexcept
    {
      return _type.Contains(Type::IgnoreClipping);
    }

    KRYS_NODISCARD bool SVGClipContent() const noexcept
    {
      return _type.Contains(Type::SVGClipContent);
    }

    KRYS_NODISCARD bool TouchEvent() const noexcept
    {
      return _type.Contains(Type::TouchEvent);
    }

    KRYS_NODISCARD bool MouseEvent() const noexcept
    {
      return !TouchEvent() && !PenEvent();
    }

    KRYS_NODISCARD bool PenEvent() const noexcept
    {
      return _type.Contains(Type::PenEvent);
    }

    KRYS_NODISCARD bool DisallowsUserAgentShadowContent() const noexcept
    {
      return _type.Contains(Type::DisallowUserAgentShadowContent);
    }

    KRYS_NODISCARD bool DisallowsUserAgentShadowContentExceptForImageOverlays() const noexcept
    {
      return _type.Contains(Type::DisallowUserAgentShadowContentExceptForImageOverlays);
    }

    KRYS_NODISCARD bool AllowsFrameScrollbars() const noexcept
    {
      return _type.Contains(Type::AllowFrameScrollbars);
    }

    KRYS_NODISCARD bool AllowsChildFrameContent() const noexcept
    {
      return _type.Contains(Type::AllowChildFrameContent);
    }

    KRYS_NODISCARD bool AllowsVisibleChildFrameContent() const noexcept
    {
      return _type.Contains(Type::AllowVisibleChildFrameContentOnly);
    }

    KRYS_NODISCARD bool AllowsAnyFrameContent() const noexcept
    {
      return AllowsChildFrameContent() || AllowsVisibleChildFrameContent();
    }

    KRYS_NODISCARD bool IsChildFrameHitTest() const noexcept
    {
      return _type.Contains(Type::ChildFrameHitTest);
    }

    KRYS_NODISCARD bool ResultIsElementList() const noexcept
    {
      return _type.Contains(Type::CollectMultipleElements);
    }

    KRYS_NODISCARD bool IncludesAllElementsUnderPoint() const noexcept
    {
      return _type.Contains(Type::IncludeAllElementsUnderPoint);
    }

    KRYS_NODISCARD bool UserTriggered() const noexcept
    {
      return _source == HitTestSource::User;
    }

    KRYS_NODISCARD bool IsForFixedContainerSampling() const noexcept
    {
      return _type.Contains(Type::ForFixedContainerSampling);
    }

    // Convenience functions
    KRYS_NODISCARD bool TouchMove() const noexcept
    {
      return Move() && TouchEvent();
    }

    KRYS_NODISCARD bool TouchRelease() const noexcept
    {
      return Release() && TouchEvent();
    }

    KRYS_NODISCARD EnumSet<Type> GetType() const noexcept
    {
      return _type;
    }
  };
}
