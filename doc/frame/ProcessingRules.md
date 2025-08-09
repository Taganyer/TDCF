## ProcessingRules

### 功能简介

    用于标识一个操作的数据处理规则，也可以作为一个操作的额外信息标识。
    进行一个操作时，会被传递到整个集群（包括子集群）中，被用于每个节点的 Processor。
    要求该类有序列化功能，能被 Communicator 进行发送和接收。

### ProcessingRules 接口实现要求

##### ```void finish_callback()```

- **每一个节点在一个操作过程结束后（本节点不会再有关于这个操作的任何动作），会调用这个函数。**

##### ```uint32_t serialize_size()```

- 标识数据被序列化所需的缓冲区大小，用户必须实现。

##### ```bool serialize(void *buffer, uint32_t buffer_size)```

- 将数据序列化到 buffer 指定的缓冲区中，当 buffer_size 指定的缓冲区大小小于```serialize_size()```的返回值时，不进行序列化，返回
  false。

##### ```bool deserialize(const void *buffer, uint32_t buffer_size)```

- 从缓冲区的数据进行反序列化，当 buffer_size 指定的缓冲区大小小于```serialize_size()```的返回值时，不进行反序列化，返回
  false。
