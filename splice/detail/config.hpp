
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SPLICE_CONFIG_HPP
#define SPLICE_CONFIG_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif

//#if defined(BOOST_WINDOWS)
//# include <boost/static_assert.hpp>
//# include <boost/detail/winapi/config.hpp>
//# if !defined(BOOST_USE_WINAPI_VERSION)
//    BOOST_STATIC_ASSERT_MSG(0,"# include <boost/config.hpp> implies that BOOST_USE_WINAPI_VERSION is defined ");
//# else
////#   undef BOOST_USE_WINAPI_VERSION
////#   define BOOST_USE_WINAPI_VERSION BOOST_WINAPI_VERSION_VISTA
////#   define _WIN32_WINNT 0x06000000
//#   define _WIN32_WINNT 0x05010000
//# endif
//#endif

#if defined(BOOST_WINDOWS) && !defined(_WIN32_WINNT) 
# define _WIN32_WINNT 0x05010000
#endif


#include <boost/version.hpp>
#include <boost/asio/detail/config.hpp>

// Default to a header-only implementation. The user must specifically request
// separate compilation by defining SPLICE_SEPARATE_COMPILATION
#if !defined(SPLICE_HEADER_ONLY)
# if !defined(SPLICE_SEPARATE_COMPILATION)
#   define SPLICE_HEADER_ONLY
# endif // !defined(SPLICE_SEPARATE_COMPILATION)
#endif // !defined(SPLICE_HEADER_ONLY)

#endif // #ifndef SPLICE_CONFIG_HPP

