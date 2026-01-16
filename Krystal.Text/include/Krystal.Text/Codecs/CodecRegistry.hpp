#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Text/ASCIILiteral.hpp"
#include "Krystal.Text/Codecs/BasicCodec.hpp"
#include "Krystal.Text/Codecs/ICodec.hpp"
#include "Krystal.Text/EncodingName.hpp"
#include <typeindex>

namespace Krys::Text
{
  class CodecRegistry
  {
  public:
    using LabelToCodecMap = Map<ASCIILiteral, ICodec *>;

  private:
    LabelToCodecMap _labelToCodecMap;
    Map<std::type_index, UniquePtr<ICodec>> _codecs;

  public:
    template <typename T>
    requires(IsSpecializationOf<T, BasicCodec>)
    void Register() noexcept
    {
      constexpr ASCIILiteral name = GetEncodingName<basic_codec_encoding_t<T>>();
      assert(!name.IsNull() && !name.IsEmpty() && "Missing encoding name.");

      std::type_index key = typeid(T);
      _codecs[key] = CreateUnique<T>();

      auto *codec = _codecs.at(key).get();
      _labelToCodecMap[name] = codec;

      constexpr auto aliases = GetEncodingAliases<basic_codec_encoding_t<T>>();
      for (auto alias : aliases)
      {
        _labelToCodecMap[alias] = codec;
      }
    }

    KRYS_NODISCARD ICodec *Find(ASCIILiteral label) const noexcept
    {
      auto it = _labelToCodecMap.find(label);
      if (it != _labelToCodecMap.end())
      {
        return it->second;
      }

      return nullptr;
    }

    KRYS_NODISCARD ASCIILiteral LabelToName(ASCIILiteral label) const noexcept
    {
      auto *codec = Find(label);
      if (codec != nullptr)
      {
        return codec->Name();
      }

      return ""_s;
    }
  };
}