
#include <iostream>

#include <splice/server.hpp>
#include <splice/web_socket/ws_session.hpp>

#include <boost/thread/locks.hpp> 
#include <boost/thread/lock_guard.hpp> 

using namespace std;

struct my_logger: splice::log_interface<my_logger>
{
  static boost::mutex mtx_;

  void track(char severity,const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg)
  {
    boost::lock_guard<boost::mutex> lck(mtx_);

    cout<<file_func_name(severity,file,line,func,obj)<<" "<<msg<<endl;
  }

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

boost::mutex my_logger::mtx_;

class my_session: public splice::ws_session<my_session,my_logger>
{
public:
  typedef splice::ws_session<my_session,my_logger>  base_t;

  my_session(boost::asio::io_service& io_service)
    :base_t(io_service)
  {
  }

  void on_read(const string&  df)
  {
    cout<<"echo "<<df<<endl;
    async_write(df);
  }
};


class my_server: public ez_server_mono<my_server,my_logger>
{
public:
  typedef ez_server_mono<my_server,my_logger>      base_t;
  typedef boost::shared_ptr<my_session>     session_ptr;

  my_server(const string& address,const string& port)
    :base_t(address,port)
  {
    start_accept<my_session>();
  }

  session_ptr construct_session()
  {
    return boost::make_shared<my_session>(get_io_service());
  }
};

int main(int argc,char* argv[])
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
  my_server server(server_name,server_port);

  cout<<"Server has started."<<endl;
  cout<<"Press Ctrl+C (Ctrl+Break) to exit."<<endl<<endl;

  // Run the server until stopped.
  server.run();

  return 0;
}