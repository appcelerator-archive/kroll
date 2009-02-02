/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <sstream>
#include <cstring>

namespace kroll
{

	bool ArgUtils::VerifyArgs(const ValueList& args, const char* sig)
	{
		std::vector<std::string>* sig_vector = ArgUtils::ParseSigString(sig);
		return ArgUtils::VerifyArgsImpl(args, sig_vector);
	}

	void ArgUtils::VerifyArgsException(const char *name, const ValueList& args, const char* sig)
	{
		std::vector<std::string>* sig_vector = ArgUtils::ParseSigString(sig);
		if (!VerifyArgsImpl(args, sig_vector))
		{
			SharedString sig = ArgUtils::GenerateSignature(name, sig_vector);
			std::string s = "Invalid arguments passed for: " + *sig;
			delete sig_vector;
			throw ValueException::FromString(s);
		}
		delete sig_vector;
	}

	inline std::vector<std::string>* ArgUtils::ParseSigString(const char* sig)
	{
		std::vector<std::string>* sig_vector = new std::vector<std::string>();
		std::string types = "";
		while (true)
		{
			if (*sig == '\0')
			{
				break;
			}
			else if (*sig == ',' || *sig == ' ')
			{
				sig_vector->push_back(types);
				types = "";
			}
			else
			{
				types += *sig;
			}
			*sig++;
		}
		return sig_vector;
	}

	SharedString ArgUtils::GenerateSignature(const char* name, std::vector<std::string>* sig_vector)
	{
		std::ostringstream out;
		bool optional = false;
		for (size_t i = 0; i < sig_vector->size(); i++)
		{
			const char *t = sig_vector->at(i).c_str();
			// The first time we see the optional
			// parameter we stay in optional mode
			// until the end of the session.
			if (*t== '?')
			{
				optional = true;
				out << "[";
				*t++;
			}

			while (*t != '\0')
			{
				if (*t == 's')
					out << "String";
				else if (*t == 'i')
					out << "Integer";
				else if (*t == 'd')
					out << "Double";
				else if (*t == 'n')
					out << "Number";
				else if (*t == 'o')
					out << "Object";
				else if (*t == 'l')
					out << "List";
				else if (*t == 'm')
					out << "Function";
				else if (*t == '0')
					out << "Null";
				*t++;
				if (*t != '\0')
					out << "|";
			}

			if (i != sig_vector->size() - 1)
				out << ",";
		}

		if (optional)
			out << "]";
		out << ")";

		return new std::string(out.str());
	}

	bool ArgUtils::VerifyArgsImpl(const ValueList& args, std::vector<std::string>* sig_vector)
	{
		bool optional = false;
		for (size_t i = 0; i < sig_vector->size(); i++)
		{
			const char *t = sig_vector->at(i).c_str();

			// The first time we see the optional
			// parameter we stay in optional mode
			// until the end of the session.
			if (*t == '?')
			{
				optional = true;
				*t++;
			}

			// Not enough args given, but we're in
			// optional mode.
			if (args.size() < i + 1 && optional)
				return true;

			// Not enough args given.
			if (args.size() < i + 1)
				return false;

			// Arg doesn't conform to arg string
			if (!VerifyArg(args.at(i), t))
				return false;
		}
		return true;
	}


	inline bool ArgUtils::VerifyArg(SharedValue arg, const char* t)
	{
		while (*t != '\0')
		{
		// Check if type of value matches current character.
		if ((*t == 's' && arg->IsString())
		 || (*t == 'i' && arg->IsInt())
		 || (*t == 'd' && arg->IsDouble())
		 || (*t == 'n' && arg->IsNumber())
		 || (*t == 'o' && arg->IsObject())
		 || (*t == 'l' && arg->IsList())
		 || (*t == 'm' && arg->IsMethod())
		 || (*t == '0' && arg->IsNull()))
			return true;
		else
			*t++;
		}
		return false;
	}

}

