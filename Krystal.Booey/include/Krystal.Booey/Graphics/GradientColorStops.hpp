#pragma once

#include "Krystal.Booey/Graphics/GradientColorStop.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <algorithm>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A gradient's colour stops, sorted by offset on demand.
  class GradientColorStops
  {
  public:
    using StopVector = SmallList<GradientColorStop, 2>;

    /// @brief Stops already sorted by offset.
    struct Sorted
    {
      StopVector Stops;
    };

  private:
    StopVector _stops;
    bool _isSorted;

  public:
    GradientColorStops() noexcept : _isSorted {true}
    {
    }

    explicit GradientColorStops(StopVector stops) noexcept : _stops {krys::move(stops)}, _isSorted {false}
    {
    }

    GradientColorStops(Sorted sortedStops) noexcept : _stops {krys::move(sortedStops.Stops)}, _isSorted {true}
    {
      krys_debug_assert(ValidateIsSorted());
    }

    void AddColorStop(GradientColorStop stop) noexcept
    {
      if (!_stops.empty() && _stops.back().Offset > stop.Offset)
      {
        _isSorted = false;
      }
      _stops.push_back(krys::move(stop));
    }

    void Sort() noexcept
    {
      if (_isSorted)
      {
        return;
      }

      std::ranges::stable_sort(_stops, {}, &GradientColorStop::Offset);
      _isSorted = true;
    }

    KRYS_NODISCARD const GradientColorStops &GetSorted() const noexcept
    {
      const_cast<GradientColorStops *>(this)->Sort();
      return *this;
    }

    KRYS_NODISCARD size_t size() const noexcept
    {
      return _stops.size();
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _stops.empty();
    }

    KRYS_NODISCARD StopVector::const_iterator begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _stops.begin();
    }
    KRYS_NODISCARD StopVector::const_iterator end() const noexcept KRYS_LIFETIME_BOUND
    {
      return _stops.end();
    }

    template <typename TMapFunction>
    KRYS_NODISCARD GradientColorStops MapColors(TMapFunction &&mapFunction) const noexcept
    {
      StopVector stops;
      stops.reserve(_stops.size());
      for (const auto &stop : _stops)
      {
        stops.push_back({stop.Offset, mapFunction(stop.Color)});
      }
      return {krys::move(stops), _isSorted};
    }

    KRYS_NODISCARD const StopVector &Stops() const noexcept KRYS_LIFETIME_BOUND
    {
      return _stops;
    }

  private:
    GradientColorStops(StopVector stops, bool isSorted) noexcept
        : _stops {krys::move(stops)}, _isSorted {isSorted}
    {
    }

    KRYS_NODISCARD bool ValidateIsSorted() const noexcept
    {
      return std::ranges::is_sorted(_stops, {}, &GradientColorStop::Offset);
    }
  };
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const GradientColorStops &stops) noexcept;
}
