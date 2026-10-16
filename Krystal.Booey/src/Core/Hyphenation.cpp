#include "Krystal.Booey/Core/Hyphenation.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  bool CanHyphenate(const StringAtom & /* localeIdentifier */) noexcept
  {
    return false;
  }

  size_t LastHyphenLocation(utf8_stringview, size_t /* beforeIndex */,
                            const StringAtom & /* localeIdentifier */) noexcept
  {
    krys_debug_assert_msg(false, "nothing can be hyphenated (CanHyphenate is false)");
    return 0;
  }
}
