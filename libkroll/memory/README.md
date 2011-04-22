Memory Management Helpers
=========================
A collection of memory management helper classes
derived from Chromium base/memory.

scoped_ptr
----------
Scopers help you manage ownership of a pointer, helping you easily manage the
a pointer within a scope, and automatically destroying the pointer at the
end of a scope.  There are two main classes you will use, which correspond
to the operators new/delete and new[]/delete[].

Should be used when ownership is linked to a single object or function.

Example:
``` C++
// Pointer is released once foo goes out of scope.
scoped_ptr<Foo> foo(new Foo("wee"));
```

Credit
------
Thanks to the Chromium project!

http://www.chromium.org/

