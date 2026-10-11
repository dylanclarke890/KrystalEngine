#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <initializer_list>
#include <tuple>

namespace krys::boo::css
{
  enum class UnitType : uint8;

  class SymbolTable
  {
  public:
    struct Value
    {
      UnitType unit;
      double value;
    };

  private:
    HashMap<ValueId, Value> _table;

  public:
    SymbolTable() noexcept = default;

    SymbolTable(std::initializer_list<std::tuple<ValueId, UnitType, double>> initializer) noexcept
    {
      for (const auto &[id, unit, value] : initializer)
      {
        _table.emplace(id, Value(unit, value));
      }
    }

    KRYS_NODISCARD Maybe<Value> Get(ValueId id) const noexcept
    {
      if (const auto it = _table.find(id); it != _table.end())
      {
        return it->second;
      }

      return null;
    }

    KRYS_NODISCARD bool Contains(ValueId id) const noexcept
    {
      return _table.find(id) != _table.end();
    }
  };
}
