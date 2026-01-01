#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"
#include "Krystal.Text/State.hpp"

namespace Krys
{
  namespace Impl
  {
    template <typename T>
    concept HasIsSelfSyncCode = requires { T::is_self_synchronizing_code::value; };
  }

  /// @brief Checks whether not an encoding has distinct sequences that can be identified unambiguously from
  /// anywhere within a larger sequence. This implies that the start of any given sequence -- including
  /// sequences that are a single input unit -- can be reliably identified even in a stream full of errors.
  /// @tparam T
  template <typename T>
  concept IsSelfSynchronizingCode =
    Impl::HasIsSelfSyncCode<T> || (IsEmpty<T> && (MaxCodePoints<T> == 1 && (MaxCodeUnits<T> == 1)));
}
