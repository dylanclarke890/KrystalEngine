#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <fstream>

namespace Krys::IO
{
  /// @brief A stream reader that reads from a native file on the filesystem.
  class NativeFileReader : public IStreamReader
  {
    Path _path;
    std::ifstream _stream;
    uint64 _size;
    ReadFlags _flags;

  public:
    static constexpr ReadFlags DefaultReadFlags = ReadFlags::None;

    NO_COPY_MOVE(NativeFileReader)

    explicit NativeFileReader(const Path &path, ReadFlags flags = DefaultReadFlags);

    explicit NativeFileReader(Path &&path, ReadFlags flags = DefaultReadFlags);

    ~NativeFileReader() noexcept override;

    /// @brief Checks if the stream is open.
    NO_DISCARD bool IsOpen() const noexcept override;

    /// @brief Opens the stream. If the stream is already open, this function does nothing.
    bool Open() override;

    /// @brief Closes the stream. If the stream is already closed, this function does nothing.
    void Close() noexcept override;

    /// @brief Reads 'count' bytes from the stream.
    /// @param dst The destination buffer to read into.
    /// @param count The number of bytes to read.
    /// @return The number of bytes actually read from the stream.
    uint64 Read(byte *dst, uint64 count) noexcept override;

    /// @brief Seeks to a specific position in the stream.
    /// @param offset The offset to seek to, relative to the origin.
    /// @param origin The reference point used to obtain the new position.
    bool Seek(int64 offset, SeekOrigin origin) noexcept override;

    /// @brief Peeks at the next byte in the stream without advancing the position.
    /// @param next A reference to a byte where the peeked value will be stored.
    /// @returns True if a byte was successfully peeked, false if the end of the stream was reached.
    NO_DISCARD bool Peek(byte &next) noexcept override;

    /// @brief Gets the total size of the stream in bytes, or 0 if the size is unknown.
    NO_DISCARD uint64 Size() const noexcept override;

    /// @brief Gets the current position in the stream.
    NO_DISCARD uint64 Position() noexcept override;

    /// @brief Checks if the end of the stream has been reached.
    NO_DISCARD bool EndOfStream() const noexcept override;
  };

  /// @brief A stream writer that writes to a native file on the filesystem.
  class NativeFileWriter : public IStreamWriter
  {
    Path _path;
    std::ofstream _stream;
    uint64 _size;
    WriteFlags _flags;

  public:
    static constexpr WriteFlags DefaultWriteFlags = WriteFlags::Create | WriteFlags::OpenAtEnd;

    NO_COPY_MOVE(NativeFileWriter)

    explicit NativeFileWriter(const Path &path, WriteFlags flags = DefaultWriteFlags);

    explicit NativeFileWriter(Path &&path, WriteFlags flags = DefaultWriteFlags);

    ~NativeFileWriter() noexcept override;

    /// @brief Checks if the stream is open.
    NO_DISCARD bool IsOpen() const noexcept override;

    /// @brief Opens the stream writer for writing. If the stream is already open, this function does nothing.
    bool Open() override;

    /// @brief Closes the stream writer. If the stream is already closed, this function does nothing.
    void Close() noexcept override;

    /// @brief Writes bytes to the stream.
    /// @param src The source buffer to write from.
    /// @param count The number of bytes to write.
    /// @returns True if the write operation was successful, false otherwise.
    bool Write(const byte *src, uint64 count) noexcept override;

    /// @brief Seeks to a specific position in the stream.
    /// @param offset The offset to seek to, relative to the origin.
    /// @param origin The reference point used to obtain the new position.
    /// @returns True if the seek operation was successful, false otherwise.
    bool Seek(int64 offset, SeekOrigin origin) noexcept override;

    /// @brief Gets the total size of the stream in bytes, or 0 if the size is unknown.
    NO_DISCARD uint64 Size() const noexcept override;

    /// @brief Gets the current position in the stream.
    NO_DISCARD uint64 Position() noexcept override;

    /// @brief Flushes the stream, ensuring all buffered data is written to the file.
    void Flush() noexcept override;
  };
}