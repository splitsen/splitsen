
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <splice/tcp_session.hpp>

#include <boost/asio/connect.hpp>

using namespace std;

struct my_logger: splice::log_interface<my_logger>
{
  void track(char severity,const char* file,unsigned line,
    const char* func,void* obj,const std::string& msg)
  {
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

using boost::asio::ip::tcp;

class my_session: public  splice::tcp_session<my_session,my_logger>
{
  enum { max_length=1024 };
public:
  typedef splice::tcp_session<my_session,my_logger> base_t;

  my_session(splice::socket_t& socket)
    :base_t(socket)
  {
  }

  static size_t run(const std::string& address
    ,const std::string& port,boost::system::error_code& ec)
  {
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);

    auto r=resolver.resolve({address.c_str(),port.c_str()},ec);
    if(ec)
      return 0;
    splice::socket_t socket(io_service);
    boost::asio::connect(socket,r,ec);
    if(ec)
      return 0;

    // socket connected, create a my_session object then 
    // launch the 'io_service handlers pump'
    io_service.post(boost::bind(&my_session::type_in,
      boost::shared_ptr<my_session>(
        boost::make_shared<my_session>(socket))));

    return io_service.run(ec);
  }

  void type_in()
  {
    char request[max_length];
    cout<<"Enter message: ";
    cin.getline(request,max_length);
    size_t request_length=strlen(request);

    async_write(std::string(request,request_length));
  }

  void on_write(boost::shared_ptr<std::string> msg,const error_code& error)
  {
    if(!error)
      cout<<"Succesfully sent"<<endl;
    else
      cout<<"Failed to send: "<<error.message()<<endl;

    // No more handler to be handled by io_service,
    // so io_service.run(ec) will exit
  }

};

int main(int argc,char* argv[])
{
  using namespace std;

  try
  {
    // Check command line arguments.
    if(argc!=2)
    {
      cerr<<"Usage: mpe_raw_client <address> <port>\n";
      cerr<<"       default to \'localhost 7777\'\n";
    }

    string address=argc!=3?"localhost":argv[1];
    string port=argc!=3?"7777":argv[2];

    cout<<"Server address: "<<address<<endl;
    cout<<"Server port: "<<port<<endl;

    boost::system::error_code ec;
    size_t count=my_session::run(address,port,ec);

    cout<<"session->run( returns:"<<
      count<<" and ec:"<<ec.message()<<endl;
  }
  catch(std::exception& e)
  {
    std::cerr<<"Exception: "<<e.what()<<"\n";
  }

  return 0;
}