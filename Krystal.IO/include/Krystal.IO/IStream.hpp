#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::IO
{
  /// @brief Interface for reading from a stream.
  class IStreamReader
  {
  public:
    NO_COPY_MOVE(IStreamReader)

    IStreamReader() = default;
    virtual ~IStreamReader() = default;

    /// @brief Checks if the stream is open.
    NO_DISCARD virtual bool IsOpen() const noexcept = 0;

    /// @brief Opens the stream. If the stream is already open, this function does nothing.
    virtual bool Open() noexcept = 0;

    /// @brief Closes the stream. If the stream is already closed, this function does nothing.
    virtual void Close() noexcept = 0;

    /// @brief Reads 'count' bytes from the stream.
    /// @return The number of bytes actually read from the stream.
    virtual uint64 Read(byte *dst, uint64 count) noexcept = 0;

    /// @brief Seeks to a specific position in the stream.
    /// @return True if the seek operation was successful, false otherwise.
    virtual bool Seek(int64 offset, SeekOrigin origin = SeekOrigin::Current) noexcept = 0;

    /// @brief Gets the total size of the stream in bytes, or 0 if the size is unknown.
    NO_DISCARD virtual uint64 Size() noexcept = 0;

    /// @brief Gets the current position in the stream.
    NO_DISCARD virtual uint64 Position() noexcept = 0;

    /// @brief Checks if the end of the stream has been reached.
    /// @return True if the end of the stream has been reached, false otherwise.
    NO_DISCARD virtual bool EndOfStream() const noexcept = 0;
  };

  /// @brief Interface for writing to a stream.
  class IStreamWriter
  {
  public:
    NO_COPY_MOVE(IStreamWriter)

    IStreamWriter() = default;
    virtual ~IStreamWriter() = default;

    /// @brief Checks if the stream is open.
    NO_DISCARD virtual bool IsOpen() const noexcept = 0;

    /// @brief Opens the stream writer for writing. If the stream is already open, this function does nothing.
    virtual bool Open() noexcept = 0;

    /// @brief Closes the stream writer. If the stream is already closed, this function does nothing.
    virtual void Close() noexcept = 0;

    /// @brief Writes bytes to the stream.
    /// @return True if the write operation was successful, false otherwise.
    virtual bool Write(const byte *src, uint64 count) noexcept = 0;

    /// @brief Seeks to a specific position in the stream.
    /// @return True if the seek operation was successful, false otherwise.
    virtual bool Seek(int64 offset, SeekOrigin origin = SeekOrigin::Current) noexcept = 0;

    /// @brief Gets the size of the stream in bytes.
    NO_DISCARD virtual uint64 Size() noexcept = 0;

    /// @brief Gets the current position in the stream.
    NO_DISCARD virtual uint64 Position() noexcept = 0;

    /// @brief Checks if the end of the stream has been reached.
    /// @return True if the end of the stream has been reached, false otherwise.
    NO_DISCARD virtual bool EndOfStream() const noexcept = 0;
  };
}