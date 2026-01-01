#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys
{
  /// @brief Retrieves the code unit type for the given type.
  template <typename T>
  class CodeUnit
  {
  public:
    /// @brief The code unit type for the encoding type.
    using type = typename remove_cvref_t<T>::code_unit;
  };

  /// @brief A typename alias for CodeUnit.
  template <typename T>
  using code_unit_t = typename CodeUnit<remove_cvref_t<T>>::type;

  /// @brief Gets the maximum number of code units that can be produced by an encoding during an encode
  /// operation, suitable for initializing a automatic storage duration ("stack-allocated") buffer.
  template <typename T>
  inline static constexpr std::size_t MaxCodeUnits = remove_cvref_t<T>::MaxCodeUnits;

  namespace Impl
  {
    template <typename T>
    concept HasCodeUnitType = requires { typename T::code_unit; };

    template <typename, typename = void>
    struct CodeUnitOrVoid
    {
      using type = void;
    };

    template <typename T>
    struct CodeUnitOrVoid<T, enable_if_t<HasCodeUnitType<T>>>
    {
      using type = code_unit_t<T>;
    };

    template <typename T>
    using code_unit_or_void_t = CodeUnitOrVoid<remove_cvref_t<T>>;
  }
}
