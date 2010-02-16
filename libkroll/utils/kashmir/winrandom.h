/********************************************************************\
 * winrandom.h -- Windows random number generator                   *
 *                                                                  *
 * Copyright (C) 2008 Kenneth Laskoski                              *
 *                                                                  *
\********************************************************************/
/** @file winrandom.h
    @brief Windows random number generator
    @author Copyright (C) 2008 Kenneth Laskoski
    based on work by
    @author Copyright (C) 1996, 1997, 1998 Theodore Ts'o
    @author Copyright (C) 2004-2008 Ralf S. Engelschall <rse@engelschall.com>

    Use, modification, and distribution are subject
    to the Boost Software License, Version 1.0.  (See accompanying file
    LICENSE_1_0.txt or a copy at <http://www.boost.org/LICENSE_1_0.txt>.)
*/

#ifndef KL_WINRANDOM_H 
#define KL_WINRANDOM_H 

#include "randomstream.h"
#include "noncopyable.h"

#include <stdexcept>

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <wincrypt.h>

namespace kashmir {
namespace system {

class WinRandom : public user::randomstream<WinRandom>, noncopyable
{
public:
	WinRandom()
    {
		//always initialize member variables.
		hProv = NULL;

		// grab the default cryptographic context to verify the machine.  
		// we don't need access to the private keys, so we use CRYPT_VERIFYCONTEXT
        if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
            throw std::runtime_error("failed to acquire cryptographic context.");
    }

    ~WinRandom()
    {
		if ( hProv )
	        CryptReleaseContext(hProv, 0);
    }

    void read(char* buffer, std::size_t count)
    {
        if (hProv && !CryptGenRandom(hProv, count, (BYTE*)buffer))
            throw std::runtime_error("system failed to generate random data.");
    }

private:
    HCRYPTPROV hProv;
};

}}

#endif
