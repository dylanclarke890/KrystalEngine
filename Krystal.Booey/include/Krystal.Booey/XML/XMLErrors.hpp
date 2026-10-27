#pragma once

#include "Krystal.Booey/Types/SourceLocation.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::xml
{
  /// @brief The errors an XML parser found, and the block that lists them at the top of the document.
  class XMLErrors
  {
  public:
    enum class Type : uint8
    {
      Warning,
      NonFatal,
      Fatal
    };

  private:
    WeakPtr<dom::Document> _document;
    int32 _errorCount {0};
    Maybe<SourceLocation> _lastErrorPosition;
    StringBuilder _errorMessages;

  public:
    explicit XMLErrors(dom::Document &) noexcept;

    void HandleError(Type, const char *message, SourceLocation) noexcept;

    void InsertErrorMessageBlock() noexcept;

  private:
    void AppendErrorMessage(utf8_stringview typeString, SourceLocation, const char *message) noexcept;
  };
}
