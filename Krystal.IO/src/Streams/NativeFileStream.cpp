#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ios>
#include <Krystal.IO/Common.hpp>
#include <Krystal.IO/Path.hpp>
#include <Krystal.Lib/Types/Span.hpp>
#include <string>
#include <system_error>
#include <utility>

namespace Krys::IO
{
#pragma region Reader

  NativeFileReader::NativeFileReader(const Path &path, ReadFlags flags)
      : _path(path), _size(0uz), _flags(flags)
  {
    NativeFileReader::Open();
  }

  NativeFileReader::NativeFileReader(Path &&path, ReadFlags flags)
      : _path(std::move(path)), _size(0uz), _flags(flags)
  {
    NativeFileReader::Open();
  }

  NativeFileReader::~NativeFileReader() noexcept
  {
    NativeFileReader::Close();
  }

  bool NativeFileReader::IsOpen() const noexcept
  {
    return _stream.is_open();
  }

  bool NativeFileReader::Open()
  {
    if (NativeFileReader::IsOpen())
    {
      return true;
    }

    try
    {
      const auto &path = _path.NativePath();
      if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
      {
        return false; // Cannot open a reader for a non-regular file
      }

      _stream = std::ifstream(_path.NativePath(), std::ios::in | std::ios::binary);

      _stream.seekg(std::streamoff(0), std::ios_base::end);
      const std::streampos end = _stream.tellg();

      if (end != std::streampos(-1))
      {
        _size = static_cast<uint64>(end);
      }

      if ((_flags & ReadFlags::OpenAtEnd) != ReadFlags::OpenAtEnd)
      {
        _stream.seekg(std::streamoff(0), std::ios_base::beg);
      }
    }
    catch (...)
    {
      return false;
    }

    return _stream.is_open() && !_stream.fail();
  }

  void NativeFileReader::Close() noexcept
  {
    if (NativeFileReader::IsOpen())
    {
      try
      {
        _stream.close();
      }
      catch (...)
      {
        return;
      }
    }
  }

  size_t NativeFileReader::Read(Span<byte> destination) noexcept
  {
    if (destination.empty() || !IsOpen() || EndOfStream())
    {
      return 0;
    }

    try
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      _stream.read(reinterpret_cast<char *>(destination.data()),
                   static_cast<std::streamsize>(destination.size()));
      // Do you have an alternative to reinterpret casting?
    }
    catch (...)
    {
      return 0;
    }

