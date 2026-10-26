#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwner.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::svg
{
  enum class SVGPropertyAccess : uint8
  {
    ReadWrite,
    ReadOnly
  };

  enum class SVGPropertyState : uint8
  {
    Clean,
    Dirty
  };

  /// @brief A value the SVG DOM exposes (`SVGLength`, `SVGNumberList`, ...), attached to the owner whose
  /// attribute it reflects.
  class SVGProperty : public RefCounted<SVGProperty>
  {
  protected:
    SVGPropertyOwner *_owner {nullptr};
    SVGPropertyAccess _access {SVGPropertyAccess::ReadWrite};
    SVGPropertyState _state {SVGPropertyState::Clean};

    SVGProperty(SVGPropertyOwner *owner = nullptr,
                SVGPropertyAccess access = SVGPropertyAccess::ReadWrite) noexcept
        : _owner(owner), _access(access)
    {
    }

  public:
    virtual ~SVGProperty() noexcept = default;

    // Managing the relationship with the owner.
    KRYS_NODISCARD bool IsAttached() const noexcept
    {
      return _owner;
    }

    virtual void Attach(SVGPropertyOwner *owner, SVGPropertyAccess access) noexcept
    {
      krys_assert(!_owner);
      krys_assert(_state == SVGPropertyState::Clean);
      _owner = owner;
      _access = access;
    }

    virtual void Detach() noexcept
    {
      _owner = nullptr;
      _access = SVGPropertyAccess::ReadWrite;
      _state = SVGPropertyState::Clean;
    }

    void Reattach(KRYS_MAYBE_UNUSED SVGPropertyOwner *owner, SVGPropertyAccess access) noexcept
    {
      krys_assert(owner == _owner);
      _access = access;
      _state = SVGPropertyState::Clean;
    }

    KRYS_NODISCARD const SVGElement *ContextElement() const noexcept
    {
      if (!_owner)
      {
        return nullptr;
      }
      return _owner->AttributeContextElement();
    }

    void CommitChange() noexcept
    {
      if (!_owner)
      {
        return;
      }
      _owner->CommitPropertyChange(this);
    }

    // DOM access.
    KRYS_NODISCARD SVGPropertyAccess Access() const noexcept
    {
      return _access;
    }

    KRYS_NODISCARD bool IsReadOnly() const noexcept
    {
      return _access == SVGPropertyAccess::ReadOnly;
    }

    // Synchronizing the SVG attribute and its reflection here.
    KRYS_NODISCARD bool IsDirty() const noexcept
    {
      return _state == SVGPropertyState::Dirty;
    }

    void SetDirty() noexcept
    {
      _state = SVGPropertyState::Dirty;
    }

    Maybe<dom::DOMString> Synchronize() noexcept
    {
      if (_state == SVGPropertyState::Clean)
      {
        return {};
      }
      _state = SVGPropertyState::Clean;
      return ValueAsString();
    }

    // This is used when calling setAttribute().
    KRYS_NODISCARD virtual dom::DOMString ValueAsString() const noexcept
    {
      return {};
    }
  };
}
