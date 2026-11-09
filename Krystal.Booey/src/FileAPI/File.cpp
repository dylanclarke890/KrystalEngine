/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/FileAPI/File.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include <chrono>
#include <filesystem>
#include <system_error>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD int64 Now() noexcept
    {
      using namespace std::chrono;
      return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
  }

  Ref<File> File::Create(utf8_stringview path, utf8_stringview nameOverride) noexcept
  {
    utf8_string name;
    utf8_string type;
    ComputeNameAndContentType(path, nameOverride, name, type);

    auto data = BlobData::Create(type);
    data->AppendFile(path);
    return AdoptRef(*new File(krys::move(data), type, utf8_string(path), krys::move(name)));
  }

  Ref<File> File::Create(List<BlobPartVariant> &&blobPartVariants, utf8_stringview filename,
                         const PropertyBag &propertyBag) noexcept
  {
    return AdoptRef(*new File(krys::move(blobPartVariants), filename, propertyBag));
  }

  Ref<File> File::Create(const Blob &blob, utf8_stringview name) noexcept
  {
    return AdoptRef(*new File(blob, name));
  }

  Ref<File> File::Create(const File &existingFile, Maybe<utf8_stringview> name) noexcept
  {
    return AdoptRef(*new File(existingFile, name));
  }

  File::File(Ref<BlobData> &&data, utf8_stringview type, utf8_string path, utf8_string name) noexcept
      : Blob(krys::move(data), type, std::nullopt, 0), _path(krys::move(path)), _name(krys::move(name))
  {
  }

  File::File(List<BlobPartVariant> &&blobPartVariants, utf8_stringview filename,
             const PropertyBag &propertyBag) noexcept
      : Blob(krys::move(blobPartVariants), propertyBag), _name(filename),
        _lastModifiedDateOverride(propertyBag.LastModified.value_or(Now()))
  {
  }

  File::File(const Blob &blob, utf8_stringview name) noexcept
      : Blob(ReferencingExistingBlob, blob), _name(name)
  {
    krys_debug_assert(!blob.IsFile());
  }

  File::File(const File &file, Maybe<utf8_stringview> name) noexcept
      : Blob(ReferencingExistingBlob, file), _path(file.Path()), _name(name ? *name : file.Name()),
        _lastModifiedDateOverride(file._lastModifiedDateOverride), _isDirectory(file.IsDirectory())
  {
  }

  int64 File::LastModified() const noexcept
  {
    if (_lastModifiedDateOverride)
    {
      return *_lastModifiedDateOverride;
    }

    // FIXME: This does sync-i/o on the main thread and also recalculates every time the method is called.
    // The i/o should be performed on a background thread, and the result should be cached along with an
    // asynchronous monitor for changes to the file.
    std::error_code error;
    auto modificationTime = std::filesystem::last_write_time(std::filesystem::path(_path), error);
    if (error)
    {
      return Now();
    }
    using namespace std::chrono;
    return duration_cast<milliseconds>(clock_cast<system_clock>(modificationTime).time_since_epoch()).count();
  }

  void File::ComputeNameAndContentType(utf8_stringview path, utf8_stringview nameOverride,
                                       utf8_string &effectiveName, utf8_string &effectiveContentType) noexcept
  {
    effectiveName =
      nameOverride.empty() ? std::filesystem::path(path).filename().u8string() : utf8_string(nameOverride);
    auto index = effectiveName.rfind(u8'.');
    if (index != utf8_string::npos)
    {
      effectiveContentType =
        MIMETypeRegistry::MIMETypeForExtension(utf8_stringview(effectiveName).substr(index + 1));
    }
  }

  utf8_string File::ContentTypeForFile(utf8_stringview path) noexcept
  {
    utf8_string name;
    utf8_string type;
    ComputeNameAndContentType(path, {}, name, type);

    return type;
  }

  bool File::IsDirectory() const noexcept
  {
    if (!_isDirectory)
    {
      std::error_code error;
      _isDirectory = !_path.empty() && std::filesystem::is_directory(std::filesystem::path(_path), error);
    }
    return *_isDirectory;
  }
}
