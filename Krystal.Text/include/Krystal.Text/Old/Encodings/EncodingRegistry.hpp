#pragma once

#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Text/Old/Encodings/Encoding.hpp"
#include "Krystal.Text/Old/Unicode.hpp"
#include <typeindex>

namespace Krys
{
  struct EncodingRegistry : NonCopyMovable<EncodingRegistry>
  {
  public:
    using LabelToEncodingMap = Map<utf8_string, Encoding *>;

  private:
    LabelToEncodingMap _labelToEncodingMap;
    Map<std::type_index, UniquePtr<Encoding>> _encodings;

  public:
    /// @brief Registers a new encoding type in the registry. Previously registered encodings of the same type
    /// will be overwritten.
    template <DerivedFrom<Encoding> T>
    void Register() noexcept
    {
      std::type_index key = typeid(T);
      _encodings[key] = CreateUnique<T>();

      auto *encoding = _encodings.at(key).get();
      const EncodingInfo &info = encoding->GetInfo();

      _labelToEncodingMap[info.Name] = encoding;
      for (const utf8_string &alias : info.Aliases)
      {
        _labelToEncodingMap[alias] = encoding;
      }
    }

    KRYS_NODISCARD Encoding *GetByLabel(const utf8_string &label) const noexcept
    {
      if (label.empty())
      {
        return nullptr;
      }

      if (auto it = _labelToEncodingMap.find(label); it != _labelToEncodingMap.end())
      {
        return it->second;
      }

      if (auto it = _labelToEncodingMap.find(ASCII::ToLower(label)); it != _labelToEncodingMap.end())
      {
        return it->second;
      }

      return nullptr;
    }

    KRYS_NODISCARD utf8_string LabelToName(const utf8_string &label) const noexcept
    {
      auto encoding = GetByLabel(utf8_string(label));
      if (encoding != nullptr)
      {
        return encoding->GetInfo().Name;
      }

      return u8"";
    }
  };
}