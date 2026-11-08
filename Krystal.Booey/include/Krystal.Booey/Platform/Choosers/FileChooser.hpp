#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <utility>

namespace krys::boo
{
  /// @brief What a file chooser offers the user, from its file input's attributes.
  struct FileChooserSettings
  {
    /// @brief Whether more than one file can be chosen (`multiple`).
    bool AllowsMultipleFiles {false};
    /// @brief The MIME types `accept` names, lowercased, such as `image/png` or `image/*`.
    List<utf8_string> AcceptMIMETypes;
    /// @brief The extensions `accept` names, lowercased with their dot, such as `.pdf`.
    List<utf8_string> AcceptFileExtensions;
    /// @brief The paths of the input's files.
    List<utf8_string> SelectedFiles;
  };

  /// @brief Who a file chooser tells about the user's choice: a file input.
  class FileChooserClient
  {
  public:
    virtual ~FileChooserClient() noexcept = default;

    /// @brief The user chose the files at @p paths, which aren't empty.
    virtual void FilesChosen(const List<utf8_string> &paths) noexcept = 0;
    /// @brief The user closed the chooser without choosing.
    virtual void FileChoosingCancelled() noexcept = 0;
  };

  /// @brief A request for the host to let the user choose files (`ChromeClient::RunOpenPanel`), answered once
  /// with `ChooseFiles` or `Cancel`, whenever the host is ready. An answer after its input went away, or a
  /// second one, does nothing.
  class FileChooser final : public RefCounted<FileChooser>
  {
    KRYS_NON_COPY_MOVABLE(FileChooser);

  private:
    FileChooserClient *_client;
    FileChooserSettings _settings;

  public:
    FileChooser(FileChooserClient &client, FileChooserSettings settings) noexcept
        : _client(&client), _settings(krys::move(settings))
    {
    }

    KRYS_NODISCARD static Ref<FileChooser> Create(FileChooserClient &client,
                                                  FileChooserSettings settings) noexcept
    {
      return CreateRef<FileChooser>(client, krys::move(settings));
    }

    KRYS_NODISCARD const FileChooserSettings &Settings() const noexcept
    {
      return _settings;
    }

    /// @brief Answers with the files at @p paths; none is a cancellation.
    void ChooseFiles(const List<utf8_string> &paths) noexcept
    {
      if (paths.empty())
      {
        Cancel();
        return;
      }
      if (auto *client = std::exchange(_client, nullptr))
      {
        client->FilesChosen(paths);
      }
    }

    /// @brief Answers that the user chose nothing.
    void Cancel() noexcept
    {
      if (auto *client = std::exchange(_client, nullptr))
      {
        client->FileChoosingCancelled();
      }
    }

    /// @brief Forgets the client, which is going away, so a later answer does nothing.
    void Invalidate() noexcept
    {
      _client = nullptr;
    }
  };
}
