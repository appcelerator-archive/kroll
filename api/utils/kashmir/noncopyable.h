/********************************************************************\
 * noncopyable.h -- prevents copy of derived classes                *
 *                                                                  *
 * Copyright (C) 2008 Kenneth Laskoski                              *
 *                                                                  *
\********************************************************************/
/** @file noncopyable.h
    @brief prevents copy of derived classes 
    @author Copyright (C) 2008 Kenneth Laskoski
    shameless copy of work by
    @author Copyright (C) 1999-2003 Beman Dawes

    Use, modification, and distribution are subject
    to the Boost Software License, Version 1.0.  (See accompanying file
    LICENSE_1_0.txt or a copy at <http://www.boost.org/LICENSE_1_0.txt>.)

    See http://www.boost.org/libs/utility for documentation.
*/

#ifndef KL_NONCOPYABLE_H
#define KL_NONCOPYABLE_H

namespace kashmir {

//  Private copy constructor and copy assignment ensure classes derived from
//  class noncopyable cannot be copied.

//  Contributed by Dave Abrahams

namespace noncopyable_  // protection from unintended ADL
{
    class noncopyable
    {
    protected:
        noncopyable() {}
        ~noncopyable() {}
    private:  // emphasize the following members are private
        noncopyable( const noncopyable& );
        const noncopyable& operator=( const noncopyable& );
    };
}

typedef noncopyable_::noncopyable noncopyable;

}

#endif
