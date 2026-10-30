#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  class SVGProperty;

  /// @brief Reaches one of an owner's properties, for the owner's property registry.
  template <typename TOwner>
  class SVGMemberAccessor
  {
  public:
    virtual ~SVGMemberAccessor() noexcept = default;

    virtual void Detach(const TOwner &) const noexcept
    {
    }

    KRYS_NODISCARD virtual bool IsAnimatedProperty() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsAnimatedLength() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool Matches(const TOwner &, const SVGProperty &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool Matches(const TOwner &, const SVGAnimatedProperty &) const noexcept
    {
      return false;
    }

    virtual void SetDirty(const TOwner &, SVGAnimatedProperty &animatedProperty) const noexcept
    {
      animatedProperty.SetDirty();
    }

    KRYS_NODISCARD virtual Maybe<dom::DOMString> Synchronize(const TOwner &) const noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual RefPtr<SVGAttributeAnimator>
      CreateAnimator(TOwner &, const dom::QualifiedName &, AnimationMode, CalcMode, bool, bool) const noexcept
    {
      return nullptr;
    }

    virtual void AppendAnimatedInstance(TOwner &, SVGAttributeAnimator &) const noexcept
    {
    }

  protected:
    SVGMemberAccessor() noexcept = default;
  };
}
