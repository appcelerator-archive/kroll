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
	
	/*static*/
 	std::string Script::GetExtension(const char *script)
	{
		std::string scriptStr(script);
		return scriptStr.substr(scriptStr.rfind("."));
	}
	
	void Script::AddScriptEvaluator(KObjectRef evaluator)
	{
		evaluators->Append(Value::NewObject(evaluator));
	}
	
	void Script::RemoveScriptEvaluator(KObjectRef evaluator)
	{
		int index = -1;
		for (unsigned int i = 0; i < evaluators->Size(); i++)
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
	
	KObjectRef Script::FindEvaluatorWithMethod(const char *method, const char *arg)
	{
		ValueList args;
		args.push_back(Value::NewString(arg));
		
		for (unsigned int i = 0; i < evaluators->Size(); i++)
		{
			KMethodRef finder = evaluators->At(i)->ToObject()->GetMethod(method);
			if (!finder.isNull())
			{
				KValueRef result = finder->Call(args);
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
	
	KValueRef Script::Evaluate(const char *mimeType, const char *name, const char *code, KObjectRef scope)
	{
		KObjectRef evaluator = this->FindEvaluatorWithMethod("canEvaluate", mimeType);
		if (!evaluator.isNull())
		{
			KMethodRef evaluate = evaluator->GetMethod("evaluate");
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
	
	AutoPtr<PreprocessData> Script::Preprocess(const char *url, KObjectRef scope)
	{
		KObjectRef evaluator = this->FindEvaluatorWithMethod("canPreprocess", url);
		if (!evaluator.isNull())
		{
			KMethodRef preprocess = evaluator->GetMethod("preprocess");
			if (!preprocess.isNull())
			{
				ValueList args;
				args.push_back(Value::NewString(url));
				args.push_back(Value::NewObject(scope));
				
				KValueRef result = preprocess->Call(args);
				
				if (result->IsObject())
				{
					KObjectRef object = result->ToObject();
					AutoPtr<PreprocessData> data = new PreprocessData();
					if (object->HasProperty("data"))
					{
						KValueRef objectData = object->Get("data");
						if (objectData->IsObject())
						{
							BlobRef blobData = objectData->ToObject().cast<Blob>();
							if (!blobData.isNull())
							{
								data->data = blobData;
							}
						}
						else if (objectData->IsString())
						{
							data->data = new Blob(objectData->ToString(), strlen(objectData->ToString()));
						}
					}
					else
					{
						throw ValueException::FromString("Preprocessor didn't return any data");
					}
					if (object->HasProperty("mimeType"))
					{
						data->mimeType = object->Get("mimeType")->ToString();
					}
					else
					{
						throw ValueException::FromString("Preprocessor didn't return a mimeType");
					}
					
					return data;
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
