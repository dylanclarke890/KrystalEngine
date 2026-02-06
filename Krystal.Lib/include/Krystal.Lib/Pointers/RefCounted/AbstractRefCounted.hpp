#pragma once

namespace Krys
{
  /// @brief Use this class when an abstract base class needs refcounting, and the refcounting implementation
  /// will be in a concrete subclass.
  class AbstractRefCounted
  {
  protected:
    virtual ~AbstractRefCounted() = default;

  public:
    virtual void ref() const = 0;
    virtual void deref() const = 0;
  };
}
