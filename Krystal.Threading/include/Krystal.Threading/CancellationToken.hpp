#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <atomic>

namespace krys::threads
{
  /// @brief A flag a requester sets to tell a long job it's no longer wanted, which the job checks between
  /// steps to stop early. Shared by both through `Ref`s; any thread can set or read it.
  class CancellationToken final : public RefCounted<CancellationToken>
  {
    KRYS_NON_COPY_MOVABLE(CancellationToken);

  private:
    std::atomic<bool> _cancelled {false};

  public:
    CancellationToken() noexcept = default;

    KRYS_NODISCARD static Ref<CancellationToken> Create() noexcept
    {
      return CreateRef<CancellationToken>();
    }

    /// @brief Tells the job it's no longer wanted.
    void Cancel() noexcept
    {
      _cancelled.store(true, std::memory_order_relaxed);
    }

    KRYS_NODISCARD bool IsCancelled() const noexcept
    {
      return _cancelled.load(std::memory_order_relaxed);
    }
  };
}
