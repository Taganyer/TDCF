## Data

### 功能简介

    该类是为了标识实体的数据，框架不会对其进行访问，底层细节由用户自行实现。
    要求该类有序列化功能，能被 Communicator 进行发送和接收，能被 Processor 进行处理。
    Data 并没有设计成虚基类，可以被实例化，此时它的 derived_type 将会为0，需要用户实现发送接受功能，但它不会被交给 Processor 处理。

### Communicator 接口实现要求

##### ```SerializableType derived_type()```

- 用来标识继承的子类类型，用户必须实现，返回值不能为 0。

##### ```uint32_t serialize_size()```

- 标识数据被序列化所需的缓冲区大小，用户必须实现。

##### ```bool serialize(void *buffer, uint32_t buffer_size)```

- 将数据序列化到 buffer 指定的缓冲区中，当 buffer_size 指定的缓冲区大小小于```serialize_size()```的返回值时，不进行序列化，返回
  false。

##### ```bool deserialize(const void *buffer, uint32_t buffer_size)```

- 从缓冲区的数据进行反序列化，当 buffer_size 指定的缓冲区大小小于```serialize_size()```的返回值时，不进行反序列化，返回
  false。
