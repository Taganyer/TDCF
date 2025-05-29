//
// Created by taganyer on 25-5-26.
//

#include <tdcf/detail/InternalEvent.hpp>

using namespace tdcf;

bool InternalEvent::is_HTC() const {
    return static_cast<int>(type) < static_cast<int>(EventType::CTNBroadcast);
}

bool InternalEvent::is_CTN() const {
    int t = static_cast<int>(type);
    return t < static_cast<int>(EventType::CTCBroadcast)
        && t > static_cast<int>(EventType::HTCAllToAll);
}

bool InternalEvent::is_CTC() const {
    return static_cast<int>(type) > static_cast<int>(EventType::CTNAllToAll);
}
