
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef LOG_INTERFACE_HPP
#define LOG_INTERFACE_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

#if defined(_MSC_VER)
#ifdef BOOST_CURRENT_FUNCTION
#undef BOOST_CURRENT_FUNCTION
#define BOOST_CURRENT_FUNCTION __FUNCTION__
#endif
#endif // #if defined(_MSC_VER)

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/current_function.hpp> // BOOST_CURRENT_FUNCTION

namespace splice
{

  template < typename up_t>
  struct log_interface
  {
    // TODO implement template specialization based on compile time value
    // https://trello.com/c/HG0MsqER
    // 
    void log_trace(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg) {}

    void log_debug(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg) {}

    void log_info(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg) {}

    void log_warning(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg) {}

    void log_error(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg) {}

    void log_fatal(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg) {}

    void log_trace_t(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg,const char* begin,size_t size)
    {
      cast_up()->log_trace(extract_file_name(file),line,func,obj,msg+reduce(begin,size));
    }

    void log_debug_t(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg,const char* begin,size_t size)
    {
      cast_up()->log_debug(extract_file_name(file),line,func,obj,msg+reduce(begin,size));
    }

    void log_info_t(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg,const char* begin,size_t size)
    {
      using namespace std;
      cast_up()->log_info(extract_file_name(file),line,func,obj,
        msg+" size="+lcs(size)+reduce(begin,size));
    }

    void log_warning_t(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg,const char* begin,size_t size)
    {
      cast_up()->log_warning(extract_file_name(file),line,func,obj,msg+reduce(begin,size));
    }

    void log_error_t(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg,const char* begin,size_t size)
    {
      cast_up()->log_error(extract_file_name(file),line,func,obj,msg+reduce(begin,size));
    }

    void log_fatal_t(const char* file,unsigned line,
      const char* func,void* obj,const std::string& msg,const char* begin,size_t size)
    {
      cast_up()->log_fatal(extract_file_name(file),line,func,obj,msg+reduce(begin,size));
    }

    // utility functions
    template< typename _T>
    std::string lcs(_T v)
    {
      return boost::lexical_cast<std::string>(v);
    }

    std::string reduce(const char* begin,size_t size) const
    {
      using namespace std;

      auto end=begin;
      advance(end,size);
      stringstream ss;
      ss<<"\"";
      if(size<(2*10+5))
      {
        ss<<string(begin,end);
      }
      else
      {
        size_t i=0;
        for(auto it=begin; it!=end; it++,i++)
        {
          char c=*it;
          if(isprint(c))
            ss<<c;
          else
            ss<<"\\"<<(int)c;

          if(i==9)
          {
            ss<<" ... ";
            advance(begin,size-11);
            it=begin;
          }
        }
      }
      ss<<"\"";

      return ss.str();
    }

    const char* extract_file_name(const char* path)
    {
      size_t i=strlen(path)-1;
      for(i; i>=0; i--)
      {
        if(path[i]=='\\'||path[i]=='/'||path[i]==':')
          break;
      }
      return path+i+1;
    }

    const char* extract_func_name(const char* func)
    {
      size_t i = std::string(func).rfind("::");
      if(i==std::string::npos)
        return func;
      return func+i+2;
    }

    std::string file_func_name(
      const char severity
      ,const char* path,unsigned line
      ,const char*func
      ,void* obj)//assuming obj is this as in EZ_FLFT
    {
      using std::string;
      using boost::lexical_cast;

      std::stringstream ss;
      ss<<"|(0x"<<std::hex<<obj<<")"<<"->";

      return 
        lexical_cast<string>(severity)
        +string("|")
        +string(extract_file_name(path))
        +string("@")
        +lexical_cast<string>(line)
        +ss.str()
        +string(extract_func_name(func));
    }

    up_t* cast_up()
    {
      return static_cast<up_t*>(this);
    }
  };

  struct no_log:log_interface<no_log>
  {};


// logger helper macro 
#define EZ_FLFT  __FILE__,__LINE__,BOOST_CURRENT_FUNCTION,cast_up()
#define EZ_FLF   __FILE__,__LINE__,BOOST_CURRENT_FUNCTION

} // namespace splice

#endif // WEBSOCKET_SERVER_HPP}