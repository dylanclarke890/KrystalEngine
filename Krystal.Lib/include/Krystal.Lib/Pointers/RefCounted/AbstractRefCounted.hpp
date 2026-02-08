#pragma once

namespace Krys
{
  /// @brief Use this class when an abstract base class needs refcounting, and the refcounting implementation
  /// will be in a concrete subclass.
  class AbstractRefCounted
  {
  protected:
    virtual ~AbstractRefCounted() noexcept = default;

  public:
    virtual void AddRef() const noexcept = 0;
    virtual void SubRef() const noexcept = 0;
  };
}
