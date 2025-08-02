//
// Created by taganyer on 25-7-16.
//
#include <tdcf/base/Errors.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;

void DBTAgent::init(const IdentityPtr& from_id, const MetaData& meta, Handle& handle) {
    create_agent_data(from_id, handle);
    auto& info = handle.agent_data<DBTAgentData>();
    info.self_serial = meta.serial;

    if (info.internal1()) {
        connect(true, info.red(), handle);
        connect(true, info.black(), handle);
    }
    if (!_t1_connected) {
        auto id = handle.accept();
        TDCF_CHECK_EXPR(id->equal_to(*info.t1()))
        connect(false, info.t1(), handle);
    }

    if (info.internal2()) {
        connect(true, info.red(), handle);
        connect(true, info.black(), handle);
    }
    if (!_t2_connected) {
        auto id = handle.accept();
        TDCF_CHECK_EXPR(id->equal_to(*info.t2()))
        connect(false, info.t2(), handle);
    }
}

DBTAgent::DBTAgentData::DBTAgentData(IdentityPtr t1_parent, IdentityPtr t2_parent,
                                     IdentityPtr red_child, IdentityPtr black_child,
                                     bool is_leaf_node_in_t1,
                                     bool is_leaf_node_in_t2) :
    t1_parent(std::move(t1_parent)), t2_parent(std::move(t2_parent)),
    red_child(std::move(red_child)), black_child(std::move(black_child)),
    is_leaf_node_in_t1(is_leaf_node_in_t1), is_leaf_node_in_t2(is_leaf_node_in_t2) {}

bool DBTAgent::DBTAgentData::in_t1_red(uint32_t serial) const {
    if (red_serial == static_cast<uint32_t>(-1)) return false;
    if (black_serial == static_cast<uint32_t>(-1)) return true;
    if (serial < self_serial && red_serial < self_serial
    || serial > self_serial && red_serial > self_serial) {
        return true;
    }
    return false;
}

bool DBTAgent::DBTAgentData::in_t2_red(uint32_t serial) const {
    if (red_serial == static_cast<uint32_t>(-1)) return false;
    if (black_serial == static_cast<uint32_t>(-1)) return true;
    uint32_t other = (serial + cluster_size - 1) % cluster_size;
    uint32_t self = (self_serial + cluster_size - 1) % cluster_size;
    uint32_t red = (red_serial + cluster_size - 1) % cluster_size;
    if (other < self && red < self || other > self && red > self) {
        return true;
    }
    return false;
}

void DBTAgent::connect(bool connect, const IdentityPtr& id, Handle& handle) {
    if (!id) return;
    auto& info = handle.agent_data<DBTAgentData>();
    if (equal_to(id, info.t1()) && !_t1_connected) {
        if (connect) handle.connect(info.t1());
    } else if (equal_to(id, info.t2()) && !_t2_connected) {
        if (connect) handle.connect(info.t2());
    } else if (equal_to(id, info.red()) && !_red_connected) {
        if (connect) handle.connect(info.red());
    } else if (equal_to(id, info.black()) && !_black_connected) {
        if (connect) handle.connect(info.black());
    }
    if (equal_to(id, info.t1())) {
        _t1_connected = true;
    }
    if (equal_to(id, info.t2())) {
        _t2_connected = true;
    }
    if (equal_to(id, info.red())) {
        _red_connected = true;
    }
    if (equal_to(id, info.black())) {
        _black_connected = true;
    }
}

