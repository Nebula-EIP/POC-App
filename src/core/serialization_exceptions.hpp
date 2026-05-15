#pragma once

#include "core_exceptions.hpp"

namespace core {

/**
 * @class SerializationException
 * @brief Base exception class for all serialization and file I/O errors.
 *
 * This exception is thrown when operations involving serialization,
 * deserialization, or file operations fail.
 */
class SerializationException : public CoreException {
   public:
    SerializationException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    SerializationException(const std::source_location &location,
                           std::format_string<Args...> fmt, Args &&...args);

    ~SerializationException() = default;
};

/**
 * @class DeserializationException
 * @brief Exception thrown when deserialization of graph or node data fails.
 *
 * This exception is thrown when attempting to deserialize corrupted data,
 * data with version mismatches, or invalid structure.
 */
class DeserializationException : public SerializationException {
   public:
    DeserializationException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    DeserializationException(const std::source_location &location,
                             std::format_string<Args...> fmt, Args &&...args);

    ~DeserializationException() = default;
};

/**
 * @class InvalidJsonSchemaException
 * @brief Exception thrown when JSON doesn't match the expected schema.
 *
 * This exception is thrown when JSON data is missing required fields,
 * has fields with wrong types, or doesn't conform to the .nebula format.
 */
class InvalidJsonSchemaException : public SerializationException {
   public:
    InvalidJsonSchemaException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    InvalidJsonSchemaException(const std::source_location &location,
                               std::format_string<Args...> fmt, Args &&...args);

    ~InvalidJsonSchemaException() = default;
};

/**
 * @class FileNotFoundException
 * @brief Exception thrown when attempting to load a file that doesn't exist.
 *
 * This exception is thrown when trying to load a graph from a file path
 * that doesn't exist or cannot be accessed.
 */
class FileNotFoundException : public SerializationException {
   public:
    FileNotFoundException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    FileNotFoundException(const std::source_location &location,
                          std::format_string<Args...> fmt, Args &&...args);

    ~FileNotFoundException() = default;
};

/**
 * @class FileAccessException
 * @brief Exception thrown when file I/O operations fail.
 *
 * This exception is thrown when file operations fail due to permissions,
 * disk errors, or other I/O issues during save or load operations.
 */
class FileAccessException : public SerializationException {
   public:
    FileAccessException(
        const std::string &err_msg,
        const std::source_location &location = std::source_location::current());

    template <typename... Args>
    FileAccessException(const std::source_location &location,
                        std::format_string<Args...> fmt, Args &&...args);

    ~FileAccessException() = default;
};

}  // namespace core

#include "serialization_exceptions.tcc"
