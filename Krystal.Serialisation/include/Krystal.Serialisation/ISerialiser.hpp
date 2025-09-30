#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Serialisation/Concepts.hpp"
#include "Krystal.Serialisation/Dispatch.hpp"

namespace Krys::Serialisation
{
  template <typename Archive>
  class ISerialiser
  {
    NO_COPY_MOVE(ISerialiser)

  protected:
    Archive _archive;

  public:
    explicit ISerialiser(IO::IStreamWriter &stream) noexcept : _archive(stream)
    {
    }

    ~ISerialiser() noexcept = default;

    template <typename T>
    void Serialise(const T &value) noexcept
    {
      DispatchSave(_archive, value);
    }

    Archive &GetArchive() noexcept
    {
      return _archive;
    }

    const Archive &GetArchive() const noexcept
    {
      return _archive;
    }
  };

  template <typename Archive>
  class IDeserialiser
  {
    NO_COPY_MOVE(IDeserialiser)

  protected:
    Archive _archive;

  public:
    explicit IDeserialiser(IO::IStreamReader &stream) noexcept : _archive(stream)
    {
    }

    ~IDeserialiser() noexcept = default;

    template <typename T>
    void Deserialise(T &value) noexcept
    {
      DispatchLoad(_archive, value);
    }

    Archive &GetArchive() noexcept
    {
      return _archive;
    }

    const Archive &GetArchive() const noexcept
    {
      return _archive;
    }
  };
}