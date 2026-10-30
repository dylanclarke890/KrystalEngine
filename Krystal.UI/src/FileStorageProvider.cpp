#include "Krystal.UI/FileStorageProvider.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <fstream>

namespace krys::ui
{
  namespace
  {
    /// @brief A local storage area that writes its items to a file whenever they change. The file holds each
    /// key and value as a 32-bit little-endian length followed by its UTF-8 bytes.
    class FileStorageArea final : public boo::MemoryStorageArea
    {
    private:
      std::filesystem::path _path;

    public:
      explicit FileStorageArea(std::filesystem::path path) noexcept
          : MemoryStorageArea(boo::StorageType::Local, DefaultLocalStorageQuota), _path(krys::move(path))
      {
        Read();
      }

      bool SetItem(const boo::dom::DOMString &key, const boo::dom::DOMString &value,
                   Maybe<boo::dom::DOMString> &oldValue) noexcept override
      {
        if (!MemoryStorageArea::SetItem(key, value, oldValue))
        {
          return false;
        }
        Write();
        return true;
      }

      Maybe<boo::dom::DOMString> RemoveItem(const boo::dom::DOMString &key) noexcept override
      {
        auto oldValue = MemoryStorageArea::RemoveItem(key);
        if (oldValue)
        {
          Write();
        }
        return oldValue;
      }

      bool Clear() noexcept override
      {
        bool hadItems = MemoryStorageArea::Clear();
        if (hadItems)
        {
          Write();
        }
        return hadItems;
      }

    private:
      void Read() noexcept
      {
        std::ifstream file(_path, std::ios::binary);
        if (!file)
        {
          return;
        }

        auto readString = [&](boo::dom::DOMString &string)
        {
          uint8 lengthBytes[4];
          if (!file.read(reinterpret_cast<char *>(lengthBytes), 4))
          {
            return false;
          }
          uint32 length = uint32 {lengthBytes[0]} | (uint32 {lengthBytes[1]} << 8)
                          | (uint32 {lengthBytes[2]} << 16) | (uint32 {lengthBytes[3]} << 24);
          string.resize(length);
          return static_cast<bool>(file.read(reinterpret_cast<char *>(string.data()), length));
        };

        boo::dom::DOMString key;
        boo::dom::DOMString value;
        while (readString(key) && readString(value))
        {
          _map.SetItemIgnoringQuota(key, value);
        }
      }

      void Write() noexcept
      {
        std::ofstream file(_path, std::ios::binary | std::ios::trunc);
        if (!file)
        {
          return;
        }

        auto writeString = [&](utf8_stringview string)
        {
          auto length = static_cast<uint32>(string.size());
          char lengthBytes[4] = {static_cast<char>(length & 0xFFu), static_cast<char>((length >> 8) & 0xFFu),
                                 static_cast<char>((length >> 16) & 0xFFu),
                                 static_cast<char>((length >> 24) & 0xFFu)};
          file.write(lengthBytes, 4);
          file.write(reinterpret_cast<const char *>(string.data()),
                     static_cast<std::streamsize>(string.size()));
        };

        for (auto &key : _map.Keys())
        {
          writeString(key);
          writeString(*_map.GetItem(key));
        }
      }
    };
  }

  FileStorageProvider::FileStorageProvider(std::filesystem::path directory) noexcept
      : _directory(krys::move(directory))
  {
    std::error_code error;
    std::filesystem::create_directories(_directory, error);
  }

  std::filesystem::path FileStorageProvider::PathForOrigin(boo::dom::DOMStringView origin) const noexcept
  {
    // An origin's characters other than ASCII letters and digits become underscores:
    // `https://example.com:8080` is `https___example_com_8080.storage`.
    std::string name;
    name.reserve(origin.size() + 8);
    for (auto c : origin)
    {
      name += text::IsASCIIAlphanumeric(c) ? static_cast<char>(c) : '_';
    }
    name += ".storage";
    return _directory / name;
  }

  Ref<boo::StorageArea> FileStorageProvider::LocalStorageArea(boo::dom::DOMStringView origin) noexcept
  {
    boo::dom::DOMString key(origin);
    auto it = _areas.find(key);
    if (it != _areas.end())
    {
      return it->second;
    }
    Ref<boo::StorageArea> area = CreateRef<FileStorageArea>(PathForOrigin(origin));
    _areas.emplace(krys::move(key), area);
    return area;
  }
}
