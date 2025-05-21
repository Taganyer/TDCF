//
// Created by taganyer on 25-5-15.
//
#pragma once

#include "Serializable.hpp"

namespace tdcf {

    class Identity : public Serializable {};

    using IdentityPtr = std::shared_ptr<Identity>;

}
