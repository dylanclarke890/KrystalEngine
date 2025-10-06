#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include <algorithm>
#include <cstring>

namespace Krys::IO
{
  /// @brief A stream that reads from a memory buffer.
  class MemoryStreamReader : public IStreamReader
  {
    List<byte> &_buffer;
    uint64 _position = 0;
    bool _isOpen = false;
    ReadFlags _flags;

  public:
    static constexpr ReadFlags DefaultReadFlags = ReadFlags::None;

    NO_COPY_MOVE(MemoryStreamReader)

    explicit MemoryStreamReader(List<byte> &buffer, ReadFlags flags = DefaultReadFlags) noexcept
        : _buffer(buffer), _flags(flags)
    {
      MemoryStreamReader::Open();
    }

    ~MemoryStreamReader() noexcept override
    {
      MemoryStreamReader::Close();
    }

    /// @brief Checks if the stream is open.
    NO_DISCARD bool IsOpen() const noexcept override
    {
      return _isOpen;
    }

    /// @brief Opens the stream. If the stream is already open, this function does nothing.
    bool Open() noexcept override
    {
      _isOpen = true;
      return _isOpen;
    }

    /// @brief Closes the stream. If the stream is already closed, this function does nothing.
    void Close() noexcept override
    {
      _isOpen = false;
      _position = 0;
    }

    /// @brief Reads 'count' bytes from the stream.
    /// @return The number of bytes actually read from the stream.
    uint64 Read(byte *dst, uint64 count) noexcept override
    {
      if (dst == nullptr || count == 0 || !_isOpen || EndOfStream())
      {
        return 0;
      }

      const uint64 remainingBytes = static_cast<uint64>(_buffer.size()) - _position;
      const uint64 bytesRead = std::min(count, remainingBytes);

      std::memcpy(dst, &_buffer[_position], bytesRead);
      _position += bytesRead;

      return bytesRead;
    }

    /// @brief Seeks to a specific position in the stream.
    /// @return True if the seek operation was successful, false otherwise.
    bool Seek(int64 offset, SeekOrigin origin = SeekOrigin::Current) noexcept override
    {
      if (!_isOpen)
      {
        return false;
      }

      int64 newPosition = 0;
      switch (origin)
      {
        case SeekOrigin::Begin:   newPosition = offset; break;
        case SeekOrigin::Current: newPosition = static_cast<int64>(_position) + offset; break;
        case SeekOrigin::End:     newPosition = static_cast<int64>(_buffer.size()) + offset; break;
        default:                  return false;
      }

      if (newPosition < 0 || newPosition > static_cast<int64>(_buffer.size()))
      {
        return false;
      }

      _position = newPosition;
      return true;
    }

    /// @brief Peeks at the next byte in the stream without advancing the position.
    NO_DISCARD bool Peek(byte &next) noexcept override
    {
      if (!_isOpen || _position >= _buffer.size())
      {
        return false;
      }
      next = _buffer[_position];
      return true;
    }

    /// @brief Gets the total size of the stream in bytes, or 0 if the size is unknown.
    NO_DISCARD uint64 Size() const noexcept override
    {
      return static_cast<uint64>(_buffer.size());
    }

    /// @brief Gets the current position in the stream.
    NO_DISCARD uint64 Position() noexcept override
    {
      return _position;
    }

    /// @brief Checks if the end of the stream has been reached.
    /// @return True if the end of the stream has been reached, false otherwise.
    NO_DISCARD bool EndOfStream() const noexcept override
    {
      return _position >= static_cast<uint64>(_buffer.size());
    }
  };

  /// @brief A stream that writes to a memory buffer.
  class MemoryStreamWriter : public IStreamWriter
  {
    List<byte> &_buffer;
    uint64 _position = 0;
    bool _isOpen = false;
    WriteFlags _flags;

  public:
    static constexpr WriteFlags DefaultWriteFlags = WriteFlags::None;

    NO_COPY_MOVE(MemoryStreamWriter)

    explicit MemoryStreamWriter(List<byte> &buffer, WriteFlags flags = DefaultWriteFlags) noexcept
        : _buffer(buffer), _flags(flags)
    {
      MemoryStreamWriter::Open();
    }

    ~MemoryStreamWriter() noexcept override
    {
      MemoryStreamWriter::Close();
    }

    /// @brief Checks if the stream is open.
    NO_DISCARD bool IsOpen() const noexcept override
    {
      return _isOpen;
    }

    /// @brief Opens the stream. If the stream is already open, this function does nothing.
    bool Open() noexcept override
    {
      if (_isOpen)
      {
        return true;
      }

      _isOpen = true;

      if ((_flags & WriteFlags::Truncate) == WriteFlags::Truncate)
      {
        _buffer.clear();
      }

      if ((_flags & WriteFlags::OpenAtEnd) == WriteFlags::OpenAtEnd)
      {
        _position = static_cast<uint64>(_buffer.size());
      }

      return _isOpen;
    }

    /// @brief Closes the stream. If the stream is already closed, this function does nothing.
    void Close() noexcept override
    {
      _isOpen = false;
      _position = 0;
    }

    /// @brief Writes 'count' bytes to the stream.
    bool Write(const byte *src, uint64 count) noexcept override
    {
      if (!_isOpen || src == nullptr)
      {
        return false;
      }

      try
      {
        // ensure the buffer can accommodate the new data
        if (_position + count > static_cast<uint64>(_buffer.size()))
        {
          _buffer.resize(_position + count);
        }

        std::memcpy(&_buffer[_position], src, count);
        _position += count;

        return true;
      }
      catch (...)
      {
        return false;
      }
    }

    /// @brief Seeks to a specific position in the stream.
    /// @return True if the seek operation was successful, false otherwise.
    bool Seek(int64 offset, SeekOrigin origin = SeekOrigin::Current) noexcept override
    {
      if (!_isOpen)
      {
        return false;
      }

      int64 newPosition = 0;
      switch (origin)
      {
        case SeekOrigin::Begin:   newPosition = offset; break;
        case SeekOrigin::Current: newPosition = static_cast<int64>(_position) + offset; break;
        case SeekOrigin::End:     newPosition = static_cast<int64>(_buffer.size()) + offset; break;
        default:                  return false;
      }

      if (newPosition < 0)
      {
        return false;
      }

      _position = static_cast<uint64>(newPosition);
      return true;
    }

    /// @brief Gets the total size of the stream in bytes, or 0 if the size is unknown.
    NO_DISCARD uint64 Size() const noexcept override
    {
      return static_cast<uint64>(_buffer.size());
    }

    /// @brief Gets the current position in the stream.
    NO_DISCARD uint64 Position() noexcept override
    {
      return _position;
    }

    void Flush() noexcept override
    {
      // No-op for memory stream
    }
  };
}