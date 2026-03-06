#pragma once
#include <vector>

namespace utils {

template<typename IdType>
class IdManager {
 public:
    IdManager() = default;
    IdManager() = default;

    IdType NewId();

    void FreeId(IdType id);

 private:
    IdType current_id_;
    std::vector<IdType> available_ids_;
};

} // namespace utils

#include "id_manager.tcc"
