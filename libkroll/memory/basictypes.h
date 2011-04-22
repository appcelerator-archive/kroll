// Excerpt from Chromium's base/memory/basictypes.h
// Only what we need to compile the other files we extracted.

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
    void operator=(const TypeName&)

