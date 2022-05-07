#### TLV协议

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

##### 3、TLV协议问题
- 数据重合

  如果设置0x01为温度，那么后面有个0x01可能是表示数据的，而不是温度！这时候需要加一个报头，固定为0xFD，用来标志一个报文的开始。

- 数据跳变

  不能保证数据传输过程中数据的跳变，0x01表示温度，跳变为0x02就表示另外的东西，这时候需要在一个字节流末尾加一个CRC校验，传输前算一下字节流的大小，传输到另外一个端口后再算一下，对比前后两个CRC的值，如果相同，表示没有发生字节跳变，如果不同，那就舍弃！

- 报头在TLV协议中

  同样也要加CRC校验
  
  

##### 4、代码示例

###### 4.1、封包代码

```c
#include<stdio.h>
#include<string.h>
 
#define     HEAD                0xFD                    //定义一个报头
#define     BUF_SIZE            128                     //定义一个buffer的大小
#define     TLV_FIXED_SIZE      5                       //固定的TLV字节流大小，不包含value值
#define     TLV_MINI_SIZE       (TLV_FIXED_SIZE+1)      //TLV字节流的最小值，value的值为1个字节
#define     ON                  1
#define     OFF                 0

//使用枚举，Tag的值会自加
enum                                                    
{
    TAG_LOGON=1,                                        //登陆Tag
    TAG_CAMERA,                                         //摄像头Tag
    TAG_LED,                                            //照明灯Tag
};
int     pack_logon(char *buf, int size, char *psw);     //声明登陆（logon）封装函数
int     pack_camera(char *buf, int size, int cmd);      //声明摄像头（camera）封装函数
int     pack_led(char *buf, int size, int cmd);         //声明照明灯（led）封装函数
void    dump_buf(char*type,char *data,int len);         //声明dump_buf
 
int main(int argc, char **argv)
{
        char        buf[BUF_SIZE];
        int         bytes;                              //一个封装函数的字节流的个数
 
        bytes = pack_logon(buf,sizeof(buf),"iot@yun");  //设置登陆（logon）函数
        dump_buf("Logon",buf,bytes);                    
    //设置dump_buf函数，把所有的字节流都放到dump_buf里面
        bytes = pack_camera(buf,sizeof(buf),ON);        //设置摄像头（camera）函数
        dump_buf("Camera",buf,bytes);                            
 
        bytes = pack_led(buf, sizeof(buf), ON);         //设置照明灯（led）函数
        dump_buf("LED",buf,bytes);
 
        return 0;
}
 
//buf   缓冲区
//size  缓冲区的大小
//psw   密码
int pack_logon(char *buf, int size, char *psw)          //定义登陆（logon）封装函数
{
    unsigned short      crc=0;                          //crc校验值
    int                 pack_len=0;
    int                 data_len=0;
    int                 ofset=0;                        //buf的索引位置
    if( !buf || !psw || size<TLV_MINI_SIZE )            //判断
    {
        printf("Invalid input argument!\n");            //无效输入内容
        return 0;
    }
    buf[ofset]=HEAD;                                    //buf[0]设为报头（HEAD）
    ofset += 1;                                         //索引加1
    buf[ofset]=TAG_LOGON;                               //buf[1]设为类型（Tag）
    ofset += 1;
    if( strlen(psw)<size-TLV_FIXED_SIZE )               //psw是一个字节流，strlen（psw）计算它的长度，buf减去5等于value的值
        data_len = strlen(psw);                         //如果密码的长度小于value的值，就用密码的值
    else
        data_len = size-TLV_FIXED_SIZE;                 //否则就用value的最大值
    pack_len = data_len+TLV_FIXED_SIZE;                 //封装TLV的总长度
    buf[ofset]=pack_len;                                //buf[2]设为长度（Length）
    ofset += 1;
    memcpy(&buf[ofset],psw,data_len);                   //从psw拷贝到buf[3,4,5...]
    ofset += data_len;                                  //索引加data_len个字节  
//    crc = crc_itu_t(MAGIC_CRC, buf, ofset);             //调用crc函数,此函数在头文件中
//   ofset += 2;                                         //索引加2
    return ofset;                                       //返回索引值
}

//buf   缓冲区
//size  缓冲区的大小
//cmd   开关 
int pack_camera(char *buf, int size, int cmd)
{
    unsigned short      crc=0;
    int                 pack_len = TLV_FIXED_SIZE+1;    //开关（cmd）的值只有一个字节
    if( !buf || size<TLV_MINI_SIZE )
    {
        printf("Invalid input argument!\n");
        return 0;
    }
    buf[0] = HEAD;
    buf[1] = TAG_CAMERA;
    buf[2] = pack_len;
    buf[3] = ( ON==cmd ) ? 0x01:0x00;
 //   crc = crc_itu_t(MAGIC_CRC,buf,4);
 //   ushort_to_bytes(&buf[4],crc);
    return pack_len;
}

int pack_led(char *buf, int size, int cmd)
{
    unsigned short      crc=0;
    int                 pack_len = TLV_FIXED_SIZE+1;    //开关（cmd）的值只有一个字节 
    if( !buf || size<TLV_MINI_SIZE )
    {
        printf("Invalid input argument!\n");
        return 0;
    }
    buf[0] = HEAD;
    buf[1] = TAG_LED;
    buf[2] = pack_len;
    buf[3] = ( ON==cmd ) ? 0x01:0x00;
//    crc = crc_itu_t(MAGIC_CRC,buf,4);
//   ushort_to_bytes(&buf[4],crc); 
    return pack_len;
}
 
//定义dump_buf  
//data  指针，指着buf的首地址
//len   buf的长度 
void dump_buf(char *type,char *data, int len)           
{
    if( type )
    {
        printf("%s:\n",type);
    }
    for(int i=0; i<len; i++)
    {
        printf("0x%.2X ",data[i]);
    }
    printf("\n");
}
```



