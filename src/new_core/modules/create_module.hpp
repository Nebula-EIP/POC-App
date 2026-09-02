#include "module.hpp"

extern "C" {

/**
 * @brief Create a new module instance.
 *
 * @return A pointer to the new module, or nullptr if creation failed.
 */
core::IModule *CreateModule();
}
