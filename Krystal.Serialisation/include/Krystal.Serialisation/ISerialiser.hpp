#pragma once

#include "Krystal.IO/IStream.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Serialisation/Concepts.hpp"

namespace Krys::Serialisation
{
  /// @brief Hook for serialisation of custom objects/types.
  template <typename Archive, typename T>
  void Save(Archive &, const T &) noexcept
  {
    static_assert(false, "You must provide a custom 'Save' function for type.");
  }

  /// @brief Hook for deserialisation of custom objects/types.
  template <typename Archive, typename T>
  void Load(Archive &, T &) noexcept
  {
    static_assert(false, "You must provide a custom 'Load' function for type.");
  }

  /// @brief Serialisation of built-in types.
  template <typename Archive, ArchiveBuiltin T>
  void Save(Archive &archive, const T &value) noexcept
  {
    archive(value);
  }

  /// @brief Deserialisation of built-in types.
  template <typename Archive, ArchiveBuiltin T>
  void Load(Archive &archive, T &value) noexcept
  {
    archive(value);
  }

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
      Save(_archive, value);
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
      Load(_archive, value);
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