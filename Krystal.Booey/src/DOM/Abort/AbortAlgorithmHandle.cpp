#include "Krystal.Booey/DOM/Abort/AbortAlgorithmHandle.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include <utility>

namespace krys::boo
{
  AbortAlgorithmHandle::AbortAlgorithmHandle(AbortSignal &signal, uint64 id) noexcept
      : _signal(CreateWeakPtr(&signal)), _id(id)
  {
  }

  AbortAlgorithmHandle::AbortAlgorithmHandle(AbortAlgorithmHandle &&other) noexcept
      : _signal(std::exchange(other._signal, {})), _id(std::exchange(other._id, 0))
  {
  }

  AbortAlgorithmHandle &AbortAlgorithmHandle::operator=(AbortAlgorithmHandle &&other) noexcept
  {
    if (this != &other)
    {
      Remove();
      _signal = std::exchange(other._signal, {});
      _id = std::exchange(other._id, 0);
    }
    return *this;
  }

  AbortAlgorithmHandle::~AbortAlgorithmHandle() noexcept
  {
    Remove();
  }

  void AbortAlgorithmHandle::Remove() noexcept
  {
    // A raw pointer: the handle can go with an object that its signal is destroying, whose reference count
    // can't be raised again.
    auto *signal = _signal.get();
    auto id = std::exchange(_id, 0);
    _signal = nullptr;
    if (signal)
    {
      dom::AbortAlgorithms::Remove(*signal, id);
    }
  }

  void AbortAlgorithmHandle::Release() noexcept
  {
    _signal = nullptr;
    _id = 0;
  }

  bool AbortAlgorithmHandle::IsActive() const noexcept
  {
    auto *signal = _signal.get();
    return signal && signal->HasAbortAlgorithm(_id);
  }
}
