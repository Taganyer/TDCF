//
// Created by taganyer on 25-5-18.
//
#pragma once

namespace tdcf {

    class Data {
    public:
        Data() = default;

        virtual ~Data() = default;

    };

    using DataPtr = std::shared_ptr<Data>;

}
