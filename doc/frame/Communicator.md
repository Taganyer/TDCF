## Communicator

### 功能简介

    该类是为了实现各种数据的发送与接受，同时具有连接和断开 Identity 指定的对端 Communicator。
    底层数据之间的交互由用户自行定义。
    该组件将会与创建由 Identity 标识的 Communicator 多条不同的连接，与同一个 Identity 只会创建一条连接。
    会自动进行连接的创建与断开。

### Communicator 接口实现要求

##### ```bool connect(const IdentityPtr& target)```

- 主动连接指定 Identity，成功返回 true，发生错误返回 false。
- 该函数接口要求设计为阻塞的，只有成功连接或者发生错误才能返回。

##### ```IdentityPtr accept()```

- 被动接受一个连接，返回对端连接的 IdentityPtr，失败返回 nullptr。
- 该函数接口要求设计为阻塞的，只有成功连接或者发生错误才能返回。

#### ```bool disconnect(const IdentityPtr& target)```

- 断开与 target 的连接，主动调用的一方将阻塞至对端调用 disconnect 为止。
- 被动调用的一方通过指定 CommunicatorEvent Type 为 DisconnectRequest，由框架自动调用 disconnect。
- 如果被动调用方有未接受的消息，需要等到所有未接收的消息全部被接受之后再提供 DisconnectRequest。

#### ```send_message(const IdentityPtr& id, const Message& message, const SerializablePtr& data)```

- 向 id 发送 message 和 data，这里的 Message 和 SerializablePtr 是配对的，**Message 和 SerializablePtr
  的数据不应该被进行任何修改**。
- 根据实际状况返回 OperationFlag。
    - Success：成功。
    - FurtherWaiting：由于某种原因此刻不能完成操作，需要进一步等待。
    - Error：发生内部错误。
- 对于 Message 用户可以简单调用：
    - ```Message::serialize(void *buffer, uint32_t buffer_size)```（缓冲区充足返回 true，可通过
      ```Message::serialize_size()```获得缓冲区所需大小）。
- 对于 SerializablePtr 用户可以简单调用：
    - ```Serializable::serialize(void *buffer, uint32_t buffer_size)```（缓冲区充足返回 true，可通过
      ```Serializable::serialize_size()```获得缓冲区所需大小）。
- SerializablePtr data 可能为 nullptr，用户需保证发送到对端时的 data 同样也为 nullptr。

#### ```get_events(EventQueue& queue)```

- 当消息事件到来时，将事件加入到 EventQueue 中，注意**同一个 Identity 事件的发送顺序要与接受顺序一致**。
- 函数可以设置为阻塞或非阻塞。
- 根据实际情况返回 OperationFlag。
    - Success：成功。
    - FurtherWaiting：由于某种原因此刻不能完成操作，需要进一步等待。
    - Error：发生内部错误。

## CommunicatorEvent

### 功能简介

    主要用于表明一个 Communicator 事件，用户需根据实际情况对其进行填充，框架会通过 get_events(EventQueue& queue)
    进行获取。

#### 分类

- Null：无效事件，不应该出现。
- MessageSendable：当```send_message```因为某种原因需要等待时，通过该事件进行唤醒。
    - **如果不进行唤醒，框架不会再主动调用```send_message```。**
    - ```IdentityPtr id```用于指定被延迟对端消息 id，不可以为 nullptr。
- ReceivedMessage：表明接受到了对端发送的一条消息。
    - ```IdentityPtr id```用于指定对端消息 id。
    - ```MetaData meta```来源于对端发送时的 Message。
    - ```SerializablePtr data```来源于对端发送时的 SerializablePtr。
- DisconnectRequest：表明有一条对端连接需要关闭。
    - ```IdentityPtr id```用来表明对端 id。
    - **向框架传递该事件时一定要保证顺序在最后，确保前面的消息全部按顺序被传递给框架。**