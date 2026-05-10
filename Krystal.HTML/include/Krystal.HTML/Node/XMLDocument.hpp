#pragma once

#include "Krystal.HTML/Node/Document.hpp"

namespace Krys::HTML
{
  class XMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(XMLDocument);

    // TODO(impl): set the type of the document to 'xml'
  };
}

// TODO(impl): add type cast traits for XMLDocument