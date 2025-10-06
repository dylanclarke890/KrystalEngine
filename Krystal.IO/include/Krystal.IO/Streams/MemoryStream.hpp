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
  class MemoryStreamReader : public IStreamReader
  {
    List<byte> &_buffer;
    uint64 _position = 0;
    bool _isOpen = false;

  public:
    NO_COPY_MOVE(MemoryStreamReader)

    explicit MemoryStreamReader(List<byte> &buffer) noexcept : _buffer(buffer)
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
      if (!_isOpen || count == 0 || EndOfStream())
      {
        return 0;
      }

      const auto remainingBytes = _buffer.size() - _position;
      const auto &bytesRead = std::min(count, remainingBytes);

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
    NO_DISCARD uint64 Size() noexcept override
    {
      return _buffer.size();
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
      return _position >= _buffer.size();
    }
  };

  class MemoryStreamWriter : public IStreamWriter
  {
    List<byte> &_buffer;
    uint64 _position = 0;
    bool _isOpen = false;

  public:
    NO_COPY_MOVE(MemoryStreamWriter)

    explicit MemoryStreamWriter(List<byte> &buffer) noexcept : _buffer(buffer), _isOpen(true)
    {
    }

    ~MemoryStreamWriter() noexcept override
    {
      _isOpen = false;
      _position = 0;
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
        if (_position + count > _buffer.size())
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
    NO_DISCARD uint64 Size() noexcept override
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