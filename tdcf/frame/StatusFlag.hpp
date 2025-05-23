//
// Created by taganyer on 25-5-23.
//
#pragma once

namespace tdcf {

    enum class StatusFlag {
        Success,
        ErrorType,
        FurtherWaiting,
        Conflict,
        TargetNotFound,
        Transition,
        ClusterOffline,
    };

    template <auto value>
    constexpr auto enum_name() {
        std::string_view name;
#if __GNUC__ || __clang__
        name = __PRETTY_FUNCTION__;
        std::size_t start = name.find('=') + 2;
        std::size_t end = name.size() - 1;
        name = std::string_view { name.data() + start, end - start };
        start = name.rfind("::");
#elif _MSC_VER
        name = __FUNCSIG__;
        std::size_t start = name.find('<') + 1;
        std::size_t end = name.rfind(">(");
        name = std::string_view{ name.data() + start, end - start };
        start = name.rfind("::");
#endif
        return start == std::string_view::npos ? name : std::string_view {
                       name.data() + start + 2, name.size() - start - 2
                   };
    }

    const char* status_flag_name(StatusFlag status);

}
