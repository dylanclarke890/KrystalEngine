#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css::mq
{
  KRYS_NODISCARD static Maybe<MediaEnvironment> &CurrentMediaEnvironment() noexcept
  {
    static MainThreadNeverDestroyed<Maybe<MediaEnvironment>> environment;
    return environment;
  }

  const MediaEnvironment *MediaEnvironmentFor(const dom::Document &) noexcept
  {
    auto &environment = CurrentMediaEnvironment();
    return environment ? &*environment : nullptr;
  }

  void SetMediaEnvironment(Maybe<MediaEnvironment> environment) noexcept
  {
    CurrentMediaEnvironment() = krys::move(environment);
  }
}
