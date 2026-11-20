#include "Krystal.I18n/DefaultLocale.hpp"
#include <string>
#include <unicode/uloc.h>

namespace krys::i18n
{
  utf8_string DefaultLocale() noexcept
  {
    const char *locale = uloc_getDefault();
    return utf8_string(reinterpret_cast<const char8 *>(locale));
  }

  void SetDefaultLocale(utf8_stringview locale) noexcept
  {
    std::string name(reinterpret_cast<const char *>(locale.data()), locale.size());
    UErrorCode status = U_ZERO_ERROR;
    uloc_setDefault(name.c_str(), &status);
  }
}
