#include <splice/tcp_session.hpp>

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

  void on_read_string(std::string& msg)
  {
    using namespace std;

    cout<<hex<<cast_up()<<
      "->my_tcp_session::on_read_string( "<<msg<<" )"<<endl;
    async_write(msg); // write back to client
  }
};
