#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::HTML
{
  class DocumentType final : public Node
  {
  private:
    utf8_string _name;
    utf8_string _publicId;
    utf8_string _systemId;

  public:
    DocumentType(Document &document, const utf8_string &name, const utf8_string &publicId,
                 const utf8_string &systemId) noexcept;

    KRYS_NODISCARD const utf8_string &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const utf8_string &PublicId() const noexcept
    {
      return _publicId;
    }

    KRYS_NODISCARD const utf8_string &SystemId() const noexcept
    {
      return _systemId;
    }

    KRYS_NODISCARD utf8_string NodeName() const noexcept override;
  };
}