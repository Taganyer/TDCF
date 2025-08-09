## Processor

### 功能简介

    用于实际的数据获取和处理。
    采用了异步架构设计，处理的结果和数据的获取不会马上要求返回，框架会在合适的时机调用 get_events 获得结果。
    底层数据获取和处理由用户自行定义。

### 注意

- **获取到的数据集 DataPtr 数目越多，框架发送的并发性可以得到改善，数目太多同样会影响性能。**

### Processor 接口实现要求

##### ```void store(const ProcessingRulesPtr& rule_ptr, const DataPtr& data_ptr)```

- 储存一个数据，ProcessingRulesPtr 来源于根节点，不同过程有不同的 ProcessingRulesPtr。

##### ```void acquire(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr)```

- 获取一个过程所需要的数据集，ProcessingRulesPtr 来源于根节点，不同过程有不同的 ProcessingRulesPtr。
    - 获取的结果是一个数据集，不能为空。

#### ```void reduce(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr, DataSet target)```

- 对一个数据集进行归约操作，ProcessingRulesPtr 来源于根节点，不同过程有不同的 ProcessingRulesPtr。
    - 归约的结果是一个数据集，不能为空。

#### ```scatter(ProcessorEventMark mark, const ProcessingRulesPtr& rule_ptr, uint32_t scatter_size, DataSet dataset)```

- 对一个数据集进行发散操作，ProcessingRulesPtr 来源于根节点，不同过程有不同的 ProcessingRulesPtr。
    - 假设结果集的大小为 n。
        - **n / scatter_size > 0 && n % scatter_size == 0**
        - **每 n / scatter_size 个连续的数据，会作为一个集合会被发送到一个子节点上。**

#### ```OperationFlag get_events(EventQueue& queue)```

- 当处理事件完成时，将事件加入到 EventQueue 中，注意**同一个 ProcessingRulesPtr 请求顺序要与结果顺序一致**。
- 函数可以设置为阻塞或非阻塞。
- 根据实际情况返回 OperationFlag。
    - Success：成功。
    - FurtherWaiting：由于某种原因此刻不能完成操作，需要进一步等待。
    - Error：发生内部错误。

## ProcessorEvent

### 功能简介

    主要用于表明一个 Processor 事件，用户需根据实际情况对其进行填充，框架会通过 get_events(EventQueue& queue)
    进行获取。

#### 分类

- Null：无效事件，不应该出现。
- Acquire：作为```store```结果的回应。
    - ```ProcessorEventMark mark```与函数传递的标识一致，不能进行任何修改。
    - ```DataSet result```是操作的结果集。
- Reduce：作为```reduce```结果的回应。
    - ```ProcessorEventMark mark```与函数传递的标识一致，不能进行任何修改。
    - ```DataSet result```是操作的结果集。
- Scatter：作为```scatter```结果的回应。
    - ```ProcessorEventMark mark```与函数传递的标识一致，不能进行任何修改。
    - ```DataSet result```是操作的结果集。