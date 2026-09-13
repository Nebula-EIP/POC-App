#include "serialization_exceptions.hpp"

// SerializationException
core::SerializationException::SerializationException(
    const std::string &err_msg, const std::source_location &location)
    : CoreException(err_msg, location) {}

// DeserializationException
core::DeserializationException::DeserializationException(
    const std::string &err_msg, const std::source_location &location)
    : SerializationException(err_msg, location) {}

// InvalidJsonSchemaException
core::InvalidJsonSchemaException::InvalidJsonSchemaException(
    const std::string &err_msg, const std::source_location &location)
    : SerializationException(err_msg, location) {}

// FileNotFoundException
core::FileNotFoundException::FileNotFoundException(
    const std::string &err_msg, const std::source_location &location)
    : SerializationException(err_msg, location) {}

// FileAccessException
core::FileAccessException::FileAccessException(
    const std::string &err_msg, const std::source_location &location)
    : SerializationException(err_msg, location) {}
