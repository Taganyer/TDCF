//
// Created by taganyer on 25-5-18.
//
#pragma once

#include "Serializable.hpp"

namespace tdcf {

    class MetaData : public Serializable {
    public:
        MetaData() = default;

        ~MetaData() override = default;

    };

    using MetaDataPtr = std::shared_ptr<MetaData>;

    class Data {
    public:
        Data() = default;

        virtual ~Data() = default;

    };

    using DataPtr = std::shared_ptr<Data>;

}
