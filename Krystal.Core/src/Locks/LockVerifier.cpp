#include "Krystal.Core/Locks/LockVerifier.hpp"

#include <cassert>

namespace Krys
{
  LockVerifier::LockVerifier() noexcept : _locked(false)
  {
  }

  void LockVerifier::Acquire() noexcept
  {
    // Assert that no one already has the lock
    assert(!_locked);
    _locked = true; // Mark as locked to detect overlapping critical sections
  }

  void LockVerifier::Release() noexcept
  {
    // Assert correct usage (that Release() is only called after Acquire())
    assert(_locked);
    _locked = false;
  }
}