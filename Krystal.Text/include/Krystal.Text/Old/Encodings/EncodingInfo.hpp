#pragma once

#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Pair.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys
{
  struct MIBenum : public StronglyTypedNumber<MIBenum, uint32>
  {
    using StronglyTypedNumber::StronglyTypedNumber;
  };

  struct Win32CodePage : public StronglyTypedNumber<Win32CodePage, uint32>
  {
    using StronglyTypedNumber::StronglyTypedNumber;
  };

  struct IsSingleByteEncoding : public StronglyTypedBool<IsSingleByteEncoding>
  {
    using StronglyTypedBool::StronglyTypedBool;
  };

  /// @brief Contains information about an encoding.
  struct EncodingInfo
  {
    constexpr static MIBenum MIBenumUnknown = MIBenum {0u};
    constexpr static Win32CodePage Win32CodePageUnknown = Win32CodePage {0u};

    /// @brief The IANA name of the encoding.
    utf8_string Name;

    /// @brief A list of aliases for the encoding.
    List<utf8_string> Aliases;

    /// @brief The MIB enum identifier of the encoding, or MIBenumUnknown if not applicable.
    MIBenum MIBenum = MIBenumUnknown;

    /// @brief The code page identifier of the encoding, or Win32CodePageUnknown if not applicable.
    Win32CodePage Win32CodePage = Win32CodePageUnknown;

    /// @brief Indicates whether the encoding uses single-byte characters.
    IsSingleByteEncoding IsSingleByte = IsSingleByteEncoding {false};

    /// @brief The byte order mark of the encoding, if any. The first element is the byte order mark as a
    /// Rune. The second element is the length of the byte order mark in bytes.
    Pair<Array<byte, 4u>, uint8> ByteOrderMark {{}, 0u};
  };
}