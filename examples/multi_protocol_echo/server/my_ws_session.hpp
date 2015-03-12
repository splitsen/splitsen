#include <splice/web_socket/ws_session.hpp>

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

  void on_read(std::string& json_msg)
  {
    using namespace std;
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

