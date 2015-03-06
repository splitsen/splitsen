
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MY_LOGGER_HPP
#define MY_LOGGER_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

#include <splice/logger/log_interface.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp> 
#include <boost/thread/lock_guard.hpp> 
#include <boost/function.hpp>

#include <fstream>

struct my_logger: splice::log_interface<my_logger>
{
  static boost::mutex mtx_;
  static std::ofstream ofs_;

  using log_signal_t=boost::function<void (const std::string&)>;
  static log_signal_t signal_;

  void track(char severity,const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg);

  // release version handle only severity messages >= info
#ifdef NDEBUG 
   // is release
    void log_trace_t(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg,const char* begin,size_t size)
    {}

    void log_debug_t(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg,const char* begin,size_t size)
    {}
#else
  void log_trace(const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg)
  {
    track('T',file,line,func,obj,msg);
  }

  void log_debug(const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg)
  {
    track('D',file,line,func,obj,msg);
  }
#endif

  void log_info(const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg)
  {
    track('I',file,line,func,obj,msg);
  }

  void log_warning(const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg)
  {
    track('W',file,line,func,obj,msg);
  }

  void log_error(const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg)
  {
    track('E',file,line,func,obj,msg);
  }

  void log_fatal(const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg)
  {
    track('F',file,line,func,obj,msg);
  }
};

#endif //MY_LOGGER_HPP
