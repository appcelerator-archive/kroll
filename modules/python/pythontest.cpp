/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "pythontest.h"
#include "pythontypes.h"
#include "pythonvalue.h"
#include "pythonlist.h"

namespace kroll
{
	void PythonUnitTestSuite::Run(Host *host)
	{
		Value* tv1 = new Value(1);
		PyObject* v1 = ValueToPythonValue(tv1);
		KR_ASSERT(PyInt_Check(v1));
		KR_ASSERT(PyInt_AsLong(v1)==1);
		
		Value* tv2 = new Value(1.0);
		PyObject* v2 = ValueToPythonValue(tv2);
		KR_ASSERT(PyFloat_Check(v2));
		KR_ASSERT(PyFloat_AsDouble(v2)==1.0);
		
		Value* tv3 = new Value("abc");
		PyObject* v3 = ValueToPythonValue(tv3);
		KR_ASSERT(PyString_Check(v3));
		KR_ASSERT_STR(PyString_AsString(v3),"abc");
		
		std::string s1("abc");
		Value* tv4 = new Value(s1);
		PyObject* v4 = ValueToPythonValue(tv4);
		KR_ASSERT(PyString_Check(v4));
		KR_ASSERT_STR(PyString_AsString(v4),"abc");
		
		Value* tv5 = new Value();
		PyObject* v5 = ValueToPythonValue(tv5);
		KR_ASSERT(Py_None==v5);
		
		Value* tv6 = Value::Undefined();
		PyObject* v6 = ValueToPythonValue(tv6);
		KR_ASSERT(Py_None==v6);
		
		Value* tv7 = Value::Null();
		PyObject* v7 = ValueToPythonValue(tv7);
		KR_ASSERT(Py_None==v7);
		
		Value* tv8 = new Value(true);
		PyObject* v8 = ValueToPythonValue(tv8);
		KR_ASSERT(PyBool_Check(v8));
		KR_ASSERT(v8 == Py_True);
				
		std::string script1;
		script1+="class Foo:\n";
		script1+="	i = 12345\n";
		script1+="	def bar(self):\n";
		script1+="		return 'hello,world'\n";
		script1+="	def blah(self,x):\n";
		script1+="		return x\n";
		script1+="	def blok(self,fn):\n";
		script1+="		fn()\n";
		script1+="\n";
		
		PyRun_SimpleString(script1.c_str());
		
		PyObject* main_module = PyImport_AddModule("__main__");
		PyObject* global_dict = PyModule_GetDict(main_module);
		PyObject* expression = PyDict_GetItemString(global_dict, "Foo");
		Value* cl1 = PythonValueToValue(expression);
		KR_ASSERT(cl1->IsObject());
		PythonValue* value = new PythonValue(expression);
		ScopedDereferencer sd1(value);
		Value* p1 = value->Get("bar");
		KR_ASSERT(p1->IsMethod());
		
		// TEST creating instance and invoking methods
		PyRun_SimpleString("foopy = Foo()");
		PyObject* foopy = PyDict_GetItemString(global_dict, "foopy");
		Value* cl2 = PythonValueToValue(foopy);
		ScopedDereferencer r(cl2);
		KR_ASSERT(cl2->IsObject());
		PythonValue* value2 = new PythonValue(foopy);
		ScopedDereferencer sd2(value2);
		
		// TEST method
		Value* p2 = value2->Get("bar");
		KR_ASSERT(p2->IsMethod());
		BoundMethod* m1 = p2->ToMethod();
		ValueList args;
		Value* mv1 = m1->Call(args);
		KR_ASSERT(!mv1->IsNull());
		KR_ASSERT_STR(mv1->ToString().c_str(),"hello,world");
		
		// TEST property accessor
		Value* p3 = value2->Get("i");
		KR_ASSERT(p3->IsInt());
		KR_ASSERT(p3->ToInt()==12345);
		
		// TEST setting properties
		Value* set1 = new Value(6789);
		value2->Set("i",set1);
		Value* p4 = value2->Get("i");
		KR_ASSERT(p4->IsInt());
		KR_ASSERT(p4->ToInt()==6789);
		
		// TEST setting invalid property - this should add it dynamically
		Value* set2 = new Value(1);
		value2->Set("i2",set2);
		Value* p5 = value2->Get("i2");
		KR_ASSERT(p5->IsInt());
		KR_ASSERT(p5->ToInt()==1);	
		
		// TEST undefined property
		Value* p6 = value2->Get("x");
		KR_ASSERT(p6->IsUndefined());
		
		// TEST invoking a function with wrong parameters and checking exception
		Value* p7 = value2->Get("blah");
		KR_ASSERT(p7->IsMethod());
		BoundMethod* mb2 = p7->ToMethod();
		Value *mv3 = NULL;
		try
		{
			mv3 = mb2->Call(args);
			KR_ASSERT(false);
		}
		catch (Value* e)
		{
			KR_ASSERT_STR(e->ToString().c_str(),"blah() takes exactly 2 arguments (1 given)");
		}
		
		// TEST invoking a function with correct parameters
		ValueList args2;
		Value* argsp1 = new Value("hello,world");
		args2.push_back(argsp1);
		Value* mv4 = mb2->Call(args2);
		KR_ASSERT(mv4);
		
		// TEST for Python callable functions
		PyRun_SimpleString("def foopyc(): return 'hello,world'");
		PyObject* foopyc = PyDict_GetItemString(global_dict, "foopyc");
		KR_ASSERT(foopyc);
		Value* cl3 = PythonValueToValue(foopyc);
		KR_ASSERT(cl3->IsMethod());
		BoundMethod* m2 = cl3->ToMethod();
		Value* mv2 = m2->Call(args);
		KR_ASSERT(!mv2->IsNull());
		KR_ASSERT_STR(mv2->ToString().c_str(),"hello,world");
		
		// TEST creating an anonymous method and calling through it
		PyObject* anon1 = BoundMethodToPythonValue(cl3->ToMethod());
		KR_ASSERT(PyCallable_Check(anon1));
		
		// TEST calling through a BoundObject from Python
		BoundObject* tbo = cl1->ToObject();
		PyObject* co1 = BoundObjectToPythonValue(NULL,NULL,tbo);
		KR_ASSERT(co1);
		PyObject* cop1 = PyObject_GetAttrString(co1, "bar");
		KR_ASSERT(cop1);
		
		Value* cl5 = PythonValueToValue(foopyc);
		KR_ASSERT(cl5->IsMethod());
		BoundMethod* m4 = cl5->ToMethod();
		Value* mv6 = m4->Call(args);
		KR_ASSERT_STR(mv6->ToString().c_str(),"hello,world");
		
		PyObject *anon2 = BoundMethodToPythonValue(m4);
		KR_ASSERT(PyCallable_Check(anon2));
		Value* tiv1 = PythonValueToValue(anon2);
		KR_ASSERT(tiv1->IsMethod());
		BoundMethod *tibm1 = tiv1->ToMethod();
		Value* tivr1 = tibm1->Call(args);
		KR_ASSERT_STR(tivr1->ToString().c_str(),"hello,world");
		
		
		PyObject* piv = BoundMethodToPythonValue(tibm1);
		Value* pivv = PythonValueToValue(piv);
		BoundMethod *pivbm = pivv->ToMethod();
		Value* pivbmv = pivbm->Call(args);
		KR_ASSERT_STR(pivbmv->ToString().c_str(),"hello,world");
		
		std::string script2;
		script2+=PRODUCT_NAME".set('x',123)\n";
		script2+="f = "PRODUCT_NAME".get('x') + 1\n";
		script2+=PRODUCT_NAME".set('y',f)\n";
		script2+="\n";
		
		PyRun_SimpleString(script2.c_str());
		
		// TEST pulling out the new bound values
		Value *x = host->GetGlobalObject()->GetNS("python.x");
		KR_ASSERT(x->ToInt()==123);
		
		Value *y = host->GetGlobalObject()->GetNS("python.y");
		KR_ASSERT(y->ToInt()==124);
		
		KR_DECREF(x);
		KR_DECREF(y);
		
		BoundList *list = new StaticBoundList();
		KR_ASSERT(list->Size()==0);
		KR_ASSERT(list->At(0)->IsUndefined());
		Value* lista = new Value(1);
		list->Append(lista);
		KR_ASSERT(list->Size()==1);
		KR_DECREF(lista);
		
		Value *listv = new Value(list);
		PyObject* apylist = ValueToPythonValue(listv);
		KR_ASSERT(apylist);
		KR_ASSERT(listv->IsList());
		KR_ASSERT(PyList_Check(apylist));
		KR_ASSERT(PyList_Size(apylist)==list->Size());
		
		PyObject *pitem = PyList_GetItem(apylist,0);
		KR_ASSERT(pitem);
		KR_ASSERT(pitem != Py_None);
		Value *vitem = PythonValueToValue(pitem);
		KR_ASSERT(vitem->IsInt());
		KR_ASSERT(vitem->ToInt()==1);

		PythonList *plist = new PythonList(apylist);
		KR_ASSERT(plist->Size()==1);
		KR_ASSERT(plist->At(0)->ToInt()==1);
		Value *vlist2 = new Value("hello");
		plist->Append(vlist2);
		KR_ASSERT(plist->Size()==2);
		Value *vlist3 = plist->At(1);
		KR_ASSERT(vlist3->ToString()=="hello");
		KR_ASSERT(list->At(10)->IsUndefined());
		
		KR_DECREF(vlist2);
		KR_DECREF(plist);
		Py_DECREF(pitem);
		KR_DECREF(vitem);
		KR_DECREF(listv);
		KR_DECREF(list);
		Py_DECREF(apylist);
		
	}
}
