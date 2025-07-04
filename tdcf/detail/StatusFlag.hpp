//
// Created by taganyer on 25-5-23.
//
#pragma once

#include <tdcf/base/Marcos.hpp>

namespace tdcf {

#define STATUS_FLAG_ITEM(MOD) \
    MOD(Success), \
    MOD(EventEnd), \
    MOD(CommunicatorSendMessageFurtherWaiting), \
    MOD(CommunicatorSendMessageError), \
    MOD(CommunicatorGetEventsFurtherWaiting), \
    MOD(CommunicatorGetEventsError), \
    MOD(ProcessorGetEventsFurtherWaiting), \
    MOD(ProcessorGetEventsError), \
    MOD(ClusterOffline),

    enum class StatusFlag {
        STATUS_FLAG_ITEM(TDCF_ENUM_MOD)
    };

    constexpr const char* status_flag_name(StatusFlag status) {
        constexpr const char *item_names[] = {
            STATUS_FLAG_ITEM(TDCF_NAME_MOD)
        };
        return item_names[static_cast<int>(status)];
    };

#undef STATUS_FLAG_ITEM

}
