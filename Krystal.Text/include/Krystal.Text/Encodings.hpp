#pragma once

#include "Krystal.Text/Encodings/ASCIIEncoding.hpp"
#include "Krystal.Text/Encodings/UTF16BigEndianEncoding.hpp"
#include "Krystal.Text/Encodings/UTF16LittleEndianEncoding.hpp"
#include "Krystal.Text/Encodings/UTF32BigEndianEncoding.hpp"
#include "Krystal.Text/Encodings/UTF32LittleEndianEncoding.hpp"
#include "Krystal.Text/Encodings/UTF8Encoding.hpp"

namespace Krys::Text::Encodings
{
  static ASCIIEncoding ASCII {};
  static UTF8Encoding UTF8 {};
  static UTF16LittleEndianEncoding UTF16LE {};
  static UTF16BigEndianEncoding UTF16BE {};
  static UTF32LittleEndianEncoding UTF32LE {};
  static UTF32BigEndianEncoding UTF32BE {};
}