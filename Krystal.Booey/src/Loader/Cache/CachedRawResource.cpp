#include "Krystal.Booey/Loader/Cache/CachedRawResource.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"

namespace krys::boo
{
  CachedRawResource::CachedRawResource(CachedResourceRequest &&request, Type type) noexcept
      : CachedResource(krys::move(request), type)
  {
    krys_debug_assert(type == Type::RawResource || type == Type::Icon);
  }

  void CachedRawResource::FinishLoading(const SharedBuffer *data) noexcept
  {
    _data = ShareRefPtr(const_cast<SharedBuffer *>(data));
    if (_data)
    {
      SetEncodedSize(static_cast<uint32>(_data->Size()));
    }
    CachedResource::FinishLoading(data);
  }
}
