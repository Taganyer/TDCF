//
// Created by taganyer on 25-7-16.
//
#include <tdcf/base/Errors.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;


void DBTAgent::init(const IdentityPtr& from_id, const MetaData& meta, Handle& handle) {
    create_agent_data(from_id, handle);
    auto& [t1, t2, red, black, leaf1] = handle.agent_data<DBTAgentData>();
    if (!leaf1) {
        if (red) handle.connect(red);
        if (black) handle.connect(black);
    } else {
        auto id = handle.accept();
        TDCF_CHECK_EXPR(id->equal_to(*t1))
    }
    if (leaf1) {
        if (red && !red->equal_to(*t1)) handle.connect(red);
        if (black && !black->equal_to(*t1)) handle.connect(black);
    } else {
        if ((!red || !red->equal_to(*t2)) && (!black || !black->equal_to(*t2))) {
            auto id = handle.accept();
            TDCF_CHECK_EXPR(id->equal_to(*t2))
        }
    }
}

SerializableType DBTAgent::derived_type() const {
    return ClusterType::dbt;
}

void DBTAgent::create_agent_data(const IdentityPtr& from_id, Handle& handle) {
    bool end = false;
    bool is_leaf_node_in_t1 = false;
    std::vector<IdentityPtr> ids;
    std::vector<int> colors;
    while (!end) {
        StatusFlag flag = handle.get_communicator_events();
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
            is_leaf_node_in_t1 = event.meta.data1[0] == 1;
            ids.emplace_back(std::move(event.id));
            colors.push_back(event.meta.data1[1]);
        }
    }
    assert(ids.size() == 4);

    int red_index, black_index;
    if (colors[2] == 0) {
        assert(colors[3] == 1);
        red_index = 2;
        black_index = 3;
    } else {
        assert(colors[3] == 0);
        red_index = 3;
        black_index = 2;
    }

    handle.connect(ids[0]);
    if (!ids[0]->equal_to(*ids[1])) {
        handle.connect(ids[1]);
    }
    handle.create_agent_data<DBTAgentData>(std::move(ids[0]), std::move(ids[1]),
                                           std::move(ids[red_index]), std::move(ids[black_index]),
                                           is_leaf_node_in_t1);
}

StatusFlag DBTAgent::handle_disconnect(const IdentityPtr& id, Handle& handle) {
    auto& [t1, t2, red, black, leaf1] = handle.agent_data<DBTAgentData>();
    handle.disconnect(id);
    if (t1->equal_to(*id)) t1 = nullptr;
    if (t2->equal_to(*id)) t2 = nullptr;
    if (red) {
        handle.disconnect(red);
        red = nullptr;
    }
    if (black) {
        handle.disconnect(black);
        black = nullptr;
    }
    if (t1 || t2) return StatusFlag::Success;
    return StatusFlag::ClusterOffline;
}

StatusFlag DBTAgent::create_progress(uint32_t version, const MetaData& meta,
                                     ProcessingRulesPtr& rule, Handle& handle) {
    switch (meta.operation_type) {
        // case OperationType::Broadcast:
        //     return Broadcast::create(version, meta, rule, handle);
        // case OperationType::Scatter:
        //     return Scatter::create(version, meta, rule, handle);
        // case OperationType::Reduce:
        //     return Reduce::create(version, meta, rule, handle);
        // case OperationType::AllReduce:
        //     return AllReduce::create(version, meta, rule, handle);
        // case OperationType::ReduceScatter:
        //     return ReduceScatter::create(version, meta, rule, handle);
        default:
            TDCF_RAISE_ERROR(error OperationType)
    }
}
