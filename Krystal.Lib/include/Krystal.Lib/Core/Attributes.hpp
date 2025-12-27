#pragma once

namespace Krys
{
  /// @brief Convenience macro for the attribute equivalent.
#define NO_DISCARD [[nodiscard]]

/// @brief Convenience macro for the attribute equivalent.
#define NO_RETURN [[noreturn]]

/// @brief Convenience macro for the attribute equivalent.
#define ALLOW_CASE_FALLTHROUGH [[fallthrough]]

/// @brief Convenience macro for the attribute equivalent.
#define BRANCH_LIKELY [[likely]]

/// @brief Convenience macro for the attribute equivalent.
#define BRANCH_UNLIKELY [[unlikely]]

/// @brief Convenience macro for the attribute equivalent.
#define MAYBE_UNUSED [[maybe_unused]]
}