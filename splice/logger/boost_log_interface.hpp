
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_LOG_INTERFACE_HPP
#define BOOST_LOG_INTERFACE_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

#include <string>
#include <boost/lexical_cast.hpp>

#include "splice_boost_log.hpp"

#ifdef SPLICE_LOG_TRACE

#include "log_interface.hpp"

namespace splice
{

  struct boost_log_interface: log_interface<boost_log_interface>
  {
    void log_trace(const char* file,unsigned line,const char* func,
      void* obj,const std::string& msg)
    {
      SPLICE_LOG_TRACE(obj)
        <<"["<<extract_file_name(file)<<"@"<<line<<"]:"
        <<"["<<func<<"]"
        <<" msg=\""<<msg<<"\"";
      SPLICE_LOG_FLUSH;
    }

    void log_debug(const char* file,unsigned line,const char* func,
      void* obj,const std::string& msg)
    {
      SPLICE_LOG_DEBUG(obj)
        <<"["<<extract_file_name(file)<<"@"<<line<<"]:"
        <<"["<<func<<"]"
        <<" msg=\""<<msg<<"\"";
      SPLICE_LOG_FLUSH;
    }

    void log_info(const char* file,unsigned line,const char* func,
      void* obj,const std::string& msg)
    {
      SPLICE_LOG_INFO(obj)
        <<"["<<extract_file_name(file)<<"@"<<line<<"]:"
        <<"["<<func<<"]"
        <<" msg=\""<<msg<<"\"";
      SPLICE_LOG_FLUSH;
    }

    void log_warning(const char* file,unsigned line,const char* func,
      void* obj,const std::string& msg)
    {
      SPLICE_LOG_WARNING(obj)
        <<"["<<extract_file_name(file)<<"@"<<line<<"]:"
        <<"["<<func<<"]"<<"["<<std::hex<<obj<<std::dec<<"]"
        <<" msg=\""<<msg<<"\"";
      SPLICE_LOG_FLUSH;
    }

    void log_error(const char* file,unsigned line,const char* func,
      void* obj,const std::string& msg)
    {
      SPLICE_LOG_ERROR(obj)
        <<"["<<extract_file_name(file)<<"@"<<line<<"]:"
        <<" msg=\""<<msg<<"\"";
      SPLICE_LOG_FLUSH;
    }

    void log_fatal(const char* file,unsigned line,const char* func,
      void* obj,const std::string& msg)
    {
      SPLICE_LOG_FATAL(obj)
        <<"["<<extract_file_name(file)<<"@"<<line<<"]:"
        <<" msg=\""<<msg<<"\"";
      SPLICE_LOG_FLUSH;
    }
  };

} // namespace splice

#endif //#ifdef SPLICE_LOG_TRACE

#endif // BOOST_LOG_INTERFACE_HPP