#pragma once
#include <vector>

#include "id_manager.hpp"

namespace utils {

template<typename IdType>
IdType IdManager::NewId() {
   if (available_ids_.empty()) {
      return current_id_++;
   } else {
      IdType id = available_ids_.back();
      available_ids_.pop_back();
      return id;
   }
}

template<typename IdType>
void IdManager::FreeId(IdType id) {
   available_ids_.push_back(id);
}

} // namespace utils
