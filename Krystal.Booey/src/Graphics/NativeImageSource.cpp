#include "Krystal.Booey/Graphics/NativeImageSource.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<NativeImageSource> NativeImageSource::Create(Ref<NativeImage> &&nativeImage) noexcept
  {
    return AdoptRef(*new NativeImageSource(krys::move(nativeImage)));
  }

  NativeImageSource::NativeImageSource(Ref<NativeImage> &&nativeImage) noexcept
      : _frame(krys::move(nativeImage))
  {
  }

  void NativeImageSource::Dump(TextStream &ts) const noexcept
  {
    ts.DumpProperty(u8"size", Size());
  }
}
