#include "Krystal.Booey/Platform/PasteboardCustomData.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    constexpr uint32 CurrentCustomDataSerializationVersion {1};

    void Encode(List<uint8> &bytes, uint32 value) noexcept
    {
      for (uint32 i = 0; i < 4; ++i)
      {
        bytes.push_back(static_cast<uint8>(value >> (i * 8)));
      }
    }

    void Encode(List<uint8> &bytes, utf8_stringview string) noexcept
    {
      Encode(bytes, static_cast<uint32>(string.size()));
      for (auto c : string)
      {
        bytes.push_back(static_cast<uint8>(c));
      }
    }

    /// @brief Reads what @ref Encode wrote, failing once it runs past the end.
    class Decoder
    {
    private:
      Span<const uint8> _bytes;
      size_t _offset {0};

    public:
      explicit Decoder(Span<const uint8> bytes) noexcept : _bytes(bytes)
      {
      }

      KRYS_NODISCARD Maybe<uint32> DecodeUInt32() noexcept
      {
        if (_bytes.size() - _offset < 4)
        {
          return null;
        }
        uint32 value = 0;
        for (uint32 i = 0; i < 4; ++i)
        {
          value |= static_cast<uint32>(_bytes[_offset++]) << (i * 8);
        }
        return value;
      }

      KRYS_NODISCARD Maybe<utf8_string> DecodeString() noexcept
      {
        auto length = DecodeUInt32();
        if (!length || _bytes.size() - _offset < *length)
        {
          return null;
        }
        utf8_string string(reinterpret_cast<const char8_t *>(_bytes.data() + _offset), *length);
        _offset += *length;
        return string;
      }
    };
  }

  // The serialised form is the version, the origin, and each type with its same-origin string, if any.
  Ref<SharedBuffer> PasteboardCustomData::CreateSharedBuffer() const noexcept
  {
    List<uint8> bytes;
    Encode(bytes, CurrentCustomDataSerializationVersion);
    Encode(bytes, _origin);
    Encode(bytes, static_cast<uint32>(_data.size()));
    for (auto &entry : _data)
    {
      Encode(bytes, entry.Type);
      Encode(bytes, entry.CustomData ? 1u : 0u);
      if (entry.CustomData)
      {
        Encode(bytes, *entry.CustomData);
      }
    }
    return SharedBuffer::Create(krys::move(bytes));
  }

  PasteboardCustomData PasteboardCustomData::FromSharedBuffer(const SharedBuffer &buffer) noexcept
  {
    constexpr uint32 maxSupportedDataSerializationVersionNumber {1};

    Decoder decoder(buffer.GetSpan());
    PasteboardCustomData result;
    auto version = decoder.DecodeUInt32();
    if (!version || *version > maxSupportedDataSerializationVersionNumber)
    {
      return {};
    }

    auto origin = decoder.DecodeString();
    if (!origin)
    {
      return {};
    }
    result._origin = krys::move(*origin);

    auto count = decoder.DecodeUInt32();
    if (!count)
    {
      return {};
    }
    for (uint32 i = 0; i < *count; ++i)
    {
      auto type = decoder.DecodeString();
      auto hasCustomData = decoder.DecodeUInt32();
      if (!type || !hasCustomData)
      {
        return {};
      }
      auto &entry = result.AddOrMoveEntryToEnd(*type);
      if (*hasCustomData)
      {
        auto customData = decoder.DecodeString();
        if (!customData)
        {
          return {};
        }
        entry.CustomData = krys::move(*customData);
      }
    }

    return result;
  }

  void PasteboardCustomData::WriteString(utf8_stringview type, utf8_stringview value) noexcept
  {
    AddOrMoveEntryToEnd(type).Data = Maybe<utf8_string>(utf8_string(value));
  }

  void PasteboardCustomData::WriteData(utf8_stringview type, Ref<SharedBuffer> &&data) noexcept
  {
    AddOrMoveEntryToEnd(type).Data = krys::move(data);
  }

  void PasteboardCustomData::WriteStringInCustomData(utf8_stringview type, utf8_stringview value) noexcept
  {
    AddOrMoveEntryToEnd(type).CustomData = utf8_string(value);
  }

  PasteboardCustomData::Entry &PasteboardCustomData::AddOrMoveEntryToEnd(utf8_stringview type) noexcept
  {
    auto it = std::ranges::find_if(_data, [&](auto &entry) { return entry.Type == type; });
    auto entry = it == _data.end() ? Entry(type) : krys::move(*it);
    if (it != _data.end())
    {
      _data.erase(it);
    }
    _data.push_back(krys::move(entry));
    return _data.back();
  }

  void PasteboardCustomData::Clear() noexcept
  {
    _data.clear();
  }

  void PasteboardCustomData::Clear(utf8_stringview type) noexcept
  {
    auto it = std::ranges::find_if(_data, [&](auto &entry) { return entry.Type == type; });
    if (it != _data.end())
    {
      _data.erase(it);
    }
  }

  List<utf8_string> PasteboardCustomData::OrderedTypes() const noexcept
  {
    List<utf8_string> types;
    types.reserve(_data.size());
    for (auto &entry : _data)
    {
      types.push_back(entry.Type);
    }
    return types;
  }

  bool PasteboardCustomData::HasData() const noexcept
  {
    return !_data.empty();
  }

  bool PasteboardCustomData::HasSameOriginCustomData() const noexcept
  {
    return std::ranges::any_of(_data, [](auto &entry) { return entry.CustomData.has_value(); });
  }

  RefPtr<SharedBuffer> PasteboardCustomData::ReadBuffer(utf8_stringview type) const noexcept
  {
    for (auto &entry : _data)
    {
      if (entry.Type != type)
      {
        continue;
      }

      if (auto *buffer = std::get_if<Ref<SharedBuffer>>(&entry.Data))
      {
        return *buffer;
      }

      return nullptr;
    }
    return nullptr;
  }

  utf8_string PasteboardCustomData::ReadString(utf8_stringview type) const noexcept
  {
    for (auto &entry : _data)
    {
      if (entry.Type != type)
      {
        continue;
      }

      if (auto *string = std::get_if<Maybe<utf8_string>>(&entry.Data); string && *string)
      {
        return **string;
      }

      return {};
    }
    return {};
  }

  Maybe<utf8_string> PasteboardCustomData::ReadStringInCustomData(utf8_stringview type) const noexcept
  {
    for (auto &entry : _data)
    {
      if (entry.Type == type)
      {
        return entry.CustomData;
      }
    }
    return null;
  }

  void PasteboardCustomData::ForEachType(const Func<void(const utf8_string &)> &function) const noexcept
  {
    for (auto &entry : _data)
    {
      function(entry.Type);
    }
  }

  void PasteboardCustomData::ForEachPlatformString(
    const Func<void(const utf8_string &type, const utf8_string &data)> &function) const noexcept
  {
    for (auto &entry : _data)
    {
      auto *string = std::get_if<Maybe<utf8_string>>(&entry.Data);
      if (!string || !*string)
      {
        continue;
      }

      function(entry.Type, **string);
    }
  }

  void PasteboardCustomData::ForEachCustomString(
    const Func<void(const utf8_string &type, const utf8_string &data)> &function) const noexcept
  {
    for (auto &entry : _data)
    {
      if (entry.CustomData)
      {
        function(entry.Type, *entry.CustomData);
      }
    }
  }

  void PasteboardCustomData::ForEachPlatformStringOrBuffer(
    const Func<void(const utf8_string &type, const PlatformData &data)> &function) const noexcept
  {
    for (auto &entry : _data)
    {
      auto *string = std::get_if<Maybe<utf8_string>>(&entry.Data);
      if (!string || *string)
      {
        function(entry.Type, entry.Data);
      }
    }
  }
}
