
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MONO_PROTOCOL_HPP
#define MONO_PROTOCOL_HPP

#ifndef BOOST_CONFIG_HPP
# include <boost/config.hpp>
#endif
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif
#include "detail/config.hpp"

#include "protocol.hpp"

namespace splice
{

  template <typename up_t,typename below_t>
  class mono_protocol: public protocol<up_t,below_t>
  {
  protected:
    // Initiate an asynchronous accept operation.
    template <typename session_t>
    void start_accept()BOOST_NOEXCEPT;

    // Handle completion of an asynchronous accept operation.
    template <typename session_t>
    void on_accept(
      boost::shared_ptr<session_t> session,
      const boost::system::error_code& error)BOOST_NOEXCEPT;
  };


} // namespace splice

#if defined(SPLICE_HEADER_ONLY)
# include "mono_protocol.hxx"
#endif // defined(SPLICE_HEADER_ONLY)

#endif // #ifndef MONO_PROTOCOL_HPP
