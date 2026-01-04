#include "Krystal.Text/EncodingDetection.hpp"
#include "Krystal.Lib/Detection/Compiler.hpp"
#include "Krystal.Lib/Detection/OS.hpp"
#include "Krystal.Text/EncodingName.hpp"
#include <string_view>

#if KRYS_OS(WINDOWS)
  #include "Krystal.Text/Impl/Windows.hpp"
#endif

namespace Krys
{
  bool is_execution_encoding_utf8() noexcept
  {
#if KRYS_OS(MACOS)
    return true;
#else
  #if KRYS_COMPILER_STL(MSVC)
    if (MB_CUR_MAX == 4)
    {
      return true;
    }
  #endif
  #if KRYS_OS(WINDOWS)
    if (Krys::Windows::DetermineActiveCodePage() == CP_UTF8)
    {
      return true;
    }
  #endif
    const char *ctypeName = setlocale(LC_CTYPE, nullptr);
    ::std::string_view adjustedCtypeName(ctypeName);
    ::std::size_t index = adjustedCtypeName.find_first_of(".");
    if (index != ::std::string_view::npos)
    {
      adjustedCtypeName = adjustedCtypeName.substr(index);
    }
    return Krys::IsEncodingNameEqual(adjustedCtypeName, "UTF-8");
#endif
  }

  bool is_execution_encoding_unicode() noexcept
  {
#if KRYS_OS(MACOS)
    return true;
#else
  #if KRYS_COMPILER_STL(MSVC)
    if (MB_CUR_MAX == 4)
    {
      return true;
    }
  #endif
  #if KRYS_OS(WINDOWS)
    int codepageId = Krys::Windows::DetermineActiveCodePage();
    if (Krys::Windows::IsUnicodeCodePage(codepageId))
    {
      return true;
    }
  #endif
    const char *ctypeName = setlocale(LC_CTYPE, nullptr);
    ::std::string_view adjustedCtypeName(ctypeName);
    ::std::size_t index = adjustedCtypeName.find_first_of(".");
    if (index != ::std::string_view::npos)
    {
      adjustedCtypeName = adjustedCtypeName.substr(index);
    }
    return Krys::IsUnicodeEncodingName(adjustedCtypeName);
#endif
  }

  bool is_wide_execution_encoding_unicode() noexcept
  {
#if KRYS_OS(WINDOWS)
    return true;
#else
  #warning "Unknown platform for wide execution encoding detection; assuming false"
    return false;
#endif
  }
}