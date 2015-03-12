#include <splice/web_socket/ws_session.hpp>

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