###### 4.2、解析代码

```c
#include<stdio.h>
#include<string.h>
#include<unistd.h> 
#include"crc-itu-t.c"
#include<stdlib.h>
 
#define TLV_MAX_SIZE    128
#define TLV_MIN_SIZE    6
#define HEAD            0xfd
 
enum
{
    TAG_LOGON=1,
    TAG_CAMERA,
    TAG_LED,
};
 
int unpack(char *buf,int bytes);

int main()
{
    int                 bytes;
    char                array[]={0xfd,0x03,0x06,0x00,0xb2,0x0e};
    
    bytes = unpack(array,sizeof(array));
 
    printf("The array has %d bytes!\n",sizeof(array));
    
    printf("array has %d bytes now!\n",bytes);
 
    return 0;
 
}
 
int unpack(char *buf,int bytes)
{
    int                 i;
    char                *ptr=NULL;
    int                 len;
    unsigned short      crc,val;
 
    if( !buf )
    {
        printf("Invailed input!\n");
        return 0;
    }
 
again:
 
    if( bytes < TLV_MIN_SIZE )                                      //数据小于一帧
    {
        printf("TLV packet is too short!\n");
        printf("Wait for continue input...\n");
        return bytes;                                               //返回半帧的值
    }
 
    for( int i=0; i<bytes; i++)                                     //数据大于一帧，开始遍历
    {
        if( (unsigned char)buf[i] == HEAD )
        {
 
            if(bytes-i < 2)                                         //这一帧中没有（length）长度
            {
                printf("\nTLV packet is too short.it is incomplete\n");
                printf("\nWait for continue input...\n");
                memmove(buf,&buf[i],bytes-i);                       //把半帧移到buf的开端
                return bytes-i;                                     //返回半帧的值
            }
 
            ptr = &buf[i];
            len = ptr[2];
 
            if(len < TLV_MIN_SIZE || len > TLV_MAX_SIZE)            //这一帧中（length）长度错误
            {
                memmove(buf,&ptr[2],bytes-i-2);                     //把这一帧扔掉
                goto again;                                         //继续遍历
            }
 
            if(len > bytes-i)                                       //（length）比剩下的还要长
            {
                memmove(buf,ptr,bytes-i);                           //把半帧移到buf的开端
                printf("TLV packet is incomplete!\n");
                printf("Wait for continue input...\n");
                return bytes-i;                                     //返回半帧的值
            }
 
            crc = crc_itu_t(MAGIC_CRC,(unsigned char *)ptr,(len-2));
            val = bytes_to_ushort( (unsigned char *)&ptr[len-2],2 );
 
            if(val != crc)                                          //两次的CRC不同
            {
                printf("CRC check error\n");
                memmove(buf,&ptr[len],bytes-i-len);                 //把这一帧扔掉
                bytes = bytes-i-len;
                goto again;                                         //继续遍历
            }
 
            for(int i=0; i<len; i++)
            {
                switch(ptr[i+1])
                {
                    case TAG_LOGON:
                                    printf("TAG_LOGON:\n");
 
                    case TAG_CAMERA:
                                    printf("TAG_CAMERA:\n");
 
                    case TAG_LED:
                                    printf("TAG_LED:\n");
                }
                
                printf("0x%02x ",ptr[i]);
            }
            printf("\n");
 
            memmove(buf,&ptr[len],bytes-i-len);                     //把这一帧扔掉
 
            bytes = bytes-i-len;
            goto again;                                             //继续遍历
        }
    } 
}
```

###### 4.3、其他装包代码

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

**版权声明：本文为CSDN博主「Sheerandeng」的原创文章，内容有修改**
**原文链接：https://blog.csdn.net/Shallwen_Deng/article/details/88930288**

**文件链接：https://wenku.baidu.com/view/ed9976eac67da26925c52cc58bd63186bceb92e2.html**
