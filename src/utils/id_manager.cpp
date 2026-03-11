#include "id_manager.hpp"

template<typename IdType>
utils::IdManager<IdType>::IdManager(IdType starting_id)
    : current_id_(starting_id) {}
