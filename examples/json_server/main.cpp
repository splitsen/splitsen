#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "my_server.hpp"

using namespace std;
using namespace splice;

#ifndef SPLICE_NO_LOG

// global logging variable
boost::log::sources::severity_logger< boost::log::trivial::severity_level > splice_log;
boost::shared_ptr<
  boost::log::sinks::synchronous_sink< boost::log::sinks::text_file_backend >>
  splice_log_sink;

void log_init()
{
  namespace logging=boost::log;
  namespace src=boost::log::sources;
  namespace sinks=boost::log::sinks;
  namespace keywords=boost::log::keywords;
  namespace expr=boost::log::expressions;

  // Thanks to:
  // http://stackoverflow.com/questions/18014335/boost-log-severity-logger-init-from-stream?lq=1
  logging::register_simple_formatter_factory<boost::log::trivial::severity_level,char>("Severity");
  logging::register_simple_filter_factory<boost::log::trivial::severity_level,char>("Severity");

  splice_log_sink=logging::add_file_log
    (
    keywords::file_name="json_server_%N.log",// file name pattern
    keywords::rotation_size=10*1024*1024, // rotate files every 10 MiB...
    keywords::time_based_rotation= // ...or at midnight
    sinks::file::rotation_at_time_point(0,0,0),
    keywords::format= // log record format
    "[%LineID%][%ThreadID%][%TimeStamp%][%Severity%]%Message%"
    );
  splice_log_sink->locked_backend()->auto_flush(true);

  // log everything
  logging::core::get()->set_filter(
    logging::trivial::severity>=logging::trivial::trace);

  boost::log::add_common_attributes();

  cout<<"Logging to "<<
    boost::filesystem::current_path().append("json_server_0.log").make_preferred().string()
    <<endl;
}
#endif // #ifndef SPLICE_NO_LOG

#if defined( BOOST_MSVC )
#pragma message( "-----------------" )
#pragma message( "std lib version: "	    BOOST_STDLIB  )
#pragma message( "C++ version: "		    BOOST_STRINGIZE(__cplusplus )  )
#pragma message( "Boost version: "		    BOOST_LIB_VERSION  )
#pragma message( "Compiler: "			    BOOST_COMPILER  )
#pragma message( "Platform: "			    BOOST_PLATFORM  )
#ifdef _DEBUG
#pragma message( "Is Debug" )
#else
#pragma message( "Is Release" )
#endif
#pragma message( "-----------------" )
#elif defined( __GNUG__ )
#pragma message "-----------------" 
#pragma message "std lib version: "		    BOOST_STDLIB  
#pragma message "C++ version: "			    BOOST_STRINGIZE(__cplusplus )  
#pragma message "Boost version: "		    BOOST_LIB_VERSION  
#pragma message "Compiler: "			    BOOST_COMPILER  
#pragma message "Platform: "			    BOOST_PLATFORM  
#ifdef _DEBUG
#pragma message "Is Debug"
#else
#pragma message "Is Release"
#endif
#pragma message "-----------------" 
#else
#error unsupported compiler
#endif

int main(int argc,char* argv[])
{
#ifndef SPLICE_NO_LOG
  log_init();

  using namespace std;
  using namespace boost::posix_time;
  using namespace boost::gregorian;

  stringstream ss;
  ss<<"Launched: json_server at "<<to_simple_string(second_clock::local_time())<<endl
    <<"working directory: "<<boost::filesystem::current_path()<<endl;
  for(int i=0; i<argc; i++)
    ss<<"argv["<<i<<"]="<<argv[i]<<endl;
  ss<<"std lib version: "<<BOOST_STDLIB<<endl
    <<"C++ version: "<<BOOST_STRINGIZE(__cplusplus)<<endl
    <<"Boost version: "<<BOOST_LIB_VERSION<<endl
    <<"Compiler: "<<BOOST_COMPILER<<endl
    <<"Platform: "<<BOOST_PLATFORM<<endl
#ifdef _DEBUG
    <<"Is Debug"<<endl
#else
    << "Is Release" << endl
#endif
    <<"Build: "<<__DATE__<<" "<<__TIME__<<endl
    <<"BOOST_VERSION="<<BOOST_VERSION<<endl
    <<"1st[] is index value."<<
    " 2nd[] is thread."<<
    " 3rd[] is date."<<
    " 4th[] is level."<<
    " 5th[] is this."<<
    " 6th[] is file name @ line number."<<
    " 7th[] is function name"<<
    " 8th[] is class address";
  SPLICE_LOG_DEBUG(0)<<ss.str();
  SPLICE_LOG_FLUSH;
#endif // #ifndef SPLICE_NO_LOG

  try
  {
    // Check command line arguments.
    if(argc!=3)
    {
      cerr<<"Usage: server <address> <port>\n";
      cerr<<"  For IPv4, try:\n";
      cerr<<"    server 0.0.0.0 7777\n";
      cerr<<"  For IPv6, try:\n";
      cerr<<"    server 0::0 7777\n";

      //return 1;
    }

    string server_name=argc!=3?"localhost":argv[1];
    string server_port=argc!=3?"7777":argv[2];

    cout<<"Server address: "<<server_name<<endl;
    cout<<"Server port: "<<server_port<<endl;
    cout<<"Server is starting..."<<endl;

    // Initialise the server.
    my_server s(server_name,server_port);

    // Run the server until stopped.
    s.run();
  }
  catch(exception& e)
  {
    cerr<<"exception: "<<e.what()<<"\n";
  }

  return 0;
}