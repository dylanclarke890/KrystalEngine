#pragma once

#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  /// @brief The data a page put on a pasteboard: strings and buffers by type, in the order written, the
  /// strings only pages of the same origin may read back, and that origin.
  class PasteboardCustomData
  {
  public:
    using PlatformData = Variant<Maybe<utf8_string>, Ref<SharedBuffer>>;

    struct Entry
    {
      utf8_string Type;
      /// @brief The string only pages of the same origin may read, or none.
      Maybe<utf8_string> CustomData;
      /// @brief The string or buffer written to the platform pasteboard; no string if none.
      PlatformData Data;

      Entry() noexcept = default;
      explicit Entry(utf8_stringview type) noexcept : Type(type)
      {
      }
      Entry(utf8_stringview type, Maybe<utf8_string> customData, PlatformData data) noexcept
          : Type(type), CustomData(krys::move(customData)), Data(krys::move(data))
      {
      }
    };

  private:
    utf8_string _origin;
    List<Entry> _data;

  public:
    PasteboardCustomData() noexcept = default;
    PasteboardCustomData(utf8_string &&origin, List<Entry> &&data) noexcept
        : _origin(krys::move(origin)), _data(krys::move(data))
    {
    }

    KRYS_NODISCARD const utf8_string &Origin() const noexcept
    {
      return _origin;
    }
    void SetOrigin(utf8_stringview origin) noexcept
    {
      _origin = utf8_string(origin);
    }

    /// @brief The origin, the same-origin strings and the types, serialised for a platform pasteboard.
    KRYS_NODISCARD Ref<SharedBuffer> CreateSharedBuffer() const noexcept;
    /// @brief The data serialised in @p buffer, or empty data if it isn't valid.
    KRYS_NODISCARD static PasteboardCustomData FromSharedBuffer(const SharedBuffer &buffer) noexcept;

    /// @brief The type of the private platform pasteboard format the serialised data is kept in.
    KRYS_NODISCARD static constexpr utf8_stringview BooeyType() noexcept
    {
      return u8"org.krystalengine.booey.custom-pasteboard-data";
    }

    /// @brief The platform string of @p type, or empty.
    KRYS_NODISCARD utf8_string ReadString(utf8_stringview type) const noexcept;
    KRYS_NODISCARD RefPtr<SharedBuffer> ReadBuffer(utf8_stringview type) const noexcept;
    /// @brief The same-origin string of @p type, or none.
    KRYS_NODISCARD Maybe<utf8_string> ReadStringInCustomData(utf8_stringview type) const noexcept;

    void WriteString(utf8_stringview type, utf8_stringview value) noexcept;
    void WriteData(utf8_stringview type, Ref<SharedBuffer> &&data) noexcept;
    void WriteStringInCustomData(utf8_stringview type, utf8_stringview value) noexcept;

    void Clear() noexcept;
    void Clear(utf8_stringview type) noexcept;

    void ForEachType(const Func<void(const utf8_string &)> &function) const noexcept;
    void ForEachPlatformString(
      const Func<void(const utf8_string &type, const utf8_string &data)> &function) const noexcept;
    void ForEachPlatformStringOrBuffer(
      const Func<void(const utf8_string &type, const PlatformData &data)> &function) const noexcept;
    void ForEachCustomString(
      const Func<void(const utf8_string &type, const utf8_string &data)> &function) const noexcept;

    KRYS_NODISCARD bool HasData() const noexcept;
    KRYS_NODISCARD bool HasSameOriginCustomData() const noexcept;

    KRYS_NODISCARD List<utf8_string> OrderedTypes() const noexcept;

    KRYS_NODISCARD const List<Entry> &Data() const noexcept
    {
      return _data;
    }

  private:
    Entry &AddOrMoveEntryToEnd(utf8_stringview type) noexcept;
  };
}
