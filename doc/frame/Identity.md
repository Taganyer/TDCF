## Identity

### 功能简介

    该类是为了标识一个节点的身份，
    要求该类有序列化功能，能被 Communicator 进行发送和接收。

### Identity 接口实现要求

##### ```virtual Uid guid()```

- **标识 Identity 全局唯一的 id，整个集群（包括子集群）不能重复。**

##### ```uint32_t serialize_size()```

- 标识数据被序列化所需的缓冲区大小，用户必须实现。

##### ```bool serialize(void *buffer, uint32_t buffer_size)```

- 将数据序列化到 buffer 指定的缓冲区中，当 buffer_size 指定的缓冲区大小小于```serialize_size()```的返回值时，不进行序列化，返回
  false。

##### ```bool deserialize(const void *buffer, uint32_t buffer_size)```

- 从缓冲区的数据进行反序列化，当 buffer_size 指定的缓冲区大小小于```serialize_size()```的返回值时，不进行反序列化，返回
  false。