void DBTAgent::create_agent_data(const IdentityPtr& from_id, Handle& handle) {
    bool end = false;
    bool is_leaf_node_in_t1 = false, is_leaf_node_in_t2 = false;
    std::vector<IdentityPtr> ids;
    std::vector<int> colors;
    std::vector<uint32_t> serials;
    while (!end) {
        StatusFlag flag = handle.get_communicator_events();
        if (flag == StatusFlag::CommunicatorGetEventsFurtherWaiting)
            flag = StatusFlag::Success;
        TDCF_CHECK_SUCCESS(flag)
        Handle::MessageEvent event;
        while (handle.get_message(event)) {
            TDCF_CHECK_EXPR(event.id->equal_to(*from_id))
            if (event.type == CommunicatorEvent::DisconnectRequest) {
                end = true;
                handle.disconnect(event.id);
                break;
            }
            assert(event.meta.operation_type == OperationType::Init);
            is_leaf_node_in_t1 = event.meta.data1[0];
            is_leaf_node_in_t2 = event.meta.data1[1];
            ids.emplace_back(std::dynamic_pointer_cast<Identity>(std::get<SerializablePtr>(event.variant)));
            colors.push_back(event.meta.data1[2]);
            serials.push_back(event.meta.serial);
        }
    }
    while (ids.size() < 4) {
        ids.emplace_back(nullptr);
        colors.emplace_back(3);
        serials.emplace_back(static_cast<uint32_t>(-1));
    }

    int red_index, black_index;
    if (colors[2] == 0) {
        assert(colors[3] == 1 || colors[3] == 3);
        red_index = 2;
        black_index = 3;
    } else {
        assert(colors[3] == 0 || colors[3] == 3);
        red_index = 3;
        black_index = 2;
    }

    handle.create_agent_data<DBTAgentData>(std::move(ids[0]), std::move(ids[1]),
                                           std::move(ids[red_index]), std::move(ids[black_index]),
                                           is_leaf_node_in_t1, is_leaf_node_in_t2);
    auto& info = handle.agent_data<DBTAgentData>();
    info.cluster_size = serials[0];
    info.red_serial = serials[red_index];
    info.black_serial = serials[black_index];
}

void DBTAgent::disconnect(const IdentityPtr& id, Handle& handle) {
    if (!id) return;
    auto& info = handle.agent_data<DBTAgentData>();
    auto ptr = id;
    bool need_disconnect = false;
    if (equal_to(ptr, info.t1())) {
        info.t1() = nullptr;
        need_disconnect = true;
    }
    if (equal_to(ptr, info.t2())) {
        info.t2() = nullptr;
        need_disconnect = true;
    }
    if (equal_to(ptr, info.red())) {
        info.red() = nullptr;
        need_disconnect = true;
    }
    if (equal_to(ptr, info.black())) {
        info.black() = nullptr;
        need_disconnect = true;
    }
    if (need_disconnect) {
        handle.disconnect(ptr);
    }
}

StatusFlag DBTAgent::handle_disconnect(const IdentityPtr& id, Handle& handle) {
    auto& info = handle.agent_data<DBTAgentData>();

    if (equal_to(info.t1(), id)) {
        disconnect(info.t1(), handle);
        if (info.internal1()) {
            disconnect(info.red(), handle);
            disconnect(info.black(), handle);
        }
    }

    if (equal_to(info.t2(), id)) {
        disconnect(info.t2(), handle);
        if (info.internal2()) {
            disconnect(info.red(), handle);
            disconnect(info.black(), handle);
        }
    }

    if (info.t1() || info.t2() || info.red() || info.black()) {
        return StatusFlag::Success;
    }
    return StatusFlag::ClusterOffline;
}

StatusFlag DBTAgent::create_progress(uint32_t version, const MetaData& meta,
                                     ProcessingRulesPtr& rule, Handle& handle) {
    switch (meta.operation_type) {
        case OperationType::Broadcast:
            return Broadcast::create(version, meta, rule, handle);
        case OperationType::Scatter:
            return Scatter::create(version, meta, rule, handle);
        case OperationType::Reduce:
            return Reduce::create(version, meta, rule, handle);
        case OperationType::AllReduce:
            return AllReduce::create(version, meta, rule, handle);
        case OperationType::ReduceScatter:
            return ReduceScatter::create(version, meta, rule, handle);
        default:
            TDCF_RAISE_ERROR(error OperationType)
    }
}
