//
// This code was originally taken from chromium's base/task.h and base/tuple.h, just trimmed to the part we need (callbacks)
//
// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _KR_CALLBACK_H_
#define _KR_CALLBACK_H_

namespace kroll
{
	template <class P>
	struct TupleTraits {
		typedef P ValueType;
		typedef P& RefType;
		typedef const P& ParamType;
	};

	template <class P>
	struct TupleTraits<P&> {
		typedef P ValueType;
		typedef P& RefType;
		typedef P& ParamType;
	};

	struct Tuple0 {
		typedef Tuple0 ValueTuple;
		typedef Tuple0 RefTuple;
	};

	template <class A>
	struct Tuple1 {
		public:
			typedef A TypeA;
			typedef Tuple1<typename TupleTraits<A>::ValueType> ValueTuple;
			typedef Tuple1<typename TupleTraits<A>::RefType> RefTuple;

			Tuple1() {}
			explicit Tuple1(typename TupleTraits<A>::ParamType a) : a(a) {}

			A a;
	};

	template <class A, class B>
	struct Tuple2 {
		public:
			typedef A TypeA;
			typedef B TypeB;
			typedef Tuple2<typename TupleTraits<A>::ValueType,
			               typename TupleTraits<B>::ValueType> ValueTuple;
			typedef Tuple2<typename TupleTraits<A>::RefType,
			               typename TupleTraits<B>::RefType> RefTuple;

			Tuple2() {}
			Tuple2(typename TupleTraits<A>::ParamType a,
			       typename TupleTraits<B>::ParamType b)
			    : a(a), b(b) {
			}

			A a;
			B b;
	};

	template <class A, class B, class C>
	struct Tuple3 {
		public:
			typedef A TypeA;
			typedef B TypeB;
			typedef C TypeC;
			typedef Tuple3<typename TupleTraits<A>::ValueType,
			               typename TupleTraits<B>::ValueType,
			               typename TupleTraits<C>::ValueType> ValueTuple;
			typedef Tuple3<typename TupleTraits<A>::RefType,
			               typename TupleTraits<B>::RefType,
			               typename TupleTraits<C>::RefType> RefTuple;

			Tuple3() {}
			Tuple3(typename TupleTraits<A>::ParamType a,
			       typename TupleTraits<B>::ParamType b,
			       typename TupleTraits<C>::ParamType c)
			    : a(a), b(b), c(c){
			}

			A a;
			B b;
			C c;
	};

	template <class ObjT, class Method>
	inline void DispatchToMethod(ObjT* obj, Method method, const Tuple0& arg) {
		(obj->*method)();
	}

	template <class ObjT, class Method, class A>
	inline void DispatchToMethod(ObjT* obj, Method method, const A& arg) {
		(obj->*method)(arg);
	}

	template <class ObjT, class Method, class A>
	inline void DispatchToMethod(ObjT* obj, Method method, const Tuple1<A>& arg) {
		(obj->*method)(arg.a);
	}

	template<class ObjT, class Method, class A, class B>
	inline void DispatchToMethod(ObjT* obj, Method method, const Tuple2<A, B>& arg) {
		(obj->*method)(arg.a, arg.b);
	}

	template<class ObjT, class Method, class A, class B, class C>
	inline void DispatchToMethod(ObjT* obj, Method method,
	                             const Tuple3<A, B, C>& arg) {
		(obj->*method)(arg.a, arg.b, arg.c);
	}

	// Callback --------------------------------------------------------------------
	//
	// A Callback is like a Task but with unbound parameters. It is basically an
	// object-oriented function pointer.
	//
	// Callbacks are designed to work with Tuples.  A set of helper functions and
	// classes is provided to hide the Tuple details from the consumer.  Client
	// code will generally work with the CallbackRunner base class, which merely
	// provides a Run method and is returned by the New* functions. This allows
	// users to not care which type of class implements the callback, only that it
	// has a certain number and type of arguments.
	//
	// The implementation of this is done by CallbackImpl, which inherits
	// CallbackStorage to store the data. This allows the storage of the data
	// (requiring the class type T) to be hidden from users, who will want to call
	// this regardless of the implementor's type T.
	//
	// Note that callbacks currently have no facility for cancelling or abandoning
	// them. We currently handle this at a higher level for cases where this is
	// necessary. The pointer in a callback must remain valid until the callback
	// is made.
	//
	// Like Task, the callback executor is responsible for deleting the callback
	// pointer once the callback has executed.
	//
	// Example client usage:
	//   void Object::DoStuff(int, string);
	//   Callback2<int, string>::Type* callback =
	//       NewCallback(obj, &Object::DoStuff);
	//   callback->Run(5, string("hello"));
	//   delete callback;
	// or, equivalently, using tuples directly:
	//   CallbackRunner<Tuple2<int, string> >* callback =
	//       NewCallback(obj, &Object::DoStuff);
	//   callback->RunWithParams(MakeTuple(5, string("hello")));

