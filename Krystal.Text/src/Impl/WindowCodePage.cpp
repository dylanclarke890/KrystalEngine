#include "Krystal.Text/Impl/Windows.hpp"
#include <map>
#include <memory>

namespace Krys::Windows
{
  thread_local std::map<uint32_t, std::unique_ptr<CPINFOEXW>> codepageDescriptors;

  bool GetCodepageDescriptor(uint32_t codepageId, CPINFOEXW **codepageInfo) noexcept
  {
    if (codepageInfo == nullptr)
    {
      return false;
    }
    if (codepageId == CP_ACP || codepageId == CP_THREAD_ACP)
    {
      codepageId = static_cast<uint32_t>(GetACP());
    }
    else if (codepageId == CP_OEMCP)
    {
      codepageId = static_cast<uint32_t>(GetOEMCP());
    }
    auto it = codepageDescriptors.find(codepageId);
    if (it == codepageDescriptors.cend())
    {
      CPINFOEXW __info = {};
      if (GetCPInfoExW(static_cast<UINT>(codepageId), 0, &__info) == 0)
      {
        // somehow failed: we can't use this
        *codepageInfo = nullptr;
        return false;
      }
      it = codepageDescriptors.insert(std::move(it), std::pair<uint32_t, std::unique_ptr<CPINFOEXW>>(
                                                       codepageId, std::make_unique<CPINFOEXW>(__info)));
    }
    *codepageInfo = it->second.get();
    return true;
  }

  bool WidecharToMultiByteConversionFailed(const WCHAR *input, size_t outputSize,
                                                 const CHAR *output, CPINFOEXW *codepageInfo) noexcept
  {
    if (!codepageInfo)
    {
      return false;
    }
    if (output[0] != codepageInfo->DefaultChar[0])
    {
      return false;
    }
    if (codepageInfo->DefaultChar[1] == '\0')
    {
      // There is only a single byte worth of default characters
      if (input[0] == codepageInfo->UnicodeDefaultChar)
      {
        // if the input IS equal to the UTF-16 default character, then it wasn't change and instead
        // just a default transfer of what was expected.
        // NOTE: this can be a false positive, but like. What the hell else are we supposed to do?
        return false;
      }
      else
      {
        // Otherwise, if there isn't a second char then we're done here: the wide character was NOT
        // already the default and it changed therein.
        return true;
      }
    }
    if (outputSize < 2u || codepageInfo->DefaultChar[1] != output[1])
    {
      return false;
    }
    if (input[0] != codepageInfo->UnicodeDefaultChar)
    {
      return false;
    }
    // if all things are equal, then there was, in fact, a failure
    return true;
  }

  bool MultiByteToWidecharConversionFailed(size_t inputSize, const CHAR *input,
                                                 const WCHAR *output, CPINFOEXW *codepageInfo) noexcept
  {
    if (!codepageInfo)
    {
      return false;
    }
    if (output[0] != codepageInfo->UnicodeDefaultChar || output[0] != '?')
    {
      // The output does not have the replacement character!
      return false;
    }
    // Okay: the output has a replaced character. It's time to check if the input already had the (original)
    // replacement character.
    if (input[0] == codepageInfo->DefaultChar[0])
    {
      // if the input is equal to the default character, then we can't be sure.
      if (codepageInfo->DefaultChar[1] == '\0')
      {
        // This is the maximum length so it definitely can't be sure.
        return false;
      }
      if (inputSize > 1u && codepageInfo->DefaultChar[1] == input[1])
      {
        // okay, so we have 2 characters and they're both equal: we cannot be sure.
        return false;
      }
    }
    // the input does not contain the same characters, and the output IS the default character:
    // that seems like a replacement! Which means the conversion DID fail.
    return true;
  }
}