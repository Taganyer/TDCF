//
// Created by taganyer on 25-5-23.
//
#pragma once

namespace tdcf {

#define STATUS_FLAG_ITEM(MOD) \
    MOD(Success), \
    MOD(Timeout), \
    MOD(FurtherWaiting), \
    MOD(FurtherWaitingAll), \
    MOD(Conflict), \
    MOD(TargetNotFound), \
    MOD(Transition), \
    MOD(ClusterOffline),

#define ENUM_MOD(item) item

#define NAME_MOD(item) #item

    enum class StatusFlag {
        STATUS_FLAG_ITEM(ENUM_MOD)
    };

    constexpr const char* status_flag_name(StatusFlag status) {
        constexpr const char *item_names[] = {
            STATUS_FLAG_ITEM(NAME_MOD)
        };
        return item_names[static_cast<int>(status)];
    };

#undef STATUS_FLAG_ITEM

#undef ENUM_MOD

#undef NAME_MOD

}
