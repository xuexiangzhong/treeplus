//
// Created by snowfly on 2021/9/30.
//

#ifndef FLYDB_BTREEFILE_H
#define FLYDB_BTREEFILE_H
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <zconf.h>
#include <MacTypes.h>
//#define M (4)
#define LIMIT_M_2 (M % 2 ? (M + 1)/2 : M/2)
#define byte unsigned char
#define IndexKeySize (6)
#define DB_INFO ".info"
#define DB_INDEX ".index"
#define DB_DATA ".db"

//#define dynamicDateNum (128)
//#define dynamicIndexNum (256)
/**
 * size  每个基础数据对象占用的大小
 * pageNum  页号
 * keynum  key 数目
 * Byte key[] key数组
 * Byte value[] 值数组
 * */
unsigned int getIntBychar4(byte char4[]);
//里面有malloc不安全 不使用
//byte* getByteByint(unsigned int num);
static inline void swapBytes(unsigned char *pIn, unsigned char *pOut, int num);
void swapByteSelf(unsigned char *pIn, int num);
void setByteByint(unsigned int num,byte key[]);
//里面有malloc不安全 不使用
//byte* getByteBylong(unsigned long num);
byte* getByteByMinNum(int num,int type);
unsigned long getLongBychar8(byte char8[]);
void setByteBylong(unsigned long num,byte key[]);
struct PageBase{//索引叶子中的data对象  维护着一页的key和数据 假设原子页子大小为4K
    unsigned int curNo;//当前页序号
    //下一页
    unsigned int nextPageNum;//页初始化的时候会变动 默认使用4个字节存储 具体看编译器
    //上一页
    unsigned int prePageNum;//页初始化的时候会变动 默认使用4个字节存储 具体看编译器
    //父页 此字段的存储影响分裂性能 已废弃
    unsigned int parentPageNum;//页初始化的时候会变动 默认使用4个字节存储 具体看编译器
    int type;//type 类型  1索引 2 索引叶子节点 data   3 data节点
    int childNum;//当前数目  char 在结构体里面也占用4个字节  所以这里直接用int了
    //以上的5个为每个库文件固定数值，都是初始化完成 之后将不再变动
};
typedef struct FastTreeDb *treedb;
struct FastTreeDb{//初始化一个db树
    char filename[1024];//文件名称 32 20  4 + 8
    //索引页号大小 永远用4k大小 data页大小 根据数据的大小情况判断
    int indexPageSize;//索引页大小
    int pageSize;//数据页大小
    int keySize;//key长度 0 为任意长度
    int valueSize;//value长度 0 为任意长度
    int keytype;// 0 默认byte型 1 字符串型 2 整型 3 浮点数型
    int maxNodeNum;//每页最大节点数  最大数目不超过256
    int maxIndexNodeNum;//每页索引最大节点数  最大数目不超过256
    unsigned int rootPageNum;//root index页号  最顶层页的页号
    unsigned int maxIndexPageNum;//当前最大索引页号
    unsigned int maxDataPageNum;//当前最大data页号
    int storetype;//存储类型 大端返回0，小端返回1  默认小端存储
    int dynamicDateNum;//动态数据的date预留key寻址空间的个数
    int dynamicIndexNum;//动态索引的date预留key寻址空间的个数
    int dbinfofd;//info
    int indexfd;//index
    int datafd;//data
};

struct ValueNode{
    byte* value;//文件名称 32 20  4 + 8
    int valueSize;//key长度  小于0表示查找失败 未找到
};

struct KeyNode{
    byte* key;//文件名称 32 20  4 + 8
    int keySize;//key长度  小于0表示查找失败 未找到
    int keyType;// 0 默认byte型 1 字符串型 2 整型 3 浮点数型
};

struct EntryNode{
    struct ValueNode valueNode;
    struct KeyNode keyNode;
};

struct SeekNode{//返回遍历对象
    byte* pagedata;//当前seek页数据
    int seekindex;//当前seek点在此页中的序号
    unsigned int curPageNo;//当前页的页号
};

struct FastTreeDb *treedb_init(char *filename, int indexPageSize,int pageSize, int keySize,int valueSize,int dynamicDateNum,int dynamicIndexNum,int keytype);





printTreedbPage(struct FastTreeDb * db,unsigned int  pagenum,int type);

//type 1 按索引遍历
//type 2 按data页顺序遍历
printTreedb(struct FastTreeDb * db,int type);

insertTreedb(struct FastTreeDb * db, unsigned char key[],unsigned char value[]);
unsigned int selectPageByDbKey(struct FastTreeDb *db,unsigned int pageNum,int type, byte key[],int keysize);
int selectIndexData(byte data[],int zongliang,byte key[],int keySize,int offset,int width,int keytype);
int selectDynamicIndexData(byte data[],int zongliang,byte key[],int keySize,int offset,int width,int keytype);
void opendb(struct FastTreeDb *db);
void closedb(struct FastTreeDb *db);
void intsertDataPageByKeyValue(struct FastTreeDb *db,unsigned int pageNum,int type,byte key[],int keysize,byte value[],int valuesize);

insertTreedbByDataAndSize(struct FastTreeDb * db, unsigned char key[],int keysize,unsigned char value[],int valuesize);

//ValueNode 的 value是malloc的 在使用完后需要释放
getDataByDbKey(struct FastTreeDb *db,byte key[],int keysize,struct ValueNode *node);

//seekType 特殊SeekNode获取定义  传1代表 取第一个date页第一个值  传2 代表取最后一页最后一个值  其余的默认按key找到对应所在页的对应的值
//SeekNode 的 pagedata在外面分配内存，注释掉了方法里面的malloc 在方法外初始化以便于后续释放
getSeekNodeByDbKey(struct FastTreeDb *db,byte key[],int keysize,struct SeekNode *node,int seekType);

//获取下一个key  type为1 表示获取下一个key的同时  next移动到对应位置 EntryNode 的 value 和 key的指针 使用完后要记得释放
//EntryNode 在外面初始化 可以避免方法内部malloc 上层使用可以更加灵活高效
int getNextBySeekNode(struct FastTreeDb *db,struct SeekNode *node,int type,struct EntryNode *entryNode);

//获取上一个key  type为1 表示获取上一个key的同时  prev移动到对应位置  EntryNode 的 value 和 key的指针 使用完后要记得释放
int getPreBySeekNode(struct FastTreeDb *db,struct SeekNode *node,int type,struct EntryNode *entryNode);

//获取当前key
int getCurNode(struct FastTreeDb *db,struct SeekNode *node,struct EntryNode *entryNode);

#endif //FLYDB_BTREEFILE_H
