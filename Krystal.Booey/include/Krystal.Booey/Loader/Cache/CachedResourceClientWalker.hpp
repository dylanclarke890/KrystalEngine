#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"

namespace krys::boo
{
  // Call this "walker" instead of iterator so people won't expect Qt or STL-style iterator interface.
  // Just keep calling next() on this. It's safe from deletions of items.
  template <typename T>
  class CachedResourceClientWalker
  {
  private:
    CachedResourceHandle<CachedResource> _resource;
    List<CachedResourceClient *> _clientVector;
    size_t _index {0};

  public:
    explicit CachedResourceClientWalker(const CachedResource &resource) noexcept
        : _resource(const_cast<CachedResource *>(&resource))
    {
      _clientVector.reserve(resource._clients.size());
      for (const auto &[client, count] : resource._clients)
      {
        _clientVector.push_back(client);
      }
    }

    T *Next() noexcept
    {
      size_t size = _clientVector.size();
      while (_index < size)
      {
        auto *next = _clientVector[_index++];
        if (next && _resource->_clients.contains(next))
        {
          krys_assert(T::ExpectedType() == CachedResourceClient::ExpectedType()
                      || next->ResourceClientType() == T::ExpectedType());
          return static_cast<T *>(next);
        }
      }
      return nullptr;
    }
  };
}
