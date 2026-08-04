# Nebula Doxygen Documentation Guide

This document defines the minimum documentation required for the **Nebula** C++ project.

All documentation must be written in English and use Doxygen comments.

## 1. Files

Every `.cpp` and `.hpp` file must start with a header containing:

* a short description of the file;
* the name of the original author;
* the creation date;
* the name of the last contributor who modified the file;
* the date of the last modification.

```cpp
/**
 * @file Connection.cpp
 * @brief Implements the Nebula connection system.
 *
 * @author Created by [github name]
 * @date Created on DD-MM-YYYY
 *
 * @author Last modified by [github name]
 * @date Last modified on DD-MM-YYYY
 */
```

The contributor modifying the file must update the last modified name and date.


## 2. Functions

Every function must document:

- what it does;
- its parameters;
- whether each parameter is required or optional;
- its return value;
- the exceptions that may be thrown.

```cpp
/**
 * @brief Opens a connection to a Nebula server.
 *
 * @param host Server hostname. Required.
 * @param port Server port. Required.
 * @param timeout Connection timeout in milliseconds. Optional.
 *
 * @return A valid connection object.
 *
 * @throws std::invalid_argument If the host is empty or the port is invalid.
 * @throws ConnectionException If the connection cannot be opened.
 */
Connection openConnection(
    const std::string& host,
    unsigned int port,
    std::chrono::milliseconds timeout = std::chrono::seconds(5)
);
```

When a function does not return a value, `@return` is not required.

```cpp
/**
 * @brief Closes the connection.
 *
 * @throws ConnectionException If the connection cannot be closed correctly.
 */
void closeConnection();
```

## 3. Classes

Every class must have a short description.

```cpp
/**
 * @brief Manages a connection to a Nebula server.
 */
class Connection
{
public:
    /**
     * @brief Sends data to the connected server.
     *
     * @param data Data to send. Required.
     *
     * @return The number of bytes sent.
     *
     * @throws ConnectionException If the connection is closed.
     */
    std::size_t send(const std::vector<std::byte>& data);
};
```

## 4. Variables

Document variables only when their purpose is not obvious.

Class members should be documented when they represent an important state or value.

```cpp
class Connection
{
private:
    std::string host_;   ///< Address of the connected server.
    unsigned int port_;  ///< Port of the connected server.
    bool connected_;     ///< Indicates whether the connection is active.
};
```

Constants should also have a short description.

```cpp
/// Default connection timeout in milliseconds.
constexpr unsigned int DEFAULT_TIMEOUT_MS = 5000;
```

## 5. Required Doxygen Tags

Use the following tags:

- `@file` for the file name;
- `@brief` for a short description;
- `@author` for the original author / last contributor;
- `@date` for creation / last modification dates;
- `@param` for function parameters;
- `@return` for the returned value;
- `@throws` for exceptions;
- `///<` for short variable or class member descriptions.

## 6. Final Rule

Documentation must stay short.

It should explain how to use the code without repeating what is already obvious from the implementation.
