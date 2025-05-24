//
// Created by taganyer on 25-5-24.
//
#pragma once

namespace tdcf {

    class NoCopy {
    public:
        NoCopy(const NoCopy&) = delete;

        NoCopy& operator=(const NoCopy&) = delete;

    protected:
        NoCopy() = default;

        ~NoCopy() = default;

    };

}
