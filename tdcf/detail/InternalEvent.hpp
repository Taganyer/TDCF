//
// Created by taganyer on 25-5-26.
//
#pragma once

#include <memory>
#include <tdcf/detail/CommandMark.hpp>
#include <tdcf/frame/Processor.hpp>

namespace tdcf {

    enum class EventType {
        HTCBroadcast,
        HTCScatter,
        HTCReduce,
        HTCAllGather,
        HTCAllReduce,
        HTCReduceScatter,
        HTCAllToAll,

        CTNBroadcast,
        CTNScatter,
        CTNReduce,
        CTNAllGather,
        CTNAllReduce,
        CTNReduceScatter,
        CTNAllToAll,

        CTCBroadcast,
        CTCScatter,
        CTCReduce,
        CTCAllGather,
        CTCAllReduce,
        CTCReduceScatter,
        CTCAllToAll,
    };

    struct InternalEvent {
        explicit InternalEvent(EventType type) : type(type) {};

        virtual ~InternalEvent() = default;

        EventType type;

        [[nodiscard]] bool is_HTC() const;

        [[nodiscard]] bool is_CTN() const;

        [[nodiscard]] bool is_CTC() const;

    };

    using InternalEventPtr = std::unique_ptr<InternalEvent>;

    struct HTCEvent : InternalEvent {
        ProcessingRulesPtr rule;

        explicit HTCEvent(EventType t, ProcessingRulesPtr rp) :
            InternalEvent(t), rule(std::move(rp)) {};
    };

    using HTCEventPtr = std::unique_ptr<HTCEvent>;

    struct CTNEvent : InternalEvent {
        ProcessingRulesPtr rule;

        explicit CTNEvent(EventType t, ProcessingRulesPtr rp) :
            InternalEvent(t), rule(std::move(rp)) {};
    };

    using CTNEventPtr = std::unique_ptr<CTNEvent>;

    struct CTCEvent : CTNEvent {
        CommandMark old;

        explicit CTCEvent(EventType t, ProcessingRulesPtr rp, const CommandMark& cm) :
            CTNEvent(t, std::move(rp)), old(cm) {};
    };

    using CTCEventPtr = std::unique_ptr<CTCEvent>;


    struct HTCBroadcast : HTCEvent {
        explicit HTCBroadcast(ProcessingRulesPtr rp) :
            HTCEvent(EventType::HTCBroadcast, std::move(rp)) {};

        DataPtr data_ptr;
        bool got_data = false;
        unsigned sent_data = 0;
    };

    struct HTCScatter : HTCEvent {
        explicit HTCScatter(ProcessingRulesPtr rp) :
            HTCEvent(EventType::HTCScatter, std::move(rp)) {};

        DataPtr data_ptr;
        Processor::DataSet data_set;
        bool got_data = false, got_dataset = false;
        unsigned sent_data = 0;
    };

    struct HTCReduce : HTCEvent {
        explicit HTCReduce(ProcessingRulesPtr rp) :
            HTCEvent(EventType::HTCReduce, std::move(rp)) {};

        Processor::DataSet data_set;
        DataPtr data_ptr;
        unsigned received_data = 0;
        bool store_data = false;
    };

    struct HTCAllGather : HTCEvent {
        explicit HTCAllGather(ProcessingRulesPtr rp) :
            HTCEvent(EventType::HTCAllGather, std::move(rp)) {};

        Processor::DataSet data_set;
        unsigned sent_data1 = 0, received_data = 0, sent_data2 = 0, store_data2 = 0;
    };

    struct HTCAllReduce : HTCEvent {
        explicit HTCAllReduce(ProcessingRulesPtr rp) :
            HTCEvent(EventType::HTCAllReduce, std::move(rp)) {};

        Processor::DataSet data_set;
        DataPtr data_ptr;
        unsigned sent_data1 = 0, received_data = 0, sent_data2 = 0;
        bool store_data = false;
    };

    struct HTCReduceScatter : HTCEvent {
        explicit HTCReduceScatter(ProcessingRulesPtr rp) :
            HTCEvent(EventType::HTCReduceScatter, std::move(rp)) {};

        Processor::DataSet data_set1;
        DataPtr data_ptr;
        Processor::DataSet data_set2;
        unsigned sent_data1 = 0, received_data = 0;
        unsigned reduce = 0, sent_data2 = 0;
        bool store_data = false;
    };

    struct HTCAllToAll : HTCEvent {
        explicit HTCAllToAll(ProcessingRulesPtr rp) :
            HTCEvent(EventType::HTCAllToAll, std::move(rp)) {};

        std::vector<Processor::DataSet> data_sets;
        unsigned got_data = 0, sent_data1 = 0, received_data = 0, sent_data2 = 0;
    };

    struct CTNBroadcast : CTNEvent {
        explicit CTNBroadcast(ProcessingRulesPtr rp) :
            CTNEvent(EventType::CTNBroadcast, std::move(rp)) {};

        DataPtr data_ptr;
        bool got_data = false, store_data = false;
    };

