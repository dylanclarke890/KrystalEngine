#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief Pads whose buttons and axes are known, by vendor (the high 16 bits) and product (the low 16
  /// bits) ID, so they can be given the standard mapping.
  enum class KnownGamepad : uint32
  {
    Dualshock3 = 0x05'4c'02'68,
    Dualshock4_1 = 0x05'4c'05'c4,
    Dualshock4_2 = 0x05'4c'09'cc,
    GamesirM2 = 0x0e'c2'04'75,
    HoripadUltimate = 0x0f'0d'00'90,
    LogitechF310 = 0x04'6d'c2'16,
    LogitechF710 = 0x04'6d'c2'19,
    Nimbus1 = 0x01'11'14'20,
    Nimbus2 = 0x10'38'14'20,
    StadiaA = 0x18'd1'94'00,
    StratusXL1 = 0x01'11'14'18,
    StratusXL2 = 0x10'38'14'18,
    StratusXL3 = 0x01'11'14'19,
    StratusXL4 = 0x10'38'14'19,
    XboxOne1 = 0x04'5e'02'e0,
    XboxOne2 = 0x04'5e'02'ea,
    XboxOne3 = 0x04'5e'02'fd,
  };
}
