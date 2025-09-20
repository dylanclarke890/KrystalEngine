#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Serialisation/IArchive.hpp"
#include "Krystal.Serialisation/Types/Builtins.hpp"

namespace Krys::Serialisation
{
  template <typename Archive>
  class ISerialiser
  {
    static_assert(DerivedFrom<Archive, IArchiveWriter<Archive>>,
                  "'Archive' must derive from 'IArchiveWriter<T>'");

    NO_COPY_MOVE(ISerialiser)

  protected:
    Archive _archive;

  public:
    explicit ISerialiser(IO::IStreamWriter &stream) noexcept : _archive(stream)
    {
    }

    ~ISerialiser() noexcept = default;

    template <typename T>
    void Serialise(T &value) noexcept
    {
      Save(_archive, value);
    }

    Archive &GetArchive() const noexcept
    {
      return _archive;
    }
  };

  template <typename Archive>
  class IDeserialiser
  {
    static_assert(DerivedFrom<Archive, IArchiveReader<Archive>>,
                  "'Archive' must derive from 'IArchiveReader<T>'");

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
      Load(_archive, value);
    }

    Archive &GetArchive() const noexcept
    {
      return _archive;
    }
  };
}