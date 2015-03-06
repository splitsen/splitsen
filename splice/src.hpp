
// By default, Splice is a header-only library. However, you may prefer to build
// Splice using separately compiled source code. To do this, add 
// #include <splice/src.hpp> to one (and only one) source file in a 
// program, then build the program with SPLICE_SEPARATE_COMPILATION defined 
// in the project/compiler settings.

#if defined(SPLICE_HEADER_ONLY)
# error Do not compile Splice library source with SPLICE_HEADER_ONLY defined
#endif

#include "http/http_session.hxx"
#include "http/mime_types.hxx"
#include "http/reply.hxx"
#include "http/request_handler.hxx"
#include "http/request_parser.hxx"
#include "tcp_session.hxx"
#include "web_socket/ws_session.hxx"
#include "web_socket/ws_handshake.hxx"
#include "serialization_session.hxx"
#include "serialization_engine.hxx"
#include "ez_server.hxx"
#include "protocol.hxx"
#include "mono_protocol.hxx"
#include "multi_protocol.hxx"

