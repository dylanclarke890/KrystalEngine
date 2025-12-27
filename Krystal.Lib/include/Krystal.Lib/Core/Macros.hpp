#pragma once

namespace Krys
{
#pragma region Function Signature

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000))                                     \
  || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
  #define KRYS_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
  #define KRYS_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__) || defined(_MSC_VER)
  #define KRYS_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
  #define KRYS_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
  #define KRYS_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199'901)
  #define KRYS_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201'103)
  #define KRYS_FUNC_SIG __func__
#else
  #warning "KRYS_FUNC_SIG unknown!"
  #define KRYS_FUNC_SIG "Unknown Function Signature"
#endif

#pragma endregion

#pragma region Compiler Warnings

  // Compiler-specific macros for disabling and restoring warnings
#if defined(_MSC_VER) // Microsoft Visual C++
  #define KRYS_DISABLE_WARNING_PUSH() __pragma(warning(push))
  #define KRYS_DISABLE_WARNING_POP() __pragma(warning(pop))
  #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName) __pragma(warning(disable : msvcWarningCode))
#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang
  #define KRYS_DISABLE_WARNING_PUSH() _Pragma("GCC diagnostic push")
  #define KRYS_DISABLE_WARNING_POP() _Pragma("GCC diagnostic pop")
  #if defined(__clang__)
    #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)                                            \
      _Pragma("clang diagnostic ignored \"" gccWarningName "\"")
  #else
    #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)                                            \
      _Pragma("GCC diagnostic ignored \"" gccWarningName "\"")
  #endif
#else
  #warning "Compiler not supported for warning suppression macros."
  #define KRYS_DISABLE_WARNING_PUSH()
  #define KRYS_DISABLE_WARNING_POP()
  #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)
#endif

#pragma endregion

#define KRYS_UNUSED(x) (void)(x) // Suppress unused variable warnings

#define NO_COPY(ClassName)                                                                                   \
  ClassName(const ClassName &) = delete;                                                                     \
  ClassName &operator=(const ClassName &) = delete;

#define NO_MOVE(ClassName)                                                                                   \
  ClassName(ClassName &&) = delete;                                                                          \
  ClassName &operator=(ClassName &&) = delete;

#define NO_COPY_MOVE(ClassName)                                                                              \
  NO_COPY(ClassName)                                                                                         \
  NO_MOVE(ClassName)

#define STATIC_CLASS(ClassName)                                                                              \
  ClassName() = delete;                                                                                      \
  ~ClassName() = delete;                                                                                     \
  NO_COPY_MOVE(ClassName)

#define MOVE_SWAP(ClassName)                                                                                 \
  ClassName(ClassName &&other) noexcept                                                                      \
  {                                                                                                          \
    Swap(other);                                                                                             \
  }                                                                                                          \
                                                                                                             \
  ClassName &operator=(ClassName &&other) noexcept                                                           \
  {                                                                                                          \
    if (this != &other)                                                                                      \
    {                                                                                                        \
      Swap(other);                                                                                           \
    }                                                                                                        \
    return *this;                                                                                            \
  }                                                                                                          \
  void swap(ClassName &other) noexcept                                                                       \
  {                                                                                                          \
    Swap(other);                                                                                             \
  }

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define KRYS_CONCAT_IMPL(x, y) x##y
#define KRYS_CONCAT(x, y) KRYS_CONCAT_IMPL(x, y)

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define IS_COMPILE_TIME std::is_constant_evaluated()
}