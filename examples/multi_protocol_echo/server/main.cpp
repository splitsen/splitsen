
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifdef SPLICE_SEPARATE_COMPILATION
# include <splice/src.hpp>
#endif

#include <splice/server.hpp>
#include <splice/web_socket/ws_session.hpp>
#include <splice/serialization_session.hpp>
#include <splice/http/http_session.hpp>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

#include "echo_message.hpp"
#include "my_http_session.hpp"
#include "my_serialization_session.hpp"
#include "my_log_session.hpp"
#include "my_ws_session.hpp"
#include "my_tcp_session.hpp"

#ifdef NDEBUG 
# pragma message( "release version" )
#else
# pragma message( "debug version" )
#endif

using namespace std;

my_logger::log_signal_t my_logger::signal_;

class my_server: public ez_server_multi_prot<my_server,5,my_logger>
{
public:
  using base_t=ez_server_multi_prot<my_server,5,my_logger>;

  my_server(const string& address,const string& port,const string& doc_root)
    :base_t(address,port)
    ,request_handler_(doc_root)
  {
    // bind logger to my own member function
    my_logger::signal_=boost::bind(&my_server::on_logger,this,_1);

    start_accept<my_ws_session>();
  }

private:
  friend class base_t;
  friend class base_t::base_t;
  using sp_log_session=boost::shared_ptr<my_log_session>;

  void on_logger(const std::string& msg)
  {
    log_session_.broadcast(msg);
  }

  void on_connect(const sp_log_session& obj,bool connect)
  {
    log_session_.insert(obj,connect);
  }

  boost::shared_ptr<my_ws_session> construct_session()
  {
    // The class built here is the first one doing 
    // handshake on a http_session request.
    // It does NOT appear in do_next_handshake function below
    return boost::make_shared<my_ws_session>(get_io_service());
  }

  template< typename _t>
  void do_next_handshake(
    _t handshake_fail_handler,
    splice::socket_t& socket,
    unsigned protocol_index, // TODO better an enum !
    splice::hand_shake_data_t& incoming_data)
  {
    // The class with highest index is built from construct_session,
    // that is the first one which is tested and so do not need to 
    // be built here.
    BOOST_ASSERT(protocol_index<get_protocol_count()-1);
    auto tag=incoming_data.tag_;

    switch(protocol_index)
    {
    case 0:
      boost::make_shared<my_tcp_session>(socket)->
        handshake(handshake_fail_handler,incoming_data);
      break;
    case 1:
      boost::make_shared<my_serialization_session>(socket)->
        handshake(handshake_fail_handler,incoming_data);
      break;
    case 2:
    {
      auto obj=boost::make_shared<my_log_session>(socket);
      obj->on_connect_sig(boost::bind(&my_server::on_connect,this,_1,_2));
      obj->handshake(handshake_fail_handler,incoming_data);
    }
    break;
    case 3:
      boost::make_shared<my_http_session>(socket,request_handler_)->
        handshake(handshake_fail_handler,incoming_data);
      break;
    default:
      BOOST_ASSERT(false);
    }
  }

private:
  http::server::request_handler request_handler_;
  struct
  {
    void clear()
    {
      boost::lock_guard<boost::mutex> lck(log_session_set_mtx_);
      log_session_set_.clear();
    }

    void insert(const sp_log_session& obj,bool insert)
    {
      boost::lock_guard<boost::mutex> lck(log_session_set_mtx_);
      if(insert)
        log_session_set_.insert(obj);
      else // remove
      {
        auto it=log_session_set_.find(obj);
        if(it!=log_session_set_.end())
          log_session_set_.erase(it);
      }
    }

    void broadcast(const std::string& msg)
    {
      boost::lock_guard<boost::mutex> lck(log_session_set_mtx_);
      for(auto it:log_session_set_)
        it->get_io_service().post(boost::bind(&my_log_session::send,it,msg));
    }

  private:
    std::set<boost::shared_ptr<my_log_session>> log_session_set_;
    boost::mutex log_session_set_mtx_;
  }log_session_;
};

int main(int argc,char* argv[])
{
  try
  {
#if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4996) // secure stuff
#endif
    const char* splice_root=getenv("SPLICE_ROOT");
#if defined(BOOST_MSVC)
# pragma warning(pop)
#endif

    // Check command line arguments.
    const auto cla=argc!=4;
    if(cla)
    {
      cerr<<"Usage: server <address> <port> <doc_root>\n";
      cerr<<"\tFor IPv4, try:\n";
      cerr<<"\t\tserver (0.0.0.0|localhost) 7777\n";
      cerr<<"\tFor IPv6, try:\n";
      cerr<<"\t\tserver 0::0 7777\n";
      if(!splice_root)
      {
        cerr<<"SPLICE_ROOT environment variable must be defined."<<endl;
        return 1;
      }
    }

    const string server_name=cla?"0.0.0.0":argv[1];
    const string server_port=cla?"7777":argv[2];
    const string src=cla?(string(splice_root)+"/examples/html_clients"):argv[3];
    boost::system::error_code ec;
    const string dest=boost::filesystem::current_path(ec).string();
    if(ec)
    {
      cerr<<"Unable to preprocess html file(s), error:"<<ec.message()<<endl;
      return 1;
    }

    auto rc=my_http_session::html_preprocess(src,dest
      ,server_name=="0.0.0.0"?"localhost":server_name // TODO OK with Chrome, test with others
      ,server_port);

    if(!rc.second)
    {
      cerr<<"Unable to preprocess html file(s), error:"
        <<rc.first.message()<<endl;
      return 1;
    }

    cout<<"Server address: "<<server_name<<endl;
    cout<<"Server port: "<<server_port<<endl;
    cout<<"Server html source folder: "<<src<<endl;
    cout<<"http address:\"http://"
      <<(server_name=="0.0.0.0"?"localhost":server_name)
      <<":"<<server_port<<endl;

    // Initialize the server.
    my_server s(server_name,server_port,dest);

    cout<<"Server has started."<<endl;
    cout<<"Press Ctrl+C (Ctrl+Break) to exit."<<endl<<endl;

    // Run the server until stopped.
    s.run();
  }
  catch(exception& e)
  {
    cerr<<"exception: "<<e.what()<<"\n";
  }

  return 0;
}