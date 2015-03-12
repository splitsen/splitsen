
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <splice/serialization_session.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/host_name.hpp>

#include "../server/echo_message.hpp"

using boost::asio::ip::tcp;

class my_session: public
  splice::serialization_session<my_session,client_server_protocol>
{
  enum { max_length=1024 };

public:
  using base_t=splice::serialization_session<my_session,client_server_protocol>;
  using msg_ptr=boost::shared_ptr<client_server_protocol>;

  my_session(splice::socket_t& socket)
    :base_t(socket)
  {
  }

  void on_socket_connected()
  {
    // by pass serialization layer to send GUID
    tcp_session<my_session>::async_write
      (std::string("{40CBD5AC-856A-4EA5-AE00-DFB199ABED64}"),&my_session::on_write_guid);
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

    io_service.post(boost::bind(&my_session::on_socket_connected,
      boost::shared_ptr<my_session>(
      boost::make_shared<my_session>(socket))));

    return io_service.run(ec);
  }

  void type_in()
  {
    char request[max_length];
    std::cout<<"Enter message: ";
    std::cin.getline(request,max_length);
    size_t request_length=strlen(request);

    client_echo_timed_t msg;
    msg.msg_=std::string(request,request_length);

    async_write(boost::make_shared<client_echo_timed_t>(msg));
  }

  void on_write_msg(const boost::system::error_code& ec,msg_ptr msg)
  {
    if(ec)
    {
      on_error_code(EZ_FLF,ec);
      return;
    }

    async_read();
  }

  void on_write_guid(boost::shared_ptr<std::string> msg,const error_code& ec)
  {
    if(ec)
    {
      cast_up()->on_error_code(EZ_FLF,ec);
      return;
    }

    // This completion handler can only be called while sending std::string,
    // so from on_socket_connected function
    BOOST_ASSERT(*msg=="{40CBD5AC-856A-4EA5-AE00-DFB199ABED64}");

    type_in();
  }

  void on_read(msg_ptr msg)
  {
    switch(msg->get_id())
    {
    case message_identification::server_echo_timed:
    {
      BOOST_ASSERT(dynamic_cast<server_echo_timed_t*>(msg.get()));
      const server_echo_timed_t& back=static_cast<const server_echo_timed_t&>(*msg);

      auto delta=back.UTC()-back.client_sent_;
      std::cout<<"Reply is: \""<<back.msg_<<"\" within "<<
        delta<<" milliseconds"<<std::endl;
    }
    break;
    default:
      BOOST_ASSERT(false);
    }
  }
};

extern void serialization_test();

int main(int argc,char* argv[])
{
  using namespace std;

  serialization_test();

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
  }
  catch(std::exception& e)
  {
    std::cerr<<"Exception: "<<e.what()<<"\n";
  }

  return 0;
}