/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef PYTHON_EVALUATOR_H_
#define PYTHON_EVALUATOR_H_

namespace kroll
{
	class PythonEvaluator : public KMethod
	{
		public:
		virtual SharedValue Call(const ValueList& args);
		virtual void Set(const char *, SharedValue);
		virtual SharedValue Get(const char *);
		virtual SharedStringList GetPropertyNames();

		private:
		static void StripLeadingWhitespace(std::string &);
		static void ConvertLineEndings(std::string &);
		static void DictToKObjectProps(PyObject* map, SharedKObject o);
		static void KObjectPropsToDict(SharedKObject o, PyObject* map);

	};
}

#endif