    return _stream.gcount();
  }

  bool NativeFileReader::Seek(int64 offset, SeekOrigin origin) noexcept
  {
    if (!IsOpen())
    {
      return false;
    }

    const std::ios_base::seekdir seekdir = [&]()
    {
      switch (origin)
      {
        default:
        case SeekOrigin::Begin:   return std::ios_base::beg;
        case SeekOrigin::Current: return std::ios_base::cur;
        case SeekOrigin::End:     return std::ios_base::end;
      }
    }();

    try
    {
      _stream.seekg(offset, seekdir);
    }
    catch (...)
    {
      return false;
    }

    return !_stream.fail();
  }

  bool NativeFileReader::Peek(byte &next) noexcept
  {
    if (!IsOpen() || EndOfStream())
    {
      return false;
    }
    try
    {
      const int peeked = _stream.peek();
      if (peeked == std::char_traits<char>::eof())
      {
        return false;
      }
      next = static_cast<byte>(peeked);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  uint64 NativeFileReader::Size() const noexcept
  {
    if (!IsOpen())
    {
      return 0u;
    }

    return _size;
  }

  size_t NativeFileReader::Position() noexcept
  {
    if (!IsOpen())
    {
      return 0uz;
    }

    try
    {
      const std::streampos position = _stream.tellg();
      if (position == std::streampos(-1))
      {
        return 0uz;
      }

      return static_cast<size_t>(position);
    }
    catch (...)
    {
      return 0uz;
    }
  }

  bool NativeFileReader::EndOfStream() const noexcept
  {
    return _stream.eof();
  }

#pragma endregion

#pragma region Writer

  // NOLINTNEXTLINE(modernize-pass-by-value)
  NativeFileWriter::NativeFileWriter(const Path &path, WriteFlags flags)
      : _path(path), _size(0uz), _flags(flags)
  {
    NativeFileWriter::Open();
  }

  NativeFileWriter::NativeFileWriter(Path &&path, WriteFlags flags)
      : _path(std::move(path)), _size(0uz), _flags(flags)
  {
    NativeFileWriter::Open();
  }

  NativeFileWriter::~NativeFileWriter() noexcept
  {
    NativeFileWriter::Flush();
    NativeFileWriter::Close();
  }

  bool NativeFileWriter::IsOpen() const noexcept
  {
    return _stream.is_open();
  }

  bool NativeFileWriter::Open()
  {
    if (NativeFileWriter::IsOpen())
    {
      return true;
    }

    std::error_code ioError;

    const auto &path = _path.NativePath();
    auto status = std::filesystem::status(path, ioError);

    if (status.type() != std::filesystem::file_type::regular)
    {
      if (status.type() != std::filesystem::file_type::not_found)
      {
        return false; // Cannot open a writer for a non-regular file
      }

      if ((_flags & WriteFlags::Create) != WriteFlags::Create)
      {
        return false; // File does not exist and Create flag is not set
      }

      // Ensure parent directories exist
      const Path parent = _path.ParentPath();
      if (parent && !std::filesystem::exists(parent.NativePath(), ioError))
      {
        if (!std::filesystem::create_directories(parent.NativePath(), ioError) || ioError)
        {
          return false; // Failed to create parent directories
        }
      }
    }

    int32 openMode = std::ios::out | std::ios::binary;

    if ((_flags & WriteFlags::Truncate) == WriteFlags::Truncate)
    {
      openMode |= std::ios::trunc;
    }

    if ((_flags & WriteFlags::OpenAtEnd) == WriteFlags::OpenAtEnd)
    {
      openMode |= std::ios::ate;
    }

    try
    {
      _stream = std::ofstream(_path.NativePath(), openMode);

      const std::streampos currentPosition = _stream.tellp();
      _stream.seekp(0, std::ios_base::end); // Move to the end to get size
      _size = _stream.tellp();
      _stream.seekp(currentPosition); // Restore the original position
    }
    catch (...)
    {
      return false;
    }

    return _stream.is_open() && !_stream.fail();
  }

  void NativeFileWriter::Close() noexcept
  {
    if (_stream.is_open())
    {
      try
      {
        _stream.close();
      }
      catch (...)
      {
        return;
      }
    }
  }

  bool NativeFileWriter::Write(Span<const byte> source) noexcept
  {
    if (!NativeFileWriter::IsOpen())
    {
      return false;
    }

    try
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      _stream.write(reinterpret_cast<const char *>(source.data()),
                    static_cast<std::streampos>(source.size()));
      _size = std::max(_size, static_cast<size_t>(_stream.tellp()));
    }
    catch (...)
    {
      return false;
    }

    return !_stream.fail();
  }

  bool NativeFileWriter::Seek(int64 offset, SeekOrigin origin) noexcept
  {
    if (!IsOpen())
    {
      return false;
    }

    const std::ios_base::seekdir dir = [&]()
    {
      switch (origin)
      {
        case SeekOrigin::Begin:   return std::ios_base::beg;
        case SeekOrigin::Current: return std::ios_base::cur;
        case SeekOrigin::End:     return std::ios_base::end;
      }
      return std::ios_base::beg; // Default case, should not happen
    }();

    try
    {
      _stream.seekp(offset, dir);
    }
    catch (...)
    {
      return false;
    }

    return _stream.good();
  }

  uint64 NativeFileWriter::Size() const noexcept
  {
    return _size;
  }

  uint64 NativeFileWriter::Position() noexcept
  {
    if (!NativeFileWriter::IsOpen())
    {
      return 0;
    }
    return _stream.tellp();
  }

  void NativeFileWriter::Flush() noexcept
  {
    if (NativeFileWriter::IsOpen())
    {
      try
      {
        _stream.flush();
      }
      catch (...)
      {
        return;
      }
    }
  }

#pragma endregion
}