    struct CTNScatter : CTNEvent {
        explicit CTNScatter(ProcessingRulesPtr rp) :
            CTNEvent(EventType::CTNScatter, std::move(rp)) {};

        DataPtr data_ptr;
        bool got_data = false, store_data = false;
    };

    struct CTNReduce : CTNEvent {
        explicit CTNReduce(ProcessingRulesPtr rp) :
            CTNEvent(EventType::CTNReduce, std::move(rp)) {};

        DataPtr data_ptr;
        bool got_data = false, sent_data = false;
    };

    struct CTNAllGather : CTNEvent {
        explicit CTNAllGather(ProcessingRulesPtr rp) :
            CTNEvent(EventType::CTNAllGather, std::move(rp)) {};

        DataPtr data_ptr1;
        bool got_data1 = false, sent_data1 = false;
        Processor::DataSet data_set;
        unsigned got_data2 = 0;
    };

    struct CTNAllReduce : CTNEvent {
        explicit CTNAllReduce(ProcessingRulesPtr rp) :
            CTNEvent(EventType::CTNAllReduce, std::move(rp)) {};

        DataPtr data_ptr1, data_ptr2;
        bool got_data1 = false, sent_data1 = false;
        bool got_data2 = false, store_data2 = false;
    };

    struct CTNReduceScatter : CTNEvent {
        explicit CTNReduceScatter(ProcessingRulesPtr rp) :
            CTNEvent(EventType::CTNReduceScatter, std::move(rp)) {};

        Processor::DataSet data_set;
        DataPtr data_ptr;
        unsigned got_data1 = 0, sent_data1 = 0;
        bool got_data2 = false, store_data2 = false;
    };

    struct CTNAllToAll : CTNEvent {
        explicit CTNAllToAll(ProcessingRulesPtr rp) :
            CTNEvent(EventType::CTNAllToAll, std::move(rp)) {};

        Processor::DataSet data_set1, data_set2;
        unsigned got_data1 = 0, sent_data1 = 0;
        unsigned got_data2 = 0, store_data2 = 0;
    };

    struct CTCBroadcast : CTCEvent {
        explicit CTCBroadcast(ProcessingRulesPtr rp, const CommandMark& cm) :
            CTCEvent(EventType::CTCBroadcast, std::move(rp), cm) {};

        DataPtr data_ptr;
        bool got_data = false;
        unsigned sent_data = false;
    };

    struct CTCScatter : CTCEvent {
        explicit CTCScatter(ProcessingRulesPtr rp, const CommandMark& cm) :
            CTCEvent(EventType::CTCScatter, std::move(rp), cm) {};

        DataPtr data_ptr;
        bool got_data = false;
        unsigned sent_data = 0;
    };

    struct CTCReduce : CTCEvent {
        explicit CTCReduce(ProcessingRulesPtr rp, const CommandMark& cm) :
            CTCEvent(EventType::CTCReduce, std::move(rp), cm) {};

        Processor::DataSet data_set;
        DataPtr data_ptr;
        unsigned got_data = 0;
        bool reduce_data = false, sent_data = false;
    };

    struct CTCAllGather : CTCEvent {
        explicit CTCAllGather(ProcessingRulesPtr rp, const CommandMark& cm) :
            CTCEvent(EventType::CTCAllGather, std::move(rp), cm) {};

        Processor::DataSet data_set1;
        unsigned got_data1 = 0, sent_data1 = 0;
        Processor::DataSet data_set2;
        unsigned got_data2 = 0, sent_data2 = 0, store_data2 = 0;
    };

    struct CTCAllReduce : CTCEvent {
        explicit CTCAllReduce(ProcessingRulesPtr rp, const CommandMark& cm) :
            CTCEvent(EventType::CTCAllReduce, std::move(rp), cm) {};

        Processor::DataSet data_set;
        DataPtr data_ptr;
        unsigned got_data1 = 0;
        bool reduce_data1 = false, sent_data1 = false, got_data2 = false, store_data2 = false;
        unsigned sent_data2 = 0;
    };

    struct CTCReduceScatter : CTCEvent {
        explicit CTCReduceScatter(ProcessingRulesPtr rp, const CommandMark& cm) :
            CTCEvent(EventType::CTCReduceScatter, std::move(rp), cm) {};

        Processor::DataSet data_set1, data_set2;
        DataPtr data_ptr;
        unsigned got_data1 = 0;
        bool reduce_data1 = false, sent_data1 = false, got_data2 = false, store_data2 = false;
        unsigned sent_data2 = 0;
    };

    struct CTCAllToAll : CTCEvent {
        explicit CTCAllToAll(ProcessingRulesPtr rp, const CommandMark& cm) :
            CTCEvent(EventType::CTCAllToAll, std::move(rp), cm) {};

        Processor::DataSet data_set1, data_set2;
        unsigned got_data1 = 0, sent_data1 = 0;
        unsigned got_data2 = 0, sent_data2 = 0, store_data2 = 0;
    };

}
