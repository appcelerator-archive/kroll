/**
 * @mainpage Kroll API Documentation
 *
 * This is the API reference for
 * <a href="http://github.com/jhaynie/kroll">Kroll</a>, a native micro-kernel that is
 * currently used for <a href="http://titaniumapp.com">Titanium</a>. To get started, you'll probably want to take a look at our
 * <a href="http://doc.titaniumapp.com/extending:start">Extending Guide Wiki</a>.
 * <br/><br/>
 *
 * Kroll makes use of the excellent general-purpose C++ Library
 * <a href="http://pocoproject.org">Poco</a>.
 * <br/>
 *
 * Wherever possible, Kroll uses shared pointers to handle garbage cleanup. The semantics
 * of a shared pointer are fairly simple, i.e..:
 * \code
 * void function() {
 *   Poco::SharedPtr<MyObject> myObject = new MyObject(arg1, arg2);
 *   //.. do things with myObject
 *   // on function exit, myObject is freed unless a copy is passed
 * }
 * \endcode
 *<br/>
 *
 * We have setup synonyms for shared pointers of the most common types, i.e:
 * - kroll::SharedValue -> SharedPtr<\link kroll::Value\endlink>
 * - kroll::SharedKObject -> SharedPtr<\link kroll::KObject\endlink>
 * - kroll::SharedKMethod -> SharedPtr<\link kroll::KMethod\endlink>
 * - etc..
 * <br/><br/>
 *
 * Important classes for use by downstream developers:
 * - kroll::Module
 * - kroll::Host
 * - kroll::Value
 * - kroll::KObject
 * - kroll::StaticBoundObject
 * - kroll::KMethod
 * - kroll::KList
 */
