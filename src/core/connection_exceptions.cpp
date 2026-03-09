#include "connection_exceptions.hpp"

// ConnectionException
core::ConnectionException::ConnectionException(
    const std::string &err_msg, const std::source_location &location)
    : CoreException(err_msg, location) {}

// InvalidPinIndexException
core::InvalidPinIndexException::InvalidPinIndexException(
    const std::string &err_msg, const std::source_location &location)
    : ConnectionException(err_msg, location) {}

// PinNotConnectedException
core::PinNotConnectedException::PinNotConnectedException(
    const std::string &err_msg, const std::source_location &location)
    : ConnectionException(err_msg, location) {}

// IncompatiblePinTypesException
core::IncompatiblePinTypesException::IncompatiblePinTypesException(
    const std::string &err_msg, const std::source_location &location)
    : ConnectionException(err_msg, location) {}

// PinAlreadyConnectedException
core::PinAlreadyConnectedException::PinAlreadyConnectedException(
    const std::string &err_msg, const std::source_location &location)
    : ConnectionException(err_msg, location) {}

// SelfConnectionException
core::SelfConnectionException::SelfConnectionException(
    const std::string &err_msg, const std::source_location &location)
    : ConnectionException(err_msg, location) {}
