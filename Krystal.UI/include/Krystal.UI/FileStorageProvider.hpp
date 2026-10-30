#pragma once

#include "Krystal.Booey/Storage/StorageArea.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <filesystem>

namespace krys::ui
{
  /// @brief Keeps Booey pages' local storage in files: one per origin in a directory, read when the origin's
  /// storage is first used and written as it changes. The host makes Booey use it with
  /// `PlatformStrategies::SetStorageProvider`.
  // FIXME: Write the files off the main thread, and less often than every change.
  class FileStorageProvider final : public boo::StorageProvider
  {
  private:
    std::filesystem::path _directory;
    HashMap<boo::dom::DOMString, Ref<boo::StorageArea>> _areas;

  public:
    /// @param directory Where the files go; made if it doesn't exist.
    explicit FileStorageProvider(std::filesystem::path directory) noexcept;

    KRYS_NODISCARD Ref<boo::StorageArea> LocalStorageArea(boo::dom::DOMStringView origin) noexcept override;

    /// @brief The file @p origin's local storage is kept in.
    KRYS_NODISCARD std::filesystem::path PathForOrigin(boo::dom::DOMStringView origin) const noexcept;
  };
}
