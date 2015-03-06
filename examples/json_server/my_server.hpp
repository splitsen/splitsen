#include "my_session.hpp"
#include <splice/server.hpp>

#ifndef SPLICE_NO_LOG
#include <splice/logger/boost_log_interface.hpp>
#endif

class my_server: public
#ifndef SPLICE_NO_LOG
  ez_server_mono<my_server,splice::boost_log_interface>
#else
  ez_server_mono<my_server>
#endif
{
public:
#ifndef SPLICE_NO_LOG
  typedef ez_server_mono<my_server,splice::boost_log_interface>
    base_t;
#else
  typedef ez_server_mono<my_server>     base_t;
#endif
  typedef boost::shared_ptr<my_session>    session_ptr;

  my_server(const std::string& address,const std::string& port);

  session_ptr construct_session();

  void on_run(size_t thread_count);

  void on_error(const boost::system::error_code& ec);
};

