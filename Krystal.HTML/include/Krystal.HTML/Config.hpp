#pragma once

// TODO: change this to false when implementing the obsolete members is done.
#define KRYS_HTML_ENABLE_OBSOLETE_MEMBERS_DEFAULT 1

// TODO: Use this macro to conditionally compile the obsolete members of the HTML elements.
#ifndef KRYS_HTML_ENABLE_OBSOLETE_MEMBERS
  #define KRYS_HTML_ENABLE_OBSOLETE_MEMBERS KRYS_HTML_ENABLE_OBSOLETE_MEMBERS_DEFAULT
#endif
