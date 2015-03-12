#include <splice/serialization_session.hpp>


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
    using namespace std;

    const auto id=msg->get_id();
    switch(id)
    {
    case message_identification::client_echo_timed:
    {
      using namespace std;

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

