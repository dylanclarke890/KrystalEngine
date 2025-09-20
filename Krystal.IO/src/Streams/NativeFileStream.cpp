#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include <filesystem>
#include <ios>

namespace Krys::IO
{
#pragma region Reader

  NativeFileReader::NativeFileReader(const Path &path) noexcept : _path(path)
  {
    Open();
  }

  NativeFileReader::~NativeFileReader() noexcept
  {
    Close();
  }

  bool NativeFileReader::IsOpen() const noexcept
  {
    return _stream.is_open();
  }

  bool NativeFileReader::Open() noexcept
  {
    const std::filesystem::path path = _path.ToString();
    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
    {
      return false; // Cannot open a reader for a non-regular file
    }

    _stream = std::ifstream(_path.ToString(), std::ios::in | std::ios::binary);
    return _stream.is_open() && !_stream.fail();
  }

  void NativeFileReader::Close() noexcept
  {
    if (_stream.is_open())
    {
      _stream.close();
    }
  }

  // NOLINTNEXTLINE(readability-make-member-function-const)
  uint64 NativeFileReader::Read(byte *dest, uint64 size) noexcept
  {
    if (!IsOpen())
    {
      return 0;
    }
    _stream.read(reinterpret_cast<char *>(dest), size);
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

    _stream.seekg(offset, dir);
    return !_stream.fail();
  }

  uint64 NativeFileReader::Size() noexcept
  {
    if (!IsOpen())
    {
      return 0;
    }
    std::streampos currentPos = _stream.tellg();
    _stream.seekg(0, std::ios_base::end);  // Move to the end to get size
    std::streampos size = _stream.tellg(); // Get the size
    _stream.seekg(currentPos);             // Restore the original position
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

  NativeFileWriter::NativeFileWriter(const Path &path) noexcept : _path(path)
  {
    Open();
  }

  NativeFileWriter::~NativeFileWriter() noexcept
  {
    Close();
  }

  bool NativeFileWriter::IsOpen() const noexcept
  {
    return _stream.is_open();
  }

  bool NativeFileWriter::Open() noexcept
  {
    const std::filesystem::path path = _path.ToString();
    if (std::filesystem::exists(path) && !std::filesystem::is_regular_file(path))
    {
      return false; // Cannot open a writer for a non-regular file
    }

    _stream = std::ofstream(_path.ToString(), std::ios::out | std::ios::binary);
    return _stream.is_open() && !_stream.fail();
  }

  void NativeFileWriter::Close() noexcept
  {
    if (_stream.is_open())
    {
      _stream.close();
    }
  }

  // NOLINTNEXTLINE(readability-make-member-function-const)
  bool NativeFileWriter::Write(const byte *src, uint64 size) noexcept
  {
    if (!IsOpen())
    {
      return false;
    }

    _stream.write(reinterpret_cast<const char *>(src), size);
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

    _stream.seekp(offset, dir);
    return _stream.good();
  }

  uint64 NativeFileWriter::Size() noexcept
  {
    if (!IsOpen())
    {
      return 0;
    }

    std::streampos currentPos = _stream.tellp();
    _stream.seekp(0, std::ios_base::end); // Move to the end to get size

    uint64 size = _stream.tellp(); // NOLINT(cppcoreguidelines-init-variables)

    _stream.seekp(currentPos); // Restore the original position
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

  bool NativeFileWriter::EndOfStream() const noexcept
  {
    return _stream.eof();
  }

#pragma endregion
}