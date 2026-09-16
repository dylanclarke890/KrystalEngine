#include "Krystal.Booey/Core/EncodingRegistry.hpp"
#include "Krystal.Core/Text/Encodings/Big5-HKSCS.hpp"
#include "Krystal.Core/Text/Encodings/EUC-KR-UHC.hpp"
#include "Krystal.Core/Text/Encodings/GB18030.hpp"
#include "Krystal.Core/Text/Encodings/ISO-8859.hpp"
#include "Krystal.Core/Text/Encodings/KOI8.hpp"
#include "Krystal.Core/Text/Encodings/Macintosh.hpp"
#include "Krystal.Core/Text/Encodings/ShiftJIS.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "Krystal.Core/Text/Encodings/Windows.hpp"

namespace krys::boo
{
  const krys::text::EncodingRegistry &EncodingRegistry() noexcept
  {
    static krys::text::EncodingRegistry registry = []() -> krys::text::EncodingRegistry
    {
      krys::text::EncodingRegistry er;

      er.Register<krys::text::Big5_HKSCS_2001>();
      er.Register<krys::text::EUC_KR_UHC>();
      er.Register<krys::text::GBK>();
      er.Register<krys::text::GB18030>();
      er.Register<krys::text::ISO_8859_2>();
      er.Register<krys::text::ISO_8859_7>();
      er.Register<krys::text::ISO_8859_8>();
      er.Register<krys::text::KOI8U>();
      er.Register<krys::text::KOI8R>();
      er.Register<krys::text::MacRoman>();
      er.Register<krys::text::MacCyrillic>();
      er.Register<krys::text::ShiftJISx0208>();
      er.Register<krys::text::UTF8>();
      er.Register<krys::text::UTF16BE>();
      er.Register<krys::text::UTF16LE>();
      er.Register<krys::text::Windows874>();
      er.Register<krys::text::Windows1250>();
      er.Register<krys::text::Windows1251>();
      er.Register<krys::text::Windows1252>();
      er.Register<krys::text::Windows1253>();
      er.Register<krys::text::Windows1254>();
      er.Register<krys::text::Windows1255>();
      er.Register<krys::text::Windows1256>();
      er.Register<krys::text::Windows1257>();
      er.Register<krys::text::Windows1258>();

      return er;
    }();

    return registry;
  }
}