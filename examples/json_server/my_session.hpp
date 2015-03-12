#include <splice/web_socket/ws_session.hpp>

#ifndef SPLICE_NO_LOG
#include <splice/logger/ws_session_boost_log.hpp>
#endif

struct msg_question_echo_timed;
struct msg_question_files_current_directory;

class my_session : public splice::ws_session 
#ifndef SPLICE_NO_LOG
    <my_session,splice::ws_session_boost_log>
#else
    <my_session>
#endif
{
public:
#ifndef SPLICE_NO_LOG
    using base_t=splice::ws_session<my_session, ws_session_boost_log>;
#else
    using base_t=splice::ws_session<my_session>;
#endif

    my_session(boost::asio::io_service& io_service);

    std::string on_message(const msg_question_echo_timed& msg);

    std::string on_message(const msg_question_files_current_directory& json_msg);

    void on_read(const std::string& json_msg);

    void on_connect();

    void on_shutdown(
      const boost::system::error_code& shut_down_ec,
      const boost::system::error_code& close_ec);
};
