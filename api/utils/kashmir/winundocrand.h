/********************************************************************\
 * winundocrand.h -- Windows random number generator                *
 *                                                                  *
 * Copyright (C) 2008 Kenneth Laskoski                              *
 *                                                                  *
\********************************************************************/
/** @file winundocrand.h
    @brief Windows random number generator
    @author Copyright (C) 2008 Kenneth Laskoski
    based on work by
    @author Copyright (C) 1996, 1997, 1998 Theodore Ts'o
    @author Copyright (C) 2004-2008 Ralf S. Engelschall <rse@engelschall.com>

    Use, modification, and distribution are subject
    to the Boost Software License, Version 1.0.  (See accompanying file
    LICENSE_1_0.txt or a copy at <http://www.boost.org/LICENSE_1_0.txt>.)
*/

#ifndef KL_WINUNDOCRAND_H
#define KL_WINUNDOCRAND_H

#include "randomstream.h"
#include "noncopyable.h"

#include <stdexcept>

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#include <windows.h>

namespace kashmir {
namespace system {

class WinUndocRand : public user::randomstream<WinUndocRand>, noncopyable
{
public:
    WinUndocRand() : hLib(LoadLibrary("ADVAPI32.DLL")), pfn(0)
    {
        if (!hLib)
            throw std::runtime_error("failed to load ADVAPI32.DLL.");

        pfn = (BOOLEAN (APIENTRY *)(void*,ULONG)) GetProcAddress(hLib,"SystemFunction036");
        if (!pfn)
        {
            FreeLibrary(hLib);
            throw std::runtime_error("failed to get ADVAPI32!RtlGenRandom address.");
        }
    }

    ~WinUndocRand()
    {
        FreeLibrary(hLib);
    }

    void read(char* buffer, std::size_t count)
    {
        if (!pfn(buffer, count))
            throw std::runtime_error("system failed to generate random data.");
    }

private:
    HMODULE hLib;
    BOOLEAN (APIENTRY *pfn)(void*, ULONG);
};

}}

#endif
