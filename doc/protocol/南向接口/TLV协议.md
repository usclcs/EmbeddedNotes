[TOC]

####　TLV协议

##### 1、定义

​		TLV协议是一种通讯协议，一般将数据封装成TLV的形式，即Tag，Length，Value。协议就是指通信双方对数据传输控制的一种规定，规定了数据格式，同步方式，传送速度，传送步骤的问题作出统一的规定。可以理解为两个节点之间为了协同工作，协商一定的规则和约定。例如我们会规定字节序，各个字段类型等。我们常见的协议有，TCP（传输控制协议），IP（网际协议），UDP（用户数据报协议）。

​		上面所提到的这些协议是已经设定好了的，并且我们也不能改变这些协议。但我们自己定义的通讯协议就不一样了，在我们自己定义的通讯协议中，我们可以自己指定数据的封装形式，然后通过TCP协议传输到网络的另一端，另一端也会采取自定义的解析数据的形式来获得想要的数据，在这里，TLV协议就是这样一种自定义的封装数据的协议。

​		TLV是一种编码方式，由数据的类型Tag，数据的长度Length，数据的内容Value构成的一组数据报文。TLV将数据以T-L-V的形式编码为字节数组，其规定了一帧数据的前一个字节或前几个字节来表示数据类型，紧接者就是数据的长度，接着就是了数据的内容。

##### 2、应用示例

​		假设现在客户端需要向服务器发送温湿度数据，假设0X01表示温度，0X02表示湿度，那么现在数据就应该封装成这样：

```
0X01 0X04 0X13 0X12 0X02 0X04 0X36 0X37
```

​		如此一来，当服务器端收到0X01时，便是表示温度的报文，收到0X02时，便是表示湿度的报文。但这个协议还不够完善，假设在传输的过程中，出现了错误，导致服务器端收到的数据变成了0X01 0X13 0X12 0X02 0X04 0X36 0X37 即在传输过程中丢失了一个字节的数据，这时如果解析这串数据，就会将0X13这个值作为温度这个数据报文的长度，从而导致传输出现了错误。

​		所以现在这个TLV还是不完备的，为了解决这个问题，我们需要在每一帧数据的前面加上一个帧头，用来标识这一帧数据，例如，我们将0XFD作为这帧数据的帧头，那么，上面的数据就变成了这样:

```
0XFD 0X01 0X04 0X13 0X12 0XFD 0X02 0X04 0X36 0X37
```

现在，当我们再解析这串数据时，就会先找0XFD，一般以0XFD作为帧头，是tag值一般从0X01开始，一般不会到0XFD。但尽管我们将0XFD作为帧头，但并不排除数据内容中可能会出现值为0XFD的数据，比如发送的数据是这样，0xFD 0x01 0x05 0xFD 0x30（第一帧数据） 0xFD 0x02 0x05 0x47 0x33（第二帧数据），但收到的是这样，0x01 0x04 0xFD 0x30 0xFD 0x02 0x04 0x47 0x33 ，如果出现这种情况，那我们的数据解析也会全部都错乱了。这时，我们可能会想到要用两个字节的数据来作为包头，比如，我们可能会采用0XFD,0XFE用作包头，但即使这样，还是有可能性出现错误，如果报文中某个数据丢失了，或者某个数据在传送过程中其值改变了，都会导致整个报文的错乱。
为了解决由于各种可能性而导致的错误，在实际中，我们会使用TLV加包头并且再把包中所有字节的数据计算得到 一个独一无二的CRC校验和值加在这一帧数据的末尾来构成一帧完整的数据。具体一些就是下面这样：

| 报文头 | Tag   | Length  | Value | Crc16校验和 |
| ------ | ----- | ------- | ----- | ----------- |
| 1字节  | 1字节 | n+5字节 | n字节 | 2字节       |

