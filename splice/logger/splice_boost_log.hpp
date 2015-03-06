
//          Copyright Jean Davy 2014-2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPLICE_LOG_HPP
#define SPLICE_LOG_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/attribute_value_set.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/counter.hpp>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

extern boost::log::sources::severity_logger<
  boost::log::trivial::severity_level > splice_log;

extern boost::shared_ptr <
  boost::log::sinks::synchronous_sink <
  boost::log::sinks::text_file_backend > > splice_log_sink;

#define SPLICE_LOG( lvl, obj )    BOOST_LOG_SEV(splice_log, lvl) << "[" << std::hex << obj << "]" << std::hex 

#define SPLICE_LOG_TRACE( obj )      SPLICE_LOG(boost::log::trivial::trace, obj) 
#define SPLICE_LOG_DEBUG( obj )      SPLICE_LOG(boost::log::trivial::debug, obj)
#define SPLICE_LOG_INFO( obj )       SPLICE_LOG(boost::log::trivial::info, obj)
#define SPLICE_LOG_WARNING( obj )    SPLICE_LOG(boost::log::trivial::warning, obj)
#define SPLICE_LOG_ERROR( obj )      SPLICE_LOG(boost::log::trivial::error, obj)
#define SPLICE_LOG_FATAL( obj )      SPLICE_LOG(boost::log::trivial::fatal, obj)

#define SPLICE_LOG_FLUSH     splice_log_sink->flush()

#endif // #ifndef SPLICE_LOG_HPP
