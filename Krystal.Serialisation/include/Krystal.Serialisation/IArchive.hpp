#pragma once

#include "Krystal.IO/IStream.hpp"

namespace Krys::Serialisation
{
  /// @brief Restricts types to only those that the archive implementations support.
  template <typename T>
  concept SupportedArchivePrimitive = Arithmetic<T> || SameType<T, byte> || SameType<T, string>;

  template <typename Derived>
  class IArchiveWriter
  {
  protected:
    IO::IStreamWriter &_stream;

  public:
    IArchiveWriter(IO::IStreamWriter &stream) noexcept : _stream(stream)
    {
    }

    template <SupportedArchivePrimitive T>
    Derived &operator()(T &value) noexcept
    {
      auto &derived = static_cast<Derived &>(*this);
      derived(value);
      return static_cast<Derived &>(*this);
    }
  };

  template <typename Derived>
  class IArchiveReader
  {
  protected:
    IO::IStreamReader &_stream;

  public:
    IArchiveReader(IO::IStreamReader &stream) noexcept : _stream(stream)
    {
    }

    template <SupportedArchivePrimitive T>
    Derived &operator()(T &value) noexcept
    {
      auto &derived = static_cast<Derived &>(*this);
      derived(value);
      return static_cast<Derived &>(*this);
    }
  };
}