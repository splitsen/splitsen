
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <utility>
#include <set>


#ifdef SPLICE_SEPARATE_COMPILATION
# include <splice/src.hpp>
#endif

#include <splice/server.hpp>
#include <splice/web_socket/ws_session.hpp>
#include <splice/serialization_session.hpp>
#include <splice/http/http_session.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

#include "echo_message.hpp"
#include "my_logger.hpp"

#ifdef NDEBUG 
# pragma message( "release version" )
#else
# pragma message( "debug version" )
#endif

using namespace std;

// serves index.html
class my_http_session:public splice::http_session<my_http_session,my_logger>
{
public:
  using base_t=splice::http_session<my_http_session,my_logger>;

  // Copy required files from src folder to dest.
  // Setup host name and port number for html files
  static std::pair<boost::system::error_code,bool> html_preprocess
    (const string& src
    ,const string& dest
    ,const string& value_of_hostName
    ,const string& value_of_portNumber
    )
  {
    using namespace std;
    using namespace boost::filesystem;

    boost::system::error_code ec;

    if((!exists(src,ec)||!is_directory(src,ec))
      ||(!exists(dest,ec)||!is_directory(dest,ec)))
      return make_pair(ec,false);

    if(equivalent(path(src),path(dest),ec))
      return make_pair(ec,false);

    const std::set<path> required=
    {"index.html"
    ,"json_client_multiprotocol.html"
    ,"admin_dashboard.html"
    ,"splice_top_horz.png"
    };

    for(auto& it:required)
    {
      if(!is_regular_file(path(src)/it,ec)||ec)
        return make_pair(ec,false);
    }

    for(auto& it:required)
    {
      path from=path(src)/it;
      path to=path(dest)/it;

      copy_file(from,to,copy_option::overwrite_if_exists,ec);
      if(ec)
        return make_pair(ec,false);

      if(it.extension()!=".html")
        continue;

      auto size=file_size(to,ec);
      if(ec)
        return make_pair(ec,false);

      // setup host name and port number
      std::string buffer(boost::numeric_cast<size_t>(size),0);
      {
        boost::filesystem::ifstream(to).read(&buffer[0],size);
      }

      const pair<const string,const string> fr[]=
      {make_pair("value_of_hostName"
      ,string("\"")+value_of_hostName+string("\""))
      ,make_pair("value_of_portNumber",value_of_portNumber)
      };

      for(const auto &it:fr) // access by const reference
      {
        auto i=buffer.find(it.first);
        if(i==string::npos)
          return make_pair(ec,false);
        buffer.replace(i,it.first.length(),it.second.c_str());
      }
      boost::filesystem::ofstream(to).write
        (buffer.c_str(),buffer.length());
    }

    return make_pair(ec,true);
  }

  my_http_session(splice::socket_t& socket
    ,http::server::request_handler& request_handler)
    :base_t(socket,request_handler)
  {
  }

protected:
  friend class my_server;
  friend base_t;
};

// communicates with mpt_serialize_client console application
class my_serialization_session:public splice::serialization_session
  <my_serialization_session,client_server_protocol,my_logger>
{
public:
  using base_t=splice::serialization_session
    <my_serialization_session,client_server_protocol,my_logger>;

  my_serialization_session(splice::socket_t& socket)
    :base_t(socket)
  {
  }

protected:
  friend class my_server;
  friend class base_t;
  friend class base_t::base_t;
  friend class base_t::base_t::base_t;

  bool try_handshake(const splice::hand_shake_data_t& incoming)
  {
    // the same GUID must be sent from client
    const char* guid="{40CBD5AC-856A-4EA5-AE00-DFB199ABED64}";
    return (strlen(guid)==incoming.size()
      &&memcmp(guid,incoming.data(),incoming.size())==0);
  }

  void on_read(msg_ptr msg)
  {
    const auto id=msg->get_id();
    switch(id)
    {
    case message_identification::client_echo_timed:
    {
      BOOST_ASSERT(dynamic_cast<client_echo_timed_t*>(msg.get()));
      const client_echo_timed_t& tmp=static_cast<const client_echo_timed_t&>(*msg);
      cout<<hex<<cast_up()<<
        "->my_serialization_session::on_read( "<<tmp.msg_<<" )"<<endl;

      // send back
      async_write(boost::make_shared<server_echo_timed_t>(tmp));
    }
    break;
    default:
      cerr<<"unknown protocol message id:"<<
        boost::lexical_cast<std::string>(id)<<endl;
      // Here is a choice, if you want or not to continue speaking
      // with a client sending wrong message(s).
      // get_io_service().stop(); // kindly kill server
      // shutdown(); // close the socket
      BOOST_ASSERT(false); // ignore that wrong message
    }
  }
};

// broadcast log messages
class my_log_session:public splice::ws_session<my_log_session,my_logger>
{
public:
  using base_t=splice::ws_session<my_log_session,my_logger>;

  my_log_session(splice::socket_t& socket)
    :base_t(socket)
  {
  }

  using connect_signal_t=boost::function<void(sp_up_t&,bool)>;
  void on_connect_sig(connect_signal_t f)
  {
    signal_=f;
  }

