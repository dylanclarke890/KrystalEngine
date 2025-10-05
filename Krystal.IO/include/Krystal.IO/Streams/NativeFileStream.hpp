#pragma once

#include "Krystal.IO/Common.hpp"
#include "Krystal.IO/IStream.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include <fstream>

namespace Krys::IO
{
  class NativeFileReader : public IStreamReader
  {
    Path _path;
    std::ifstream _stream;

  public:
    NO_COPY_MOVE(NativeFileReader)

    explicit NativeFileReader(const Path &path);

    explicit NativeFileReader(Path &&path);

    ~NativeFileReader() noexcept override;

    NO_DISCARD bool IsOpen() const noexcept override;

    bool Open() noexcept override;

    void Close() noexcept override;

    uint64 Read(byte *dst, uint64 count) noexcept override;

    bool Seek(int64 offset, SeekOrigin origin) noexcept override;

    NO_DISCARD bool Peek(byte &next) noexcept override;

    NO_DISCARD uint64 Size() noexcept override;

    NO_DISCARD uint64 Position() noexcept override;

    /// @brief Checks if the end of the stream has been reached.
    /// @return True if the end of the stream has been reached, false otherwise.
    NO_DISCARD bool EndOfStream() const noexcept override;
  };

  class NativeFileWriter : public IStreamWriter
  {
    Path _path;
    std::ofstream _stream;

  public:
    NO_COPY_MOVE(NativeFileWriter)

    explicit NativeFileWriter(const Path &path);

    explicit NativeFileWriter(Path &&path);

    ~NativeFileWriter() noexcept override;

    NO_DISCARD bool IsOpen() const noexcept override;

    bool Open() noexcept override;

    void Close() noexcept override;

    bool Write(const byte *src, uint64 count) noexcept override;

    bool Seek(int64 offset, SeekOrigin origin) noexcept override;

    NO_DISCARD uint64 Size() noexcept override;

    NO_DISCARD uint64 Position() noexcept override;

    void Flush() noexcept override;

    /// @brief Checks if the end of the stream has been reached.
    /// @return True if the end of the stream has been reached, false otherwise.
    NO_DISCARD bool EndOfStream() const noexcept override;
  };
}