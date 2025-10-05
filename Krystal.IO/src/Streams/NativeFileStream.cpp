#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.Lib/Types.hpp"
#include <filesystem>
#include <ios>

namespace Krys::IO
{
#pragma region Reader

  NativeFileReader::NativeFileReader(const Path &path) : _path(path)
  {
    NativeFileReader::Open();
  }

  NativeFileReader::NativeFileReader(Path &&path) : _path(std::move(path))
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

  bool NativeFileReader::Open() noexcept
  {
    try
    {
      const auto path = _path.ToString();
      if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
      {
        return false; // Cannot open a reader for a non-regular file
      }

      _stream = std::ifstream(_path.ToString(), std::ios::in | std::ios::binary);
    }
    catch (...)
    {
      return false;
    }

    return _stream.is_open() && !_stream.fail();
  }

  void NativeFileReader::Close() noexcept
  {
    if (_stream.is_open())
    {
      try
      {
        _stream.close();
      }
      // NOLINTNEXTLINE(bugprone-empty-catch)
      catch (...)
      {
      }
    }
  }

  uint64 NativeFileReader::Read(byte *dest, uint64 count) noexcept
  {
    if (!IsOpen() || count == 0 || EndOfStream())
    {
      return 0;
    }

    try
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      _stream.read(reinterpret_cast<char *>(dest), (int64)count);
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
      _stream.seekg(offset, dir);
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

  uint64 NativeFileReader::Size() noexcept
  {
    if (!IsOpen())
    {
      return 0;
    }

    std::streampos size;
    try
    {
      const std::streampos currentPos = _stream.tellg();
      _stream.seekg(0, std::ios_base::end); // Move to the end to get size
      size = _stream.tellg();               // Get the size
      _stream.seekg(currentPos);            // Restore the original position
    }
    catch (...)
    {
      return 0;
    }

    return size;
  }

  uint64 NativeFileReader::Position() noexcept
  {
    if (!IsOpen())
    {
      return 0;
    }
    return _stream.tellg();
  }

  bool NativeFileReader::EndOfStream() const noexcept
  {
    return _stream.eof();
  }

#pragma endregion

#pragma region Writer

  NativeFileWriter::NativeFileWriter(const Path &path) : _path(path)
  {
    NativeFileWriter::Open();
  }

  NativeFileWriter::NativeFileWriter(Path &&path) : _path(std::move(path))
  {
    NativeFileWriter::Open();
  }

  NativeFileWriter::~NativeFileWriter() noexcept
  {
    NativeFileWriter::Close();
  }

  bool NativeFileWriter::IsOpen() const noexcept
  {
    return _stream.is_open();
  }

  bool NativeFileWriter::Open() noexcept
  {
    try
    {
      const auto path = _path.ToString();
      if (std::filesystem::exists(path) && !std::filesystem::is_regular_file(path))
      {
        return false; // Cannot open a writer for a non-regular file
      }
      _stream = std::ofstream(_path.ToString(), std::ios::out | std::ios::binary);
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
      // NOLINTNEXTLINE(bugprone-empty-catch)
      catch (...)
      {
      }
    }
  }

  bool NativeFileWriter::Write(const byte *src, uint64 size) noexcept
  {
    if (!IsOpen())
    {
      return false;
    }

    try
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      _stream.write(reinterpret_cast<const char *>(src), (int64)size);
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

  uint64 NativeFileWriter::Size() noexcept
  {
    if (!IsOpen())
    {
      return 0;
    }

    uint64 size {};
    try
    {
      const std::streampos currentPos = _stream.tellp();
      _stream.seekp(0, std::ios_base::end); // Move to the end to get size
      size = _stream.tellp();
      _stream.seekp(currentPos); // Restore the original position
    }
    catch (...)
    {
      return 0;
    }

    return size;
  }

  uint64 NativeFileWriter::Position() noexcept
  {
    if (!IsOpen())
    {
      return 0;
    }
    return _stream.tellp();
  }

  void NativeFileWriter::Flush() noexcept
  {
    if (IsOpen())
    {
      try
      {
        _stream.flush();
      }
      // NOLINTNEXTLINE(bugprone-empty-catch)
      catch (...)
      {
      }
    }
  }

  bool NativeFileWriter::EndOfStream() const noexcept
  {
    return _stream.eof();
  }

#pragma endregion
}