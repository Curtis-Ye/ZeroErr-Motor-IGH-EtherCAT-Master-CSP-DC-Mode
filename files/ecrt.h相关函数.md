# IGH相关函数
##  ecrt_request_master()
用于请求主站，即创建主站。  
 ## ecrt_master_slave_config()
 创建一个从站，返回一个ec_slave_config_t类型对象。  
 ## ecrt_slave_config_pdos()
 用于检查从站PDO映射是否被正确配置。  
## ecrt_domain_reg_pdo_entry_list()
用于将前面注册的域栏目与创建的域进行关联。  
## ecrt_master_application_time()
把“主站认为当前应该是多少时间”告诉 EtherCAT 网络。  
## ecrt_slave_config_dc()
表示启用DC、启用SYNC0。  
## ecrt_master_activate()
启动主站并判断主站是否启动。  
## ecrt_domain_data()
用于返回指向domain1的第一个对象的指针。  
## ecrt_master_receive()
用于从网卡接收 EtherCAT 从站返回的数据，并更新到主站内存。  
## ecrt_slave_config_state()
查询从站配置对象（Slave Configuration）的状态信息。  
## ecrt_domain_queue()
用于把 Domain 中修改过的 PDO 数据加入到下一次发送的 EtherCAT 报文中。  
## ecrt_master_sync_reference_clock()
用于同步参考从站，，告诉参考从站其时间应该等于PC时间。  
## ecrt_master_sync_slave_clocks()
同步其他从站。  
## ecrt_master_send()
发送EtherCAT报文,真正把之前准备好的报文发出去。  
## ecrt_master_reference_clock_time()
该函数是 IGH EtherCAT 主站提供的一个读取参考时钟（Reference Clock）当前时间值的函数。  
## EC_READ_S32()
从 PDO 过程数据区读取一个 32 位有符号整数（int32_t）。  
## EC_READ_U16()
从 PDO 过程数据区读取一个 16 位无符号整数（uint16_t）。  
## EC_WRITE_U16()
向 PDO 过程数据区写入一个 16 位无符号整数（uint16_t）。  
