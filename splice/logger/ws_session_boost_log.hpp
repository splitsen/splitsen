
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef WEBSOCKET_SESSION_BOOST_LOG_HPP
#define WEBSOCKET_SESSION_BOOST_LOG_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

#include "boost_log_interface.hpp"

#ifdef SPLICE_LOG_TRACE

#include <boost/logic/tribool_io.hpp>
#include <locale>
#include <sstream>

namespace splice
{

  struct ws_session_boost_log: boost_log_interface
  {
    void log_trace_t(const char* file,unsigned line,const char* func,
      void* obj,const std::string& err_msg,
      const char* incoming_data,
      std::size_t bytes_transferred)
    {
      boost::logic::tribool result;
      http_request_t hs_req;
      boost::tie(result,boost::tuples::ignore)=request_parser().parse(
        hs_req,incoming_data,
        incoming_data+bytes_transferred);

      std::string str=reduce(incoming_data,bytes_transferred);

      std::stringstream ss;
      ss<<"bytes_transferred="<<bytes_transferred<<" "
        <<" bytes=\""<<str<<"\""
        <<" error=\""<<err_msg<<"\""
        <<" result="<<std::boolalpha<<result;

      log_trace(file,line,func,obj,ss.str());
    }
  };

} //namespace splice {

#endif //#ifdef SPLICE_LOG_TRACE

#endif // #ifndef WEBSOCKET_SESSION_BOOST_LOG_HPP
