/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"
#if defined(KROLL_API_EXPORT) || defined(_KROLL_H_)
#include <Poco/URI.h>
#include <Poco/TemporaryFile.h>
#include <Poco/FileStream.h>
#endif
namespace UTILS_NS
{
namespace URLUtils
{
	namespace
	{
		const char HEX2DEC[256] =
		{
			/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
			/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
	
			/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	
			/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	
			/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
			/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
		};

		// Only alphanum is safe.
		const char SAFE[256] =
		{
			/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
			/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0,
			/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,
	
			/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
			/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,1,
			/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
			/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
	
			/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	
			/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
			/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
		};
	}

	std::string EncodeURIComponent(std::string src)
	{
		const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
		const unsigned char *pSrc = (const unsigned char *)src.c_str();
		const int SRC_LEN = (int) src.length();
		unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
		unsigned char * pEnd = pStart;
		const unsigned char * const SRC_END = pSrc + SRC_LEN;

		for (; pSrc < SRC_END; ++pSrc)
		{
			if (SAFE[*pSrc])
				*pEnd++ = *pSrc;
			else
			{
				// escape this char
				*pEnd++ = '%';
				*pEnd++ = DEC2HEX[*pSrc >> 4];
				*pEnd++ = DEC2HEX[*pSrc & 0x0F];
			}
		}

		std::string sResult((char *)pStart, (char *)pEnd);
		delete [] pStart;

		return sResult;
	}

	std::string DecodeURIComponent(std::string src)
	{
		// Note from RFC1630: "Sequences which start with a percent
		// sign but are not followed by two hexadecimal characters
		// (0-9, A-F) are reserved for future extension"

		const unsigned char * pSrc = (const unsigned char *)src.c_str();
		const int SRC_LEN = (int) src.length();
		const unsigned char * const SRC_END = pSrc + SRC_LEN;
		// last decodable '%'
		const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

		char * const pStart = new char[SRC_LEN];
		char * pEnd = pStart;

		while (pSrc < SRC_LAST_DEC)
		{
			if (*pSrc == '%')
			{
				char dec1, dec2;
				if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
					&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
				{
					*pEnd++ = (dec1 << 4) + dec2;
					pSrc += 3;
					continue;
				}
			}

			*pEnd++ = *pSrc++;
		}

		// the last 2- chars
		while (pSrc < SRC_END)
			*pEnd++ = *pSrc++;

		std::string sResult(pStart, pEnd);
		delete [] pStart;

		return sResult;
	}


	std::string FileURLToPath(std::string url)
	{
		size_t fileLength = 7; // file://
		if (url.find("file://") == 0)
		{
			url = url.substr(fileLength);
		}

		if ('/' != KR_PATH_SEP_CHAR)
		{
			for (size_t i = 0; i < url.size(); i++)
			{
				if (url[i] == '/')
				{
						url[i] = KR_PATH_SEP_CHAR;
				}
			}
		}
		return url;
	}

	std::string PathToFileURL(std::string pathIn)
	{
		// We need to convert this to a Unicode string, because we'll
		// be iterating through it a character at a time. This might
		// break with UTF-8.
		std::wstring path(UTF8ToWide(pathIn));

		// Windows needs it's slashes swapped.
#if OS_WIN32
		for (size_t i = 0; i < path.size(); i++)
		{
			if (path[i] == L'\\')
			{
				path[i] = L'/';
			}
		}
#endif
		std::string url("file://");
		std::vector<std::wstring> pieces;
		FileUtils::TokenizeWide(path, pieces, L"/");
		for (size_t i = 0; i < pieces.size(); i++)
		{
			std::string piece(WideToUTF8(pieces[i]));
			url.append("/");
#if OS_WIN32
			// Don't encode the C: section.
			if (i != 0)
#endif
				piece = EncodeURIComponent(piece);

			url.append(piece);
		}
		return url;
	}

#if defined(KROLL_API_EXPORT) || defined(_KROLL_H_)
	static std::string NormalizeAppURL(const std::string& url)
	{
		size_t appLength = 6; // app://
		std::string id(Host::GetInstance()->GetApplication()->id);
		size_t idLength = id.size();
		std::string idPart(url.substr(appLength, idLength));

		if (idPart == id)
		{
			return url;
		}
		else
		{
			return std::string("app://") + id + "/" + url.substr(appLength);
		}
	}