​		网络中的另一端（比如服务器端）接收到这一帧报文后，也同样按照这样的格式来解析这一帧报文，然后采用同样的CRC算法来为接收到的数据计算一个CRC校验和值，再把计算得到的CRC值与收到的CRC值作比较，如果这两个CRC值是相同的，就说明收到的数据是正确的数据，反之，如果这两个CRC值不一样，就证明收到的数据是错误的，如此一来，便很好的解决了数据在传输过程中可能出现的各种错误而导致整个数据报文都错乱。



##### 3、代码示例

```c
/**
 * TLV协议装包过程代码
 * 以上的代码是将序列号(sn)，时间(time),温度（temper）三种类型的数据按照字节流封装在buf中，
 * 并且每一帧数据都有packet_head,Tag，length，value和CRC校验和。
 */
int packtlv(char *buf,int size,char *sn,struct tm *time_now,float *temper)
    {
        unsigned    short   crc16;
        int                 pack_len = 0;
        int                 data_len = 0;
        int                 data_len_sn = 0;
        int                 data_len_time = 0;
        int                 data_len_temper = 0;
        int                 ofset = 0;
        float                 rv = 0;
        float                 rv1 = 0;
     	// if  arguments less MIN_SIZE or NULL,return 0 
        if(!buf || !temper || size < MIN_SIZE || !sn || !time_now )  
        {
            printf("Invalid arguments\n");
            return 0;
        }
        memset(buf,0,sizeof(buf));
        buf[ofset] = PACK_HEADER;
        printf("%#X\n",buf[ofset]);
        ofset += 1;
        buf[ofset] = TAG_TEMPER;
        ofset += 1;
        // if data_len size too long,maybe result overflow 
        if(strlen(sn) <= size - FIXED_SIZE)
        {
            data_len_sn = strlen(sn);
        }
        else
        {
            data_len_sn = size - FIXED_SIZE;
        }
        data_len_time = 6;
        data_len_temper = 2;
        data_len = data_len_sn + 1 + data_len_time + 1 + 1  + data_len_temper + 1 + 1;
        pack_len = data_len + FIXED_SIZE;
        buf[ofset] = pack_len;
        ofset += 1;
        /* value include three part,first is sn,second is time,third is temperature */
        memcpy(&buf[ofset],sn,data_len_sn); 
        ofset += data_len_sn;
        buf[ofset] = '/';
        ofset += 1;
        buf[ofset] = time_now -> tm_year;
        printf("%d\n",buf[ofset]);
        ofset += 1;
        buf[ofset] = time_now -> tm_mon + 1;
        ofset += 1;
        buf[ofset] = time_now ->tm_mday;
        ofset += 1;
        buf[ofset] = 0X20;
        ofset += 1;
        buf[ofset] = time_now ->tm_hour;
        ofset += 1;
        buf[ofset] = time_now ->tm_min;
        ofset += 1;
        buf[ofset] = time_now ->tm_sec;
        ofset += 1;
        buf[ofset] = '/';
        ofset += 1;
        rv = modff(*temper,&rv1);
        printf("%f\n%f\n",rv,rv1);
        buf[ofset] = rv1;
        printf("%#X\n",(int)rv1);
        ofset += 1;
     
        buf[ofset] = rv;
        printf("%#X\n",(float)buf[ofset]);
        printf("%f\n",(float)buf[ofset]);
        printf("temper = %f\n",*temper);
        printf("temperature = %f\n",(float)buf[ofset]);
        ofset += 1;
        buf[ofset] = 'c';
        ofset += 1;
        buf[ofset] = 0X0a;
        ofset += 1;
        crc16 = crc_itu_t(MAGIC_CRC,buf,ofset);
        ushort_to_bytes(&buf[ofset],crc16);
        ofset += 2;
        return ofset;
    }
```



------

**版权声明：本文转载于CSDN博主「心.跳」的原创文章，内容有修改**
**原文链接：https://blog.csdn.net/weixin_43001046/article/details/89048728**