	// Base for all Callbacks that handles storage of the pointers.
	template <class T, typename Method>
	class CallbackStorage {
		public:
			CallbackStorage(T* obj, Method meth) : obj_(obj), meth_(meth) { }

	protected:
		T* obj_;
		Method meth_;
	};

	// Interface that is exposed to the consumer, that does the actual calling
	// of the method.
	template <typename Params>
	class CallbackRunner {
		public:
		typedef Params TupleType;

		virtual ~CallbackRunner() {}
		virtual void RunWithParams(const Params& params) = 0;

		// Convenience functions so callers don't have to deal with Tuples.
		inline void Run() {
			RunWithParams(Tuple0());
		}

		template <typename Arg1>
		inline void Run(const Arg1& a) {
			RunWithParams(Params(a));
		}

		template <typename Arg1, typename Arg2>
		inline void Run(const Arg1& a, const Arg2& b) {
		  RunWithParams(Params(a, b));
		}

		template <typename Arg1, typename Arg2, typename Arg3>
		inline void Run(const Arg1& a, const Arg2& b, const Arg3& c) {
			RunWithParams(Params(a, b, c));
		}

		template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		inline void Run(const Arg1& a, const Arg2& b, const Arg3& c, const Arg4& d) {
			RunWithParams(Params(a, b, c, d));
		}

		template <typename Arg1, typename Arg2, typename Arg3,
		          typename Arg4, typename Arg5>
		inline void Run(const Arg1& a, const Arg2& b, const Arg3& c,
		                const Arg4& d, const Arg5& e) {
			RunWithParams(Params(a, b, c, d, e));
		}
	};

	template <class T, typename Method, typename Params>
	class CallbackImpl : public CallbackStorage<T, Method>,
	                     public CallbackRunner<Params> {
		public:
		CallbackImpl(T* obj, Method meth) : CallbackStorage<T, Method>(obj, meth) { }
		virtual void RunWithParams(const Params& params) {
			// use "this->" to force C++ to look inside our templatized base class; see
			// Effective C++, 3rd Ed, item 43, p210 for details.
			DispatchToMethod(this->obj_, this->meth_, params);
		}
	};

	// 0-arg implementation
	struct Callback0 {
		typedef CallbackRunner<Tuple0> Type;
	};

	template <class T>
	typename Callback0::Type* NewCallback(T* object, void (T::*method)()) {
		return new CallbackImpl<T, void (T::*)(), Tuple0 >(object, method);
	}

	// 1-arg implementation
	template <typename Arg1>
	struct Callback1 {
		typedef CallbackRunner<Tuple1<Arg1> > Type;
	};

	template <class T, typename Arg1>
	typename Callback1<Arg1>::Type* NewCallback(T* object, void (T::*method)(Arg1)) {
		return new CallbackImpl<T, void (T::*)(Arg1), Tuple1<Arg1> >(object, method);
	}

	// 2-arg implementation
	template <typename Arg1, typename Arg2>
	struct Callback2 {
		typedef CallbackRunner<Tuple2<Arg1, Arg2> > Type;
	};

	template <class T, typename Arg1, typename Arg2>
	typename Callback2<Arg1, Arg2>::Type* NewCallback(
		T* object,
		void (T::*method)(Arg1, Arg2)) {
			return new CallbackImpl<T, void (T::*)(Arg1, Arg2),
			Tuple2<Arg1, Arg2> >(object, method);
		}

	// 3-arg implementation
	template <typename Arg1, typename Arg2, typename Arg3>
	struct Callback3 {
		typedef CallbackRunner<Tuple3<Arg1, Arg2, Arg3> > Type;
	};

	template <class T, typename Arg1, typename Arg2, typename Arg3>
	typename Callback3<Arg1, Arg2, Arg3>::Type* NewCallback(
		T* object,
		void (T::*method)(Arg1, Arg2, Arg3)) {
			return new CallbackImpl<T,  void (T::*)(Arg1, Arg2, Arg3),
			Tuple3<Arg1, Arg2, Arg3> >(object, method);
		}

	// An UnboundMethod is a wrapper for a method where the actual object is
	// provided at Run dispatch time.
	template <class T, class Method, class Params>
	class UnboundMethod {
		public:
			UnboundMethod(Method m, Params p) : m_(m), p_(p) {}
			void Run(T* obj) const {
			DispatchToMethod(obj, m_, p_);
		}
		private:
			Method m_;
			Params p_;
	};
}


#endif