	std::string& BlankPageURL()
	{
		static std::string url("app://__blank__.html");
		return url;
	}

	static std::string& BlankURLToFilePath()
	{
		static std::string path;
		if (path.empty())
		{
			Poco::TemporaryFile temp;
			temp.keepUntilExit();
			path = temp.path();

			std::string contents("<html><body></body></html>");
			Poco::FileStream stream;
			stream.open(path, std::ios::out);
			stream.write(contents.c_str(), contents.size());
			stream.close();
		}
		return path;
	}

	std::string NormalizeURL(const std::string& url)
	{
		Poco::URI inURI = Poco::URI(url);
		if (url == BlankPageURL())
		{
			return url;
		}
		if (inURI.getScheme() != "app")
		{
			return url;
		}
		else
		{
			return NormalizeAppURL(url);
		}
	}

	std::string URLToPath(const std::string& url)
	{
		Poco::URI inURI = Poco::URI(url);
		try
		{
			if (url == BlankPageURL())
			{
				return BlankURLToFilePath();
			}
			if (inURI.getScheme() == "ti")
			{
				return TiURLToPath(url);
			}
			else if (inURI.getScheme() == "app")
			{
				return AppURLToPath(url);
			}
			else if (inURI.getScheme().empty())
			{
				// There is no scheme for this URL, so we have to/ guess at this point if
				// it's a path or a relative app:// URL. If a file can be found, assume thi
				// is a file path.
				if (FileUtils::IsFile(url))
					return url;

				// Otherwise treat this like an app:// URL relative to the root.
				std::string newURL("app://");
				newURL.append(url);
				return AppURLToPath(newURL);
			}
		}
		catch (ValueException& e)
		{
			SharedString ss = e.DisplayString();
			Logger* log = Logger::Get("URLUtils");
			log->Error("Could not convert %s to a path: %s", url.c_str(), ss->c_str());
		}
		return url;
	}

	std::string TiURLToPath(const std::string& tiURL)
	{
		try
		{
			Poco::URI inURI = Poco::URI(tiURL);

			if (inURI.getScheme() != "ti")
			{
				return tiURL;
			}

			std::string host(inURI.getHost());
			SharedApplication app = Host::GetInstance()->GetApplication();
			std::string path(app->GetComponentPath(host));

			if (path.empty())
			{
				throw ValueException::FromString("Could not find component "+host);
			}

			std::vector<std::string> segments;
			inURI.getPathSegments(segments);

			for (size_t i = 0; i < segments.size(); i++)
			{
				path = FileUtils::Join(path.c_str(), segments[i].c_str(), NULL);
			}
			return path;
		}
		catch (ValueException& e)
		{
			SharedString ss = e.DisplayString();
			Logger* log = Logger::Get("URLUtils");
			log->Error("Could not convert %s to a path: %s", tiURL.c_str(), ss->c_str());
		}
		catch (...)
		{
			Logger* log = Logger::Get("URLUtils");
			log->Error("Could not convert %s to a path", tiURL.c_str());
		}
		return tiURL;
	}

	std::string AppURLToPath(const std::string& inURL)
	{
		try
		{
			Poco::URI inURI = Poco::URI(inURL);
			if (inURI.getScheme() != "app")
			{
				return inURL;
			}

			std::string appURL(NormalizeAppURL(inURL));
			inURI = Poco::URI(appURL);

			SharedApplication app = Host::GetInstance()->GetApplication();
			std::string path(app->GetResourcesPath());

			std::vector<std::string> segments;
			inURI.getPathSegments(segments);
			for (size_t i = 0; i < segments.size(); i++)
			{
				path = FileUtils::Join(path.c_str(), segments[i].c_str(), NULL);
			}
			return path;
		}
		catch (ValueException& e)
		{
			SharedString ss = e.DisplayString();
			Logger* log = Logger::Get("URLUtils");
			log->Error("Could not convert %s to a path: %s", inURL.c_str(), ss->c_str());
		}
		catch (...)
		{
			Logger* log = Logger::Get("URLUtils");
			log->Error("Could not convert %s to a path", inURL.c_str());
		}

		return inURL;
	}
#endif
}
}
