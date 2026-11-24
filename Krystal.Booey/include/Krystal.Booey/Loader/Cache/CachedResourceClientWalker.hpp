#pragma once

#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"

namespace krys::boo
{
  /// @brief Goes through a resource's clients as `T`s (`Next` until it returns null), skipping any removed in
  /// the meantime, so the clients can be told of something that may remove some of them.
  template <typename T>
  class CachedResourceClientWalker
  {
  private:
    CachedResourceHandle<CachedResource> _resource;
    List<CachedResourceClient *> _clientVector;
    size_t _index {0};

  public:
    /// @brief Takes the clients @p resource has now, and keeps @p resource alive.
    explicit CachedResourceClientWalker(const CachedResource &resource) noexcept
        : _resource(const_cast<CachedResource *>(&resource))
    {
      _clientVector.reserve(resource._clients.size());
      for (const auto &[client, count] : resource._clients)
      {
        _clientVector.push_back(client);
      }
    }

    /// @brief The next client still added, or null at the end.
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
