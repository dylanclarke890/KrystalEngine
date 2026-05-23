#pragma once

#include "Krystal.Lib/Detection/Compiler.hpp"
#include "Krystal.Lib/Detection/OS.hpp"

#if KRYS_OS(WINDOWS)
  #include <cstdint>
  #if KRYS_COMPILER(MSVC) || KRYS_COMPILER(GCC)
    #pragma push_macro("NOMINMAX")
    #pragma push_macro("WIN32_LEAN_AND_MEAN")
    #pragma push_macro("VC_EXTRALEAN")
    #undef NOMINMAX
    #undef WIN32_LEAN_AND_MEAN
    #undef VC_EXTRALEAN

    #define NOMINMAX 1
    #define WIN32_LEAN_AND_MEAN 1
    #define VC_EXTRALEAN 1
  #endif

  #if KRYS_COMPILER(MSVC)
    #include <cstddef>
    #include <cwchar>
    #include <locale>
  #endif

  #include <winapifamily.h>
  #include <Windows.h>

  #if !defined(_KERNELX) && !defined(_ONECORE)
    #if defined(WINAPI_FAMILY) || defined(WINAPI_FAMILY_APP)
      #define KRYS_FILE_APIS_ARE_ANSI 0
    #else
      #define KRYS_FILE_APIS_ARE_ANSI 1
    #endif
  #else
    #define KRYS_FILE_APIS_ARE_ANSI 0
  #endif

namespace Krys::Text::Windows
{
  inline int DetermineActiveCodePage() noexcept
  {
  #if defined(_STL_LANG) || defined(_YVALS_CORE_H) || defined(_STDEXT)
    // Removed in later versions of VC++
    if (___lc_codepage_func() == CP_UTF8)
    {
      return CP_UTF8;
    }
  #endif // VC++ stuff

  #if KRYS_FILE_APIS_ARE_ANSI
    if (!::AreFileApisANSI())
    {
      return CP_OEMCP;
    }
  #endif

    return CP_ACP;
  }

  inline bool IsUnicodeCodePage(int codepageId)
  {
    switch (codepageId)
    {
      case CP_UTF7:
      case CP_UTF8:
      case 1'200:  // UTF-16, Little Endian ("utf-16")
      case 1'201:  // UTF-16, Big Endian ("unicodeFFFE")
      case 12'000: // UTF-16, Little Endian ("utf-32")
      case 12'001: // UTF-16, Big Endian ("utf-32BE")
      case 54'936: // GB18030, 4 bytes long
        return true;
      default: return false;
    }
  }

  struct mbtowc_default_used
  {
    int Flags;
    const char *DefaultChar;
    BOOL *DefaultCharUsed;
  };

  inline mbtowc_default_used WidecharToMultiByteUsedChar(uint32_t codepageId, const char *defaultChar,
                                                         BOOL *defaultCharUsed)
  {
    mbtowc_default_used used = {0, defaultChar, defaultCharUsed};
    switch (codepageId)
    {
      case 50'220:
      case 50'221:
      case 50'222:
      case 50'225:
      case 50'227:
      case 50'229:
      case 57'002:
      case 57'003:
      case 57'004:
      case 57'005:
      case 57'006:
      case 57'007:
      case 57'008:
      case 57'009:
      case 57'010:
      case 57'011:
      case 65'000: //(UTF-7)
      case 42:     // (Symbol)
        // flags are not allowed for these
        used.Flags = 0;
        // UNDOCUMENTED: these parameters are not allowed to be pass as anything but "nullptr" for
        // these conversions. Microsoft, of course, refuses to mention this.
        used.DefaultChar = nullptr;
        used.DefaultCharUsed = nullptr;
        break;
      case 54'936:
      case 65'001: // UTF-8
        used.Flags |= WC_ERR_INVALID_CHARS;
        break;
      case 0:
      case 1:
      case 2:
      case 3:
        // unsure for these...?
        used.Flags |= WC_ERR_INVALID_CHARS;
        break;
      default: break;
    }
    switch (codepageId)
    {
      case 65'000: //(UTF-7)
      case 65'001: //(UTF-8)
        // not allowed to use default character pointers
        used.DefaultChar = nullptr;
        used.DefaultCharUsed = nullptr;
      default: break;
    }
    return used;
  }

  inline uint32_t MultiByteToWidecharFlags(uint32_t codepageId)
  {
    switch (codepageId)
    {
      case 50'220:
      case 50'221:
      case 50'222:
      case 50'225:
      case 50'227:
      case 50'229:
      case 57'002:
      case 57'003:
      case 57'004:
      case 57'005:
      case 57'006:
      case 57'007:
      case 57'008:
      case 57'009:
      case 57'010:
      case 57'011:
      case 65'000: //(UTF-7)
      case 42:     // (Symbol)
        // flags are not allowed for these
        return 0;
      default: break;
    }
    return MB_ERR_INVALID_CHARS;
  }

  bool GetCodepageDescriptor(uint32_t codepageId, CPINFOEXW **codepageInfo) noexcept;

  // double-check if we were screwed over by the conversion: given
  // Win32's undocumented fuckups around this, the only way to know if
  // we actually failed is by checking if the single character we output
  // is equal to a replacement character, and if the replacement
  // character is NOT present in the original stream. The proper way to
  // do this is to call GetCPInfoExW and then using a comparison to the
  // MultiByte stream. But there's so many different things wrong with
  // it, and it's hard to know.

  bool WidecharToMultiByteConversionFailed(const WCHAR *input, size_t outputSize, const CHAR *output,
                                           CPINFOEXW *codepageInfo) noexcept;

  bool MultiByteToWidecharConversionFailed(size_t inputSize, const CHAR *input, const WCHAR *output,
                                           CPINFOEXW *codepageInfo) noexcept;
}

  #if KRYS_COMPILER(MSVC) || KRYS_COMPILER(GCC)
    #pragma pop_macro("VC_EXTRALEAN")
    #pragma pop_macro("WIN32_LEAN_AND_MEAN")
    #pragma pop_macro("NOMINMAX")
  #endif
#endif
