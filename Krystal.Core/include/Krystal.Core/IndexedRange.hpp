#pragma once

#include <utility>

namespace Krys
{
  template <typename T>
  class IndexedRange
  {
  private:
    const T& _container;

  public:
    struct Iterator
    {
      size_t index;
      typename T::const_iterator iter;

      bool operator!=(const Iterator& other) const
      {
        return iter != other.iter;
      }

      void operator++()
      {
        ++index;
        ++iter;
      }

      auto operator*() const
      {
        return std::make_pair(index, *iter);
      }
    };

    IndexedRange(const T& container) : _container(container)
    {
    }

    Iterator begin() const
    {
      return { 0, _container.begin() };
    }

    Iterator end() const
    {
      return { _container.size(), _container.end() };
    }
  };
}
