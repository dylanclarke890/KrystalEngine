#include "Krystal.HTML/Context.hpp"
#include "Krystal.Text/Codecs/BasicCodec.hpp"
#include "Krystal.Text/Encodings/Big5-HKSCS.hpp"
#include "Krystal.Text/Encodings/EUC-KR-UHC.hpp"
#include "Krystal.Text/Encodings/GB18030.hpp"
#include "Krystal.Text/Encodings/GBK.hpp"
#include "Krystal.Text/Encodings/ISO_8859_2.hpp"
#include "Krystal.Text/Encodings/ISO_8859_7.hpp"
#include "Krystal.Text/Encodings/ISO_8859_8.hpp"
#include "Krystal.Text/Encodings/KOI8R.hpp"
#include "Krystal.Text/Encodings/KOI8U.hpp"
#include "Krystal.Text/Encodings/MacCyrillic.hpp"
#include "Krystal.Text/Encodings/MacRoman.hpp"
#include "Krystal.Text/Encodings/ShiftJISx0208.hpp"
#include "Krystal.Text/Encodings/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/Encodings/Windows1250.hpp"
#include "Krystal.Text/Encodings/Windows1251.hpp"
#include "Krystal.Text/Encodings/Windows1252.hpp"
#include "Krystal.Text/Encodings/Windows1253.hpp"
#include "Krystal.Text/Encodings/Windows1254.hpp"
#include "Krystal.Text/Encodings/Windows1255.hpp"
#include "Krystal.Text/Encodings/Windows1256.hpp"
#include "Krystal.Text/Encodings/Windows1257.hpp"
#include "Krystal.Text/Encodings/Windows1258.hpp"
#include "Krystal.Text/Encodings/Windows874.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace
{
  using namespace Krys;
  using namespace Krys::Text;

  template <template <typename...> class Encoding>
  using Decoder = BasicCodec<Encoding<byte, Text::UnicodeCodePoint>>;

  template <typename T>
  using SchemeDecoder = BasicCodec<T>;
}

namespace Krys::HTML
{
  Context CreateContext() noexcept
  {
    using namespace ::Krys::Text;

    Context context {};

    context.DecoderRegistry.Register<Decoder<basic_big5_hkscs>>();
    context.DecoderRegistry.Register<Decoder<basic_euc_kr_uhc>>();
    context.DecoderRegistry.Register<Decoder<basic_gb18030>>();
    context.DecoderRegistry.Register<Decoder<basic_gbk>>();
    context.DecoderRegistry.Register<Decoder<basic_iso_8859_2>>();
    context.DecoderRegistry.Register<Decoder<basic_iso_8859_7>>();
    context.DecoderRegistry.Register<Decoder<basic_iso_8859_8>>();
    context.DecoderRegistry.Register<Decoder<basic_koi8_r>>();
    context.DecoderRegistry.Register<Decoder<basic_koi8_u>>();
    context.DecoderRegistry.Register<Decoder<basic_mac_cyrillic>>();
    context.DecoderRegistry.Register<Decoder<basic_mac_roman>>();
    context.DecoderRegistry.Register<Decoder<basic_shift_jis_x0208>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_874>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1250>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1251>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1252>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1253>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1254>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1255>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1256>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1257>>();
    context.DecoderRegistry.Register<Decoder<basic_windows_1258>>();
    context.DecoderRegistry.Register<Decoder<basic_utf8>>();

    return context;
  }
}