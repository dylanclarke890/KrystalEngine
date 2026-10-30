#pragma once

#include "Krystal.Booey/Core/EncodingRegistry.hpp"
#include "Krystal.Booey/HTML/Enums/ContentType.hpp"
#include "Krystal.Booey/HTML/Enums/EncodingSource.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLEncodingSniffer.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/ICodec.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"

namespace krys::boo::html
{
  struct IsFinalChunk : StronglyTypedBool<IsFinalChunk>
  {
    using StronglyTypedBool::StronglyTypedBool;
  };

  class TextResourceDecoder
  {
  public:
    struct IsFinalChunk : StronglyTypedBool<IsFinalChunk>
    {
      using StronglyTypedBool::StronglyTypedBool;
    };

  private:
    enum class Confidence : uint8
    {
      Certain,
      Tentative,
    };

    ContentType _contentType {ContentType::PlainText};
    krys::text::ICodec *_decoder {nullptr};
    EncodingSource _source {EncodingSource::Default};
    Confidence _confidence {Confidence::Tentative};
    bool _useBOMDetection : 1 = true;
    bool _checkedForBOM : 1 = false;
    bool _useAutoDetect : 1 = true;
    bool _checkedAutoDetect : 1 = false;
    bool _alwaysUseUTF8 : 1 = false;
    bool _decodingStarted : 1 = false;
    List<byte> _buffer;

  public:
    TextResourceDecoder(ContentType contentType, utf8_stringview charset) noexcept : _contentType(contentType)
    {
      if (!charset.empty())
      {
        if (auto *codec = EncodingRegistry().Find(charset))
        {
          _decoder = codec;
          _source = EncodingSource::Transport;
          _confidence = Confidence::Certain;
        }
      }
    }

    TextResourceDecoder(const string &mimeType, utf8_stringview charset) noexcept
        : TextResourceDecoder(DetermineContentType(mimeType), charset)
    {
    }

    /// @see https://encoding.spec.whatwg.org/#utf-8-decode
    KRYS_NODISCARD static utf32_string TextFromUTF8(krys::text::CodecRegistry &registry,
                                                    Span<const byte> data)
    {
      constexpr Array<byte, 3> byteOrderMarkUTF8 = {byte {0xEF}, byte {0xBB}, byte {0xBF}};
      auto decoder = TextResourceDecoder(registry, "text/plain", "UTF-8");
      if (!ByteUtils::Compare<3>(data, byteOrderMarkUTF8))
      {
        (void)decoder.Decode(byteOrderMarkUTF8);
      }
      return decoder.Decode(data, IsFinalChunk(true));
    }

    /// Unified streaming decode entry point
    KRYS_NODISCARD utf32_string Decode(Span<const byte> bytes, IsFinalChunk isFinal = IsFinalChunk(false))
    {
      if (!_decodingStarted)
      {
        if (_useBOMDetection && !_checkedForBOM)
        {
          if (bytes.size() + _buffer.size() < 3 && !isFinal)
          {
            _buffer.append_range(bytes);
            return {};
          }

          AppendBytes(bytes);
          ScanBOM();
          _checkedForBOM = true;
          bytes = {};
        }

        if (_confidence != Confidence::Certain && _useAutoDetect && !_checkedAutoDetect
            && _contentType == ContentType::HTML)
        {
          if (_buffer.size() < 1'024 && !isFinal)
          {
            AppendBytes(bytes); // await more data for auto-detect
            return {};
          }

          if (auto *detected = HTMLEncodingSniffer::Detect(_buffer, _registry.get()))
          {
            _decoder = detected;
            _source = EncodingSource::MetaTag;
            _confidence = Confidence::Tentative;
          }

          _checkedAutoDetect = true;
        }

        EnsureDecoder();
        _decodingStarted = true;
      }

      AppendBytes(bytes);

      auto result = _decoder->Decode(_buffer);

      if (result.ErrorCode == krys::Text::EncodingError::OK)
      {
        _buffer.clear();
        return result.Output;
      }

      if (result.ErrorCode == krys::Text::EncodingError::IncompleteSequence)
      {
        if (!isFinal)
        {
          // keep remainder for next chunk
          List<byte> remainder;
          remainder.append_range(result.Input);
          _buffer = std::move(remainder);
          return result.Output;
        }

        // Final chunk: emit replacement and finish
        result.Output.push_back(krys::text::UnicodeReplacementChar);

        ResetForReuse();

        return result.Output;
      }

      krys_debug_assert(result.ErrorCode != krys::Text::EncodingError::InsufficientOutputSpace);
      return result.Output;
    }

  private:
    TextResourceDecoder(krys::Text::CodecRegistry &registry, ContentType contentType) noexcept
        : _registry(registry), _contentType(contentType)
    {
    }

    void AppendBytes(Span<const byte> bytes)
    {
      if (!bytes.empty())
      {
        _buffer.append_range(bytes);
      }
    }

    void EnsureDecoder()
    {
      if (_decoder)
      {
        return;
      }

      _decoder = EncodingRegistry().Find(u8"utf-8");
      krys_assert(_decoder);

      _source = EncodingSource::Default;
      _confidence = Confidence::Tentative;
    }

    void ResetForReuse()
    {
      _buffer.clear();
      _checkedForBOM = false;
      _checkedAutoDetect = false;
      _decodingStarted = false;
      _confidence = Confidence::Tentative;
    }

    void ScanBOM()
    {
      using namespace krys::Text;

      auto bom = krys::Text::BOM::DetectByteOrderMark(_buffer);
      size_t bomLength = 0uz;

      switch (bom)
      {
        case krys::Text::EncodingId::utf8:
          _decoder = _registry.get().Find("utf-8"_s);
          bomLength = 3uz;
          break;
        case krys::Text::EncodingId::utf16le:
          _decoder = _registry.get().Find("utf-16le"_s);
          bomLength = 2uz;
          break;
        case krys::Text::EncodingId::utf16be:
          _decoder = _registry.get().Find("utf-16be"_s);
          bomLength = 2uz;
          break;
        default: return;
      }

      _buffer.erase(_buffer.begin(), _buffer.begin() + bomLength);
      _source = EncodingSource::BOM;
      _confidence = Confidence::Certain;
    }
  };
}
