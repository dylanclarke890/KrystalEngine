#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <initializer_list>
#include <tuple>

namespace krys::boo::css
{
  enum class UnitType : uint8;

  /// @brief Values for the symbols (e.g. the channel keywords of relative colours) a calc() may reference.
  class SymbolTable
  {
  public:
    struct Entry
    {
      UnitType Unit;
      double Value;
    };

  private:
    HashMap<ValueId, Entry> _table;

  public:
    SymbolTable() noexcept = default;

    SymbolTable(std::initializer_list<std::tuple<ValueId, UnitType, double>> initializer)
    {
      for (const auto &[id, unit, value] : initializer)
      {
        _table.emplace(id, Entry {unit, value});
      }
    }

    KRYS_NODISCARD Maybe<Entry> Get(ValueId id) const noexcept
    {
      if (const auto it = _table.find(id); it != _table.end())
      {
        return it->second;
      }

      return null;
    }

    KRYS_NODISCARD bool Contains(ValueId id) const noexcept
    {
      return _table.contains(id);
    }
  };
}
