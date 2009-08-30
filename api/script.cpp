/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "script.h"
#include <Poco/File.h>
#include <Poco/TemporaryFile.h>

namespace kroll
{
	/*static*/
	SharedPtr<Script> Script::instance = 0;
	
	/*static*/
	SharedPtr<Script> Script::GetInstance()
	{
		return instance;
	}
	
	/*static*/
	void Script::Initialize()
	{
		instance = new Script();
	}
	
	/*static*/
	bool Script::HasExtension(const char *script, const char *extension)
	{
		std::string scriptStr(script);
		std::string extStr(".");
		extStr += extension;
		
		return scriptStr.size() >= extStr.size() && scriptStr.substr(scriptStr.size()-extStr.size()) == extStr;
	}
	
	void Script::AddScriptEvaluator(SharedKObject evaluator)
	{
		evaluators->Append(Value::NewObject(evaluator));
	}
	
	void Script::RemoveScriptEvaluator(SharedKObject evaluator)
	{
		int index = -1;
		for (size_t i = 0; i < evaluators->Size(); i++)
		{
			if (evaluators->At(i)->ToObject()->Equals(evaluator))
			{
				index = i;
				break;
			}
		}
		if (index != -1)
		{
			evaluators->Remove(index);
		}
	}
	
	SharedKObject Script::FindEvaluatorWithMethod(const char *method, const char *arg)
	{
		ValueList args;
		args.push_back(Value::NewString(arg));
		
		for (size_t i = 0; i < evaluators->Size(); i++)
		{
			SharedKMethod finder = evaluators->At(i)->ToObject()->GetMethod(method);
			if (!finder.isNull())
			{
				SharedValue result = finder->Call(args);
				if (result->IsBool() && result->ToBool())
				{
					return evaluators->At(i)->ToObject();
				}
			}
		}
		return 0;
	}
	
	bool Script::CanEvaluate(const char *mimeType)
	{
		return !this->FindEvaluatorWithMethod("canEvaluate", mimeType).isNull();
	}
	
	bool Script::CanPreprocess(const char *url)
	{
		return !this->FindEvaluatorWithMethod("canPreprocess", url).isNull();
	}
	
	SharedValue Script::Evaluate(const char *mimeType, const char *name, const char *code, SharedKObject scope)
	{
		SharedKObject evaluator = this->FindEvaluatorWithMethod("canEvaluate", mimeType);
		if (!evaluator.isNull())
		{
			SharedKMethod evaluate = evaluator->GetMethod("evaluate");
			if (!evaluate.isNull())
			{
				ValueList args;
				args.push_back(Value::NewString(mimeType));
				args.push_back(Value::NewString(name));
				args.push_back(Value::NewString(code));
				args.push_back(Value::NewObject(scope));
				return evaluate->Call(args);
			}
			else
			{
				throw ValueException::FromFormat(
					"Error evaluating: No \"evaluate\" method found on evaluator for mimeType: \"%s\"", mimeType);
			}
		}
		else
		{
			throw ValueException::FromFormat("Error evaluating: No evaluator found for mimeType: \"%s\"", mimeType);
		}
	}
	
	SharedString Script::Preprocess(const char *url, SharedKObject scope)
	{
		SharedKObject evaluator = this->FindEvaluatorWithMethod("canPreprocess", url);
		if (!evaluator.isNull())
		{
			SharedKMethod preprocess = evaluator->GetMethod("preprocess");
			if (!preprocess.isNull())
			{
				ValueList args;
				args.push_back(Value::NewString(url));
				args.push_back(Value::NewObject(scope));
				
				SharedValue result = preprocess->Call(args);
				
				if (result->IsString())
				{
					// TODO : have the preprocessor return mime type?
					// Forcing to HTML extension makes pages display correctly, but scripts fail to execute
					std::string extension = url;
					extension = extension.substr(extension.rfind("."));
					//std::string extension = "html";
					
					Poco::File tempFile(Poco::TemporaryFile::tempName()+extension);
					Poco::TemporaryFile::registerForDeletion(tempFile.path());
					tempFile.createFile();
 
					std::ofstream ostream(tempFile.path().c_str());
					ostream << result->ToString();
					ostream.close();
				
					return new std::string(URLUtils::PathToFileURL(tempFile.path()));
				}
			}
			else
			{
				throw ValueException::FromFormat(
					"Error preprocessing: No \"preprocess\" method found on evaluator for url: \"%s\"", url);
			}
		}
		else
		{
			throw ValueException::FromFormat("Error preprocessing: No evaluator found for url: \"%s\"", url);
		}
		return 0;
	}
	
	
}