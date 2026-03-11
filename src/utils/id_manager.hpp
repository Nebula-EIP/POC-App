#pragma once
#include <vector>

namespace utils {

/**
 * @brief Utility class wich generates unique ids
 */
template<typename IdType>
class IdManager {
 public:
    IdManager(IdType starting_id = 0) = default;
    ~IdManager() = default;

    /**
     * @brief Generates a new id, increments internal counter
     * 
     * @returns New unique id of type IdType
     */
    IdType NewId();

    /**
     * @brief Get the currently highest free id
     * 
     * @warning The id returned by this method may be returned by NewId
     * 
     * @return Highest available id of type IdType
     */
    IdType current_id() const noexcept;

    /**
     * @brief Frees an id, allowing it to be given by NewId again
     */
    void FreeId(IdType id);

 private:
    IdType current_id_;
    std::vector<IdType> available_ids_;
};

} // namespace utils

#include "id_manager.tcc"
