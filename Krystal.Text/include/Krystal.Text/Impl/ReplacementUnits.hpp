#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Text/Impl/Unicode.hpp"

namespace Krys::Impl
{
  template <typename TChar>
  constexpr inline Array<TChar, 1> QuestionMarkReplacementUnits {
    {static_cast<TChar>(Krys::Impl::Unicode::ASCIIReplacement)}};

  template <typename TChar>
  constexpr inline Array<TChar, 1> xFFFDReplacementUnits {
    {static_cast<TChar>(Krys::Impl::Unicode::Replacement)}};

  template <typename TChar>
  constexpr inline Array<TChar, 3> xFFFDUTF8ReplacementUnits {
    {static_cast<TChar>(0xEF), static_cast<TChar>(0xBF), static_cast<TChar>(0xBD)}};

  template <typename TChar>
  constexpr inline Array<TChar, 4> xFFFDGB18030ReplacementUnits {
    {static_cast<TChar>(0x84), static_cast<TChar>(0x31), static_cast<TChar>(0xA4), static_cast<TChar>(0x37)}};
}
