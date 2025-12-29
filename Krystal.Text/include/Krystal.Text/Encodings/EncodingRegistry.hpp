#pragma once

#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  struct EncodingRegistry : NonCopyMovable<EncodingRegistry>
  {
  public:
    using LabelToEncodingMap = Map<utf8_string, Encoding *>;

  private:
    LabelToEncodingMap _labelToEncodingMap;

  public:
    template <typename T>
    void Register() noexcept
    {
      static T encodingInstance;
      const EncodingInfo &info = encodingInstance.GetInfo();

      _labelToEncodingMap[info.Name] = &encodingInstance;

      for (const utf8_string &alias : info.Aliases)
      {
        _labelToEncodingMap[alias] = &encodingInstance;
      }
    }

    KRYS_NODISCARD const Encoding *GetByLabel(const utf8_string &label) const noexcept
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