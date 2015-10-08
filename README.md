#iso8583

-----------------------------------------------------------------------

####目录
* 介绍
* 编译
* lua API
* C API

-----------------------------------------------------------------------
1. 介绍
这是一个给lua使用的iso8583解包和封包库， 使用c语言开发。因为里面先用c语言实现了一个iso8583的库，然后用lua 的api封装了一层，所以c语言也可以拿来用。

2. 编译
执行make， 会生成iso8583.so文件。将iso8583.so复制到lua的cpath目录中，就可以使用require("iso8583")来导入库的功能了。

3. lua API
使用local iso8583 = require("iso8583") 就可以导入库的功能了。
**常量**
*FIX*       ： 固定长度域，用于定义域的类型。
*LLVAR*  ： 2字节的变长域，用于定义域的类型。
*LLLVAR*： 3字节的变长域，用于定义域的类型。
*LEFT*    ： 左对齐，用于定义域的对齐方式。
*RIGHT*  ： 右对齐，用于定义域的对齐方式。
*L*         ： 左对齐，用于定义域的对齐方式。
*R*         ： 右对齐，用于定义域的对齐方式。
*ZIP*      ： 将数据作为HEX格式压缩成二进制的格式。
*UNZIP*  ： 不对数据做压缩处理。
*Z*         ：将数据作为HEX格式压缩成二进制的格式。
*U*         ：不对数据做压缩处理。
**函数**
*new(fields_define)*  ： 
新建一个iso8583对象，如果没有传入参数则使用默认的域定义，否则使用fields_define指定的域定义。fields_define 是一个table数组，用来指明iso8583域定义信息。比如
 `fields[61] = { size = 999, type = iso8583.LLLVAR, align = iso8583.L, pad = 'F', compress = iso8583.Z }`
表明第61域是变长域，左对齐，补位使用F，需要将数据当成HEX压缩成二进制数据。
*Unpack(iso8583_str)*  ：
解包iso8583数据，输入的参数是iso8583的报文，根据配置解出数据。
返回一个数组，如果对应的域有数据，则存入对应的域中。
其中第一域的值如果是"1", 则BITMAP为128位，如果为"0",则BITMAP为64。
*Pack(data)* ：
将data中的数据组成iso8583报文。

4. C API
`struct iso8583 *iso8583_create();`
创建一个iso8583结构体。
`int iso8583_define(struct iso8583 *handle, unsigned int index, unsigned int size, char pad, unsigned int type, unsigned int align, unsigned int compress);`
设置iso8583域定义。
`int iso8583_set(struct iso8583 *handle, unsigned int index, const unsigned char *data, unsigned int size);`
设置iso8583数据。
`int iso8583_get(struct iso8583 *handle, unsigned int index, const unsigned char **data, unsigned int *size);`
获取iso8583数据。
`int iso8583_pack(struct iso8583 *handle, unsigned char *data, unsigned int *size);`
封包iso8583数据，将设置好的数据封成iso8583包。
`int iso8583_unpack(struct iso8583 *handle, unsigned char *data, unsigned int *size);`
解包iso8583数据，将iso8583包解析，然后可以用iso8583_get获取数据。
`int iso8583_clear_datas(struct iso8583 *handle);`
清除设置好的iso8583域数据。
`int iso8583_clear_fields(struct iso8583 *handle);`
清除域定义配置数据，恢复到默认的域定义配置。
`int iso8583_clear(struct iso8583 *handle);`
清除域数据和域配置数据。
`int iso8583_destroy(struct iso8583 *handle);`
销毁掉iso8583结构体，释放iso8583占用的内存。
