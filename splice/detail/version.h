
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//


#ifndef EZ_SOCKET_VERSION_HPP
#define EZ_SOCKET_VERSION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

// EZ_SOCKET_VERSION % 100 is the sub-minor version
// EZ_SOCKET_VERSION / 100 % 1000 is the minor version
// EZ_SOCKET_VERSION / 100000 is the major version
#define EZ_SOCKET_VERSION 000000 // 0.00.0

#endif // EZ_SOCKET_VERSION_HPP
