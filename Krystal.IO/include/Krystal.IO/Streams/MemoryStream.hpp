#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/Streams/Stream.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <algorithm>
#include <cstring>

namespace Krys::IO
{
  /// @brief A stream that reads from a memory buffer.
  class MemoryStreamReader : public IStreamReader
  {
    List<byte> &_buffer;
    size_t _position = 0uz;
    bool _isOpen = false;
    ReadFlags _flags;

  public:
    constexpr static ReadFlags DefaultReadFlags = ReadFlags::None;

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
    KRYS_NODISCARD bool IsOpen() const noexcept override
    {
      return _isOpen;
    }

    /// @brief Opens the stream. If the stream is already open, this function does nothing.
    bool Open() override
    {
      if (_isOpen)
      {
        return true;
      }

      _isOpen = true;

      if ((_flags & ReadFlags::OpenAtEnd) == ReadFlags::OpenAtEnd)
      {
        _position = _buffer.size();
      }

      return true;
    }

    /// @brief Closes the stream. If the stream is already closed, this function does nothing.
    void Close() noexcept override
    {
      _isOpen = false;
      _position = 0uz;
    }

    /// @brief Reads bytes from the stream into the destination buffer.
    /// @return The number of bytes actually read from the stream.
    size_t Read(Span<byte> destination) noexcept override
    {
      if (!_isOpen || destination.empty() || EndOfStream())
      {
        return 0;
      }

      const size_t remaining = _buffer.size() - _position;
      const size_t bytesRead = std::min(destination.size(), remaining);

      std::memcpy(destination.data(), &_buffer[_position], bytesRead);
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

      _position = static_cast<size_t>(newPosition);
      return true;
    }

    /// @brief Peeks at the next byte in the stream without advancing the position.
    KRYS_NODISCARD bool Peek(byte &next) noexcept override
    {
      if (!_isOpen || EndOfStream())
      {
        return false;
      }

      next = _buffer[_position];
      return true;
    }

    /// @brief Gets the total size of the stream in bytes, or 0 if the size is unknown.
    KRYS_NODISCARD size_t Size() const noexcept override
    {
      return _buffer.size();
    }

    /// @brief Gets the current position in the stream.
    KRYS_NODISCARD size_t Position() noexcept override
    {
      return _position;
    }

    /// @brief Checks if the end of the stream has been reached.
    /// @return True if the end of the stream has been reached, false otherwise.
    KRYS_NODISCARD bool EndOfStream() const noexcept override
    {
      return _position >= _buffer.size();
    }
  };

  /// @brief A stream that writes to a memory buffer.
  class MemoryStreamWriter : public IStreamWriter
  {
    List<byte> &_buffer;
    size_t _position = 0;
    bool _isOpen = false;
    WriteFlags _flags;

  public:
    constexpr static WriteFlags DefaultWriteFlags = WriteFlags::None;

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
    KRYS_NODISCARD bool IsOpen() const noexcept override
    {
      return _isOpen;
    }

    /// @brief Opens the stream. If the stream is already open, this function does nothing.
    bool Open() override
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
        _position = _buffer.size();
      }

      return _isOpen;
    }

    /// @brief Closes the stream. If the stream is already closed, this function does nothing.
    void Close() noexcept override
    {
      _isOpen = false;
      _position = 0uz;
    }

    /// @brief Writes bytes to the stream from the source buffer.
    bool Write(Span<const byte> source) noexcept override
    {
      if (!_isOpen || source.empty())
      {
        return false;
      }

      try
      {
        // ensure the buffer can accommodate the new data
        if (_position + source.size() > _buffer.size())
        {
          _buffer.resize(_position + source.size());
        }

        std::memcpy(&_buffer[_position], source.data(), source.size());
        _position += source.size();

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

      _position = static_cast<size_t>(newPosition);
      return true;
    }

    /// @brief Gets the total size of the stream in bytes.
    KRYS_NODISCARD size_t Size() const noexcept override
    {
      return _buffer.size();
    }

    /// @brief Gets the current position in the stream.
    KRYS_NODISCARD size_t Position() noexcept override
    {
      return _position;
    }

    void Flush() noexcept override
    {
      // No-op for memory stream
    }
  };
}