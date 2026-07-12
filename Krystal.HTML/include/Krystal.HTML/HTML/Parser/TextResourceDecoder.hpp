#pragma once

#include "Krystal.HTML/HTML/Enums/ContentType.hpp"
#include "Krystal.HTML/HTML/Enums/EncodingSource.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLEncodingSniffer.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/Codecs/ICodec.hpp"
#include "Krystal.Text/Decode/Decode.hpp"
#include "Krystal.Text/Unicode.hpp"
#include <cassert>

namespace Krys::HTML
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

    ReferenceWrapper<Krys::Text::CodecRegistry> _registry;
    ContentType _contentType {ContentType::PlainText};
    Krys::Text::ICodec *_decoder {nullptr};
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
    TextResourceDecoder(Krys::Text::CodecRegistry &registry, ContentType contentType,
                        const string &charset) noexcept
        : TextResourceDecoder(registry, contentType)
    {
      if (!charset.empty())
      {
        if (auto *codec = registry.Find(charset))
        {
          _decoder = codec;
          _source = EncodingSource::Transport;
          _confidence = Confidence::Certain;
        }
      }
    }

    TextResourceDecoder(Krys::Text::CodecRegistry &registry, const string &mimeType,
                        const string &charset) noexcept
        : TextResourceDecoder(registry, DetermineContentType(mimeType), charset)
    {
    }

    /// @see https://encoding.spec.whatwg.org/#utf-8-decode
    KRYS_NODISCARD static utf32_string TextFromUTF8(Krys::Text::CodecRegistry &registry,
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

      if (result.ErrorCode == Krys::Text::EncodingError::OK)
      {
        _buffer.clear();
        return result.Output;
      }

      if (result.ErrorCode == Krys::Text::EncodingError::IncompleteSequence)
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
        result.Output.push_back(Krys::Text::Unicode::Replacement<char32>);

        ResetForReuse();

        return result.Output;
      }

      assert(result.ErrorCode != Krys::Text::EncodingError::InsufficientOutputSpace);
      return result.Output;
    }

  private:
    TextResourceDecoder(Krys::Text::CodecRegistry &registry, ContentType contentType) noexcept
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
        return;

      using Krys::Text::operator""_s;
      _decoder = _registry.get().Find("utf-8"_s);
      assert(_decoder);
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
      using namespace Krys::Text;

      auto bom = Krys::Text::BOM::DetectByteOrderMark(_buffer);
      size_t bomLength = 0uz;

      switch (bom)
      {
        case Krys::Text::EncodingId::utf8:
          _decoder = _registry.get().Find("utf-8"_s);
          bomLength = 3uz;
          break;
        case Krys::Text::EncodingId::utf16le:
          _decoder = _registry.get().Find("utf-16le"_s);
          bomLength = 2uz;
          break;
        case Krys::Text::EncodingId::utf16be:
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
