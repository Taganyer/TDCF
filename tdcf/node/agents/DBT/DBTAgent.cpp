//
// Created by taganyer on 25-7-16.
//
#include <tdcf/base/Errors.hpp>
#include <tdcf/node/agents/DBT/DBTAgent.hpp>

using namespace tdcf;


void DBTAgent::init(const IdentityPtr& from_id, const MetaData& meta, Handle& handle) {
    create_agent_data(from_id, handle);
    auto& [red, black, t1p, t2p, leaf1] = handle.agent_data<DBTAgentData>();
    if (!leaf1) {
        if (red) handle.connect(red);
        if (black) handle.connect(black);
    } else {
        auto id = handle.accept();
        TDCF_CHECK_EXPR(id->equal_to(*t1p))
    }
    if (leaf1) {
        if (red && !red->equal_to(*t1p)) handle.connect(red);
        if (black && !black->equal_to(*t1p)) handle.connect(black);
    } else {
        if ((!red || !red->equal_to(*t2p)) && (!black || !black->equal_to(*t2p))) {
            auto id = handle.accept();
            TDCF_CHECK_EXPR(id->equal_to(*t2p))
        }
    }
}

SerializableType DBTAgent::derived_type() const {
    return ClusterType::dbt;
}

void DBTAgent::create_agent_data(const IdentityPtr& from_id, Handle& handle) {
    bool end = false;
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
            } else {
                assert(event.meta.operation_type == OperationType::Init);
                ids.emplace_back(std::move(event.id));
                colors.push_back(event.meta.data1[0]);
            }
        }
    }
    assert(ids.size() == 6);
    bool is_leaf_node_in_t1 = !ids[1] && !ids[2];
    if (is_leaf_node_in_t1) {
        assert(ids[3] || ids[4]);
    } else {
        assert(!ids[3] && !ids[4]);
    }
    int red_index, black_index;
    if (is_leaf_node_in_t1) {
        if (colors[1] == 0) {
            red_index = 1;
            black_index = 2;
        } else {
            red_index = 2;
            black_index = 1;
        }
    } else {
        if (colors[4] == 0) {
            red_index = 4;
            black_index = 5;
        } else {
            red_index = 5;
            black_index = 4;
        }
    }
    handle.create_agent_data<DBTAgentData>(std::move(ids[red_index]), std::move(ids[black_index]),
                                           std::move(ids[0]), std::move(ids[3]), is_leaf_node_in_t1);
}

StatusFlag DBTAgent::handle_disconnect(const IdentityPtr& id, Handle& handle) {
    auto& [red, black, t1p, t2p, leaf1] = handle.agent_data<DBTAgentData>();
    TDCF_CHECK_EXPR(id->equal_to(*t1p))
    handle.disconnect(id);
    if (!leaf1) {
        if (red) handle.disconnect(red);
        if (black) handle.disconnect(black);
    }
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
