#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"
#include "Krystal.Core/Text/ICodec.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::text
{
  class EncodingRegistry
  {
    KRYS_NON_COPYABLE(EncodingRegistry);

  private:
    SmallList<UniquePtr<ICodec>> _encodings;
    HashMap<utf8_stringview, ICodec *> _lookupMap;

  public:
    EncodingRegistry() noexcept = default;

    EncodingRegistry(EncodingRegistry &&) noexcept = default;

    template <Encoding TEncoding>
    void Register() noexcept
    {
      _encodings.push_back(CreateUnique<Codec<TEncoding>>());
      auto *encoding = _encodings.back().get();

      Span<const utf8_stringview> aliases = TEncoding::Aliases;
      for (const auto &alias : aliases)
      {
        _lookupMap[alias] = encoding;
      }
    }

    KRYS_NODISCARD ICodec *Find(utf8_stringview name) const noexcept
    {
      auto it = _lookupMap.find(name);
      if (it != _lookupMap.end())
      {
        return it->second;
      }
     
      return nullptr;
    }
  };
}