  void send(const std::string& msg)
  {
    namespace ba=boost::asio;
    using namespace splice;

    // Take care of not generating log message,
    // otherwise will loop indefinitely
    data_frame_ptr df(mk_data_frame(msg+"<br>"));
    ba::async_write(get_socket(),df->to_buffers(),
      get_strand().wrap(boost::bind(
      &my_log_session::on_write_quiet,shared_from_this(),
      df,
      ba::placeholders::error)));
  }

protected:
  friend class my_server;
  friend class base_t;
  friend class base_t::base_t;
  friend class base_t::base_t::base_t;

  void on_write_quiet(
    splice::data_frame_ptr df,
    const boost::system::error_code& error)
  {
    if(error)
      cast_up()->on_error(EZ_FLF);
  }

  bool try_handshake(const splice::hand_shake_data_t& incoming)
  {
    // the same GUID must be sent from client
    const char* guid="{82F0D331-F6C1-4227-8015-C89276E1931B}";
    return (strlen(guid)==incoming.size()
      &&memcmp(guid,incoming.data(),incoming.size())==0);
  }

  void on_handshake_success()
  {
    BOOST_ASSERT(!signal_.empty());
    signal_(shared_from_this(),true);
  }

  void on_shutdown(
    const boost::system::error_code& shut_down_ec,
    const boost::system::error_code& close_ec)
  {
    base_t::on_shutdown(shut_down_ec,close_ec);
    BOOST_ASSERT(!signal_.empty());
    signal_(shared_from_this(),false);
  }


private:
  connect_signal_t signal_;
};

// communicates with echo_client_multiprotocol.html with json messages
class my_ws_session:public splice::ws_session<my_ws_session,my_logger>
{
public:
  using base_t=splice::ws_session<my_ws_session,my_logger>;

  my_ws_session(boost::asio::io_service& io_service)
    :base_t(io_service)
  {
  }

protected:
  friend class my_server;
  friend class base_t;
  friend class base_t::base_t;

  bool try_handshake(const splice::hand_shake_data_t& incoming)
  {
    // the same GUID must be sent from client
    const char* guid="{FB3D4012-F196-4299-9531-0AEACBB3ED89}";
    return (strlen(guid)==incoming.size()
      &&memcmp(guid,incoming.data(),incoming.size())==0);
  }

  void on_read(string& json_msg)
  {
    using namespace boost::property_tree;

    cout<<hex<<cast_up()<<
      "--------------> my_ws_session receive \""<<json_msg<<"\""<<endl;

    try
    {
      ptree pt;
      {
        stringstream ss(json_msg);
        read_json(ss,pt);
      }

      string answer;
      switch(client_server_protocol::get_id(pt))
      {
      case message_identification::client_echo_timed:
        answer=server_echo_timed_t(client_echo_timed_t(pt)).json();
        break;
      default:
        //cerr<<"unknown protocol message id:"<<msgId<<endl;
        // Here is a choice, if you want or not to continue speaking
        // with a client sending wrong message(s).
        // get_io_service().stop(); // kindly kill server
        // shutdosn(); // close the socket
        return; // ignore that wrong message
      }

      async_write(answer);
    }
    catch(ptree_error& err)
    {
      // same issue handling that in default case above
      cerr<<__FILE__<<"@"<<__LINE__<<":"<<
        "property tree exception:"<<err.what()<<endl;
    }

  }
};

// communicates with mpt_raw_client console application
class my_tcp_session:public splice::tcp_session<my_tcp_session,my_logger>
{
public:
  using base_t=splice::tcp_session<my_tcp_session,my_logger>;

  my_tcp_session(splice::socket_t& socket)
    :base_t(socket)
  {
  }

protected:
  friend class my_server;
  friend base_t;

  bool try_handshake(const splice::hand_shake_data_t& incoming)
  {
    // the same GUID must be sent from client
    const char* guid="{21BB758F-BC6A-42C4-92B1-6B5332CEAEBF}";
    return (strlen(guid)==incoming.size()
      &&memcmp(guid,incoming.data(),incoming.size())==0);
  }

  void on_handshake_success()
  {
    async_read_string();
  }

  void on_read_string(string& msg)
  {
    cout<<hex<<cast_up()<<
      "->my_tcp_session::on_read_string( "<<msg<<" )"<<endl;
    async_write(msg); // write back to client
  }
};

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
    // Check command line arguments.
    const auto cla=argc!=4;
    if(cla)
    {
      cerr<<"Usage: server <address> <port> <doc_root>\n";
      cerr<<"\tFor IPv4, try:\n";
      cerr<<"\t\tserver (0.0.0.0|localhost) 7777\n";
      cerr<<"\tFor IPv6, try:\n";
      cerr<<"\t\tserver 0::0 7777\n";
      if(!getenv("SPLICE_ROOT"))
      {
        cerr<<"SPLICE_ROOT environment variable must be defined."<<endl;
        return 1;
      }
    }

    const string server_name=cla?"0.0.0.0":argv[1];
    const string server_port=cla?"7777":argv[2];
    const string src=cla?(string(getenv("SPLICE_ROOT"))+"/examples/html_clients"):argv[3];
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