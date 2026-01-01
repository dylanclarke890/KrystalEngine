#pragma once

/// @brief A token to derive from, which in some cases allows external members to place customization points
/// and extension functions in the global namespace. This is useful for enabling functionality against C-like
/// types.
template <typename...>
class KrysHijackGlobalToken
{
};

namespace Krys::Hijack
{
  /// @brief A token to derive from, which in some cases allows external members to place customization
  /// points and extension functions in the hijack namespace. Extension points would be defined in the
  /// "namespace Krys::Hijack { /* here */ }}" area.
  template <typename...>
  class Token
  {
  };
}
