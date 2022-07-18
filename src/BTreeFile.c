//
// Created by snowfly on 2021/9/30.
//
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include "BTreeFile.h"

const int indexPage = 4*1024;
const int keylen = 20;
const int valuelen = 100;

static int compare(unsigned char key[], unsigned char data[], int keysize,int keytype);
int compareDynamic(unsigned char key[], unsigned char data[], int keysize, int datakeysize, int keytype);
/**
 * fd 文件操作句柄
 * size  每个基础数据对象占用的大小
 * pageNum  页号
 * keynum  key 数目
 * Byte key[] key数组
 * Byte value[] 值数组
 * */

int intsertDataByDataPage(struct FastTreeDb *db, unsigned int num, byte key[], byte value[],int keysize,int valuesize);

void swapDouble64(unsigned char *string, unsigned int *pInt, int i);

static unsigned int splitDataPage(struct FastTreeDb *pDb, unsigned int num);

static unsigned int splitDynamicDataPage(struct FastTreeDb *pDb, unsigned int num);

void initFirstPage(struct FastTreeDb *pDb);

static unsigned int getNextIndexPage(struct FastTreeDb *pDb);

static unsigned int insertPageAndKeyIndex(struct FastTreeDb *pb, unsigned int indexnum,byte *key, unsigned int childpage,int keysize);

static unsigned int splitIndexPage(struct FastTreeDb *pDb, unsigned int indexnum);

static unsigned int splitDynamicIndexPage(struct FastTreeDb *pDb, unsigned int indexnum);

int readIndexByPageNum(struct FastTreeDb *pb, void *data, unsigned int num);

int readDataByPageNum(struct FastTreeDb *pb, void *data, unsigned int num);

void showPageBase(struct PageBase *pBase);

int writeDataByPageNum(struct FastTreeDb *pb, void *data, unsigned int num);

static int writeIndexByPageNum(struct FastTreeDb *db,  void *data, unsigned int indexnum);

void setkeytoDataPageByte(struct FastTreeDb *db, byte data[],byte *key, byte *value, int start,
                      int isupdate);
void setDynamicData(struct FastTreeDb *db, byte data[],byte *key, int keysize,byte *value,int valuesize, int start,
                          int isupdate);
static void setkeytoIndexPageByte(struct FastTreeDb *db,  byte data[], byte *key, unsigned int childpage,
                      int start, int isupdate);
static void setDynamicIndex(struct FastTreeDb *db,  byte data[], byte *key, int keysize,
                            unsigned int childpage, int start, int isupdate);

void showdb(struct FastTreeDb *pDb);

static unsigned int getNextDataPage(struct FastTreeDb *db);

static void initRootIndexBykeyPage(struct FastTreeDb *pDb, unsigned int num, unsigned char *key, unsigned int indexnum,int keysize);

void moveDataPage(unsigned char *data, int chazhi, int start,int startoff,int maxdataoff);

void printIndexPage(struct FastTreeDb *pDb, unsigned char *data);

void printDynamicIndexPage(struct FastTreeDb *pDb, unsigned char *data);

void printDatePage(struct FastTreeDb *db, unsigned char *b);

void printDynamicDatePage(struct FastTreeDb *db, unsigned char *b);

static void moveIndexPage(unsigned char *data, int chazhi, int start,int startoff,int maxdataoff);

void
getDataByPageAndKey(struct FastTreeDb *pDb, unsigned int pageNum, unsigned char *key, int keysize, struct ValueNode *pNode);

unsigned int
selectFirstOrLastDatePageByDb(struct FastTreeDb *db,unsigned int pageNum,int type,int firstOrLast);

void setEntryNodeByIndex(struct EntryNode *entryNode, int index, unsigned char *pagedata, int keysize, int valuesize);

unsigned int getIntBychar4(byte char4[]){
    unsigned int num = 0;
    for(int i = 0 ;i < 4;i++){
//        num = num * 256 + char4[i]; //0.089s
        //移位 除法+取余 n>>a为除 n <<a 为乘 a为2的次方数 0.098
        num = (num << 8) + char4[i];//0.093s
    }
    return num;
}
static inline void swapBytes(unsigned char *pIn, unsigned char *pOut, int num) {
    for( int i=0;i<num;i++)
        pOut[num-1-i] = pIn[i];
}
//底高位转化 常用于大小端的对转 1000万次 0.13s
void swapByteSelf(unsigned char *pIn, int num) {//假设总长度为n位 则转化N/2次
    for( int i=0;i<num/2;i++){
        unsigned char temp = pIn[i];
        pIn[i] = pIn[num-1-i];
        pIn[num-1-i] = temp;
    }
}

//byte *getByteByint(unsigned int num){
//    //1000 万次 0.65s
//    byte *b = (byte *)malloc(4);
//    for(int i = 0 ;i < 4;i++){
//        b[3-i] = num - (num>>8<<8);
//        num = num >> 8;
//    }
//    //1000 万次 0.67s
////    byte * b = malloc(4);
////    swapBytes(&num,b,4);
//    return b;
//}


void setByteByint(unsigned int num,byte key[]){
    //1000万次 0.11  加上1000万的数组定义是0.15
//    swapBytes(&num,key,4);
    for(int i = 0 ;i < 4;i++){
//        取余操作
//        a % b = a & (b-1)(b=2n)
//        即：a % 2n = a & (2n-1)
//        key[3-i] = num &(255);
//        num = num >> 8;
//        （2）移位 除法+取余 n>>a为除 n <<a 为乘 a为2的次方数 0.098
        key[3-i] = num - (num>>8<<8);
        num = num >> 8;
//        普通做法 1000w次 0.23s
//        key[3-i] = num%255;
//        if(num > 0){
//            num = num/255;
//        }
    }
}
unsigned long getLongBychar8(byte char8[]){
    unsigned long num = 0;
    for(int i = 0 ;i < 8;i++){
//        num = num * 256 + char8[i];
        num = (num << 8) + char8[i];
    }
    return num;
}
//byte *getByteBylong(unsigned long num){
////    byte key[4] = {};
////    byte (*key)[4];
////    key = (byte(*)[4])malloc(4);
////    printf("long=%ld",num);//
//    byte *b = (byte *)malloc(8);
//    for(int i = 0 ;i < 8;i++){
//        b[7-i] = num - (num>>8<<8);
//        num = num >> 8;
////        b[7-i] = num%255;
////        if(num > 0){
////            num = num/255;
////        }
//    }
//    return b;
//}
//type 1 小端 0 大端
//num 字节位数
//带符号的数字 取最小值
byte * getByteByMinNum(int num,int type){
    byte *b = (byte *)malloc(num);
    for(int i = 0 ;i < num;i++){
        b[i] = 0x00;
    }
    if(type == 1){//小端 符号位在最后面
        b[num-1] = 0x80;
    }else if(type == 0){//大端 符号位在最前面
        b[0] = 0x80;
    }
    return b;
}
void setByteBylong(unsigned long num,byte key[]){
    for(int i = 0 ;i < 8;i++){
        key[7-i] = num - (num>>8<<8);
        num = num >> 8;
    }
}
//大端返回0，小端返回1
int checkCPU()
{
    union
    {
        int a;
        char b;
    }c;
    c.a=1;

    return (c.b == 1);
}
struct FastTreeDb *treedb_init(char *filename, int indexPageSize,int pageSize, int keySize,int valueSize,int dynamicDateNum,int dynamicIndexNum,int keytype)
{//初始化
    struct FastTreeDb *db = calloc(1, sizeof(*db));//db信息
    assert(db != NULL);//断言 如果分配失败则结束
//    struct bplus_tree *tree = calloc(1, sizeof(db));//给tree分配内存
//    assert(tree != NULL);//断言 如果分配失败则结束
    char infofilename[1024]= "";
//    /* load index boot file */
    strcat(infofilename,filename);
    strcat(infofilename,DB_INFO);
//    printf("%s %d\n",strcat(db->filename, ".info"),sizeof(*db));
    int fd = open(infofilename,  O_RDWR , 0644);//读取初始化元数据
    if (fd >= 0) {//元数据已经存在 则读取文件加载 info文件
        printf("读取到元数据!\n");
        ssize_t len = pread(fd,db,sizeof(struct FastTreeDb),0);//从0的位置开始读
        if (db->storetype != checkCPU()) {
            fprintf(stderr, "存储的大小端不匹配!\n");
            return NULL;
        }
        close(fd);
    } else {//没有则从头开始
        if (strlen(filename) >= 1024) {//文件名长度校验
            fprintf(stderr, "db file name too long!\n");
            return NULL;
        }

        if (indexPageSize > 0 && indexPageSize%4096 != 0) {//大小必须为4K的整倍
            fprintf(stderr, "indexPageSize 必须是4096的倍数 !当前值 %d\n",indexPageSize);
            return NULL;
        }

        if (pageSize > 0 && pageSize%4096 != 0) {//大小必须为4K的整倍
            fprintf(stderr, "pageSize 必须是4096的倍数 !\n");
            return NULL;
        }
        if (keySize + valueSize >= pageSize) {
            fprintf(stderr, "key value值设置过大!\n");
            return NULL;
        }
        if ((keytype == 2 || keytype == 3) && !(keySize == 4 || keySize == 8)) {//整型或浮点型只支持4或8字节
            fprintf(stderr, "整型或浮点型只支持4或8字节!\n");
            return NULL;
        }
        if (keySize == 0 && dynamicIndexNum <= 0) {
            fprintf(stderr, "任意长度的key 必须设置预留索引空间数目!\n");
            return NULL;
        }
        if ((valueSize == 0 || keySize == 0 ) && dynamicDateNum <= 0) {
            fprintf(stderr, "任意长度的data 必须设置预留索引data的数目!\n");
            return NULL;
        }
        printf("未读取到元数据!\n");
       int  fd2 = open(infofilename, O_CREAT | O_RDWR | O_APPEND, 0644);// 写入
       strcpy(db->filename, filename);//copy文件名给树
        db->valueSize = valueSize;
        db->keySize = keySize;
        db->indexPageSize = indexPageSize;
        db->pageSize = pageSize;
        db->storetype = checkCPU();//大端返回0，小端返回1
        db->dynamicDateNum = dynamicDateNum;
        db->dynamicIndexNum = dynamicIndexNum;
        //每个key和value 除了本身的长度 还需要存储索引的长度
        //1 固定key value的情况下  索引和数组 都额外 +1 位
        //2 固定key  非固定value的情况下
        // 索引页 只有额外 +1 位
        // data 页的按实际value大小处理 所以设置位0
        //3 key 和 value 都不固定的情况下
        db->keytype = keytype;// 0 默认byte型 1 字符串型 2 整型 3 浮点数型
        // 都按实际大小处理
        int size  = sizeof(struct PageBase);
        if(keySize > 0){
            db->maxIndexNodeNum = (indexPageSize-size )/(keySize + IndexKeySize);//每个key序列后面还需要跟页号（4字节）默认按6个字节来
            initFirstPage(db); //初始化一个最小的key值为root索引页的初始值
        }else{
            db->maxIndexNodeNum = 0;
            initFirstPage(db); //初始化一个最小的key值为root索引页的初始值
            //不固定的key 需要额外加寻址空间 寻址空间为 key的偏移量和key的size  一共8字节
            //寻址空间目前固定预留 128个节点的空间 也就是128*12的长度索引空间 dynamicDateNum * 8
        }
        if(valueSize > 0){
            db->maxNodeNum = (indexPageSize-size)/(keySize + valueSize);//因为key value的长度固定，那么可以直接按顺序排列
        }else{//value 不固定 那么datapage数目也不固定
            //这种结构一般先预留一个key的寻址空间
            //一个寻址空间包含 key地址偏移量 和key的长度  value的长度  共12个字节
            //寻址空间目前固定预留 128个节点的空间 也就是128*12的长度索引空间 dynamicIndexNum * 12
            db->maxNodeNum = 0;
        }
        db->rootPageNum = 1;//最大的索引页 默认为1
        db->maxIndexPageNum = 1;//最大索引页号
        db->maxDataPageNum = 1;//最大数据页号
       ssize_t t =pwrite(fd2,db,sizeof(struct FastTreeDb),0);//从0的位置开始写
       close(fd2);
    }
    return db;
}

void initFirstPage(struct FastTreeDb *db) {
    //首先创建一个默认的data数据页
    //写入一个root页放入第一页
    char datafilename[1024]= "";
//    /* load index boot file */
    strcat(datafilename,db->filename);
    strcat(datafilename,DB_DATA);
    int  fd3 = open(datafilename, O_CREAT | O_RDWR | O_APPEND, 0644);// 第一个数据页页写入
    byte b[db->pageSize];
    int prefixsize  = sizeof(struct PageBase);
    //初始化一个data页信息
    struct PageBase pageBase = {
            curNo : 1,//当前页序号
            //下一页
            nextPageNum:0,//页初始化的时候会变动 默认使用4个字节存储 具体看编译器
            //上一页
            prePageNum:0,//页初始化的时候会变动 默认使用4个字节存储 具体看编译器
            //父页
            parentPageNum:1,//data页的父节点设置为默认索引的页号1
            type:3,//type 类型 索引还是data 1代表索引 2 代表叶子节点索引 默认为data类型的页
            childNum:0,//当前数目默认为0
    };
    memmove(&b[0], &pageBase, prefixsize);//24个字节
    //然后设置第一个key值和对应的页信息
    memset(&b[prefixsize], 0, db->pageSize - prefixsize);//24个字节
    pwrite(fd3,b,db->pageSize,0);//从0的位置开始写入data页
    close(fd3);

    //默认页写好后开始创建第一个索引页

    //写入一个root页放入第一页
    char indexrootname[1024]= "";
//    /* load index boot file */
    strcat(indexrootname,db->filename);
    strcat(indexrootname,DB_INDEX);
    int  indexfd = open(indexrootname, O_CREAT | O_RDWR | O_APPEND, 0644);// 第一个root索引页写入
//        byte *b = (byte *)malloc(indexPageSize);
    byte indexb[db->indexPageSize];
    //初始化一个index页 赋值
    pageBase.type = 2;//type 类型 索引还是data 1代表索引 2 代表叶子节点索引 默认为data类型的页
    pageBase.parentPageNum = 0;//初始化的索引为root索引 父级页为0
    pageBase.childNum = 1;//当前数目默认设为1 值为之前创建的data页，页号1
//    showPageBase(&pageBase);
    memmove(&indexb[0], &pageBase, prefixsize);//24个字节 设置前缀
    //然后设置第一个key信息 这个key
    byte p[4];
    setByteByint(1,p);
    if(db->keySize > 0){
        //默认最小值设置
        if(db->keytype == 0 || db->keytype == 1 ){
            memset(&indexb[prefixsize], 0, db->keySize);//keysize个字节 设置key个字符
        }else if(db->keytype == 2){
            byte * minkey = getByteByMinNum(db->keySize,db->storetype);
            memmove(&indexb[prefixsize],&minkey[0], db->keySize);//4个字节 设置前缀
            free(minkey);
        }else if(db->keytype == 3){
            if(db->keySize == 4){
                float f3 = -0x1.fffffeP+127f;
                byte * minkey = &f3;
                memmove(&indexb[prefixsize],&minkey[0], db->keySize);//4个字节 设置前缀
            }else if(db->keySize == 8){
                double f3 = -0x1.fffffffffffffP+1023;
                byte * minkey = &f3;
                memmove(&indexb[prefixsize],&minkey[0], db->keySize);//4个字节 设置前缀
            }
        }else{
            memset(&indexb[prefixsize], 0, db->keySize);//keysize个字节 设置key个字符
        }
        memmove(&indexb[prefixsize + db->keySize],p, 4);//4个字节 设置前缀
    }else{//暂时不处理这种非固定key的情况
        //默认最小值设置 任意长度的key必须为byte流
        //先预留N位的索引空间  每个索引占8个字节 前4位是起始位，后4位是key的长度 用 unsigned int 类型
        //末尾追加的4位是当前page的未使用空间的起始偏移量 常用数字为128*8+4 = 1028位
        int  startoff = prefixsize + db->dynamicIndexNum * 8 +4;//初始偏移位置
        memmove(&indexb[prefixsize],&startoff, 4);//设置起始第一个key偏移量
        unsigned int  startinitsize = 1;
        memmove(&indexb[prefixsize + 4],&startinitsize, 4);//设置第一个key的长度值

        //设置偏移量位置的值
        unsigned int space = startoff + 5;//已占用空间的偏移量  第一个key是一个字节，加上页号4字节 共占用5字节
        memmove(&indexb[startoff - 4],&space,4);//设置占用的空间

//        memset(&indexb[prefixsize+8], 0,sizenum-8);//将索引位设置位
        memset(&indexb[startoff],0, 1);//4个字节 设置前缀key 1位 值位0
        memmove(&indexb[startoff+ 1],&p, 4);//4个字节 设置data页号
    }
    pwrite(indexfd,indexb,db->indexPageSize,0);//从0的位置开始写入root索引页1
    close(indexfd);
//    free(p);
    //初始化一个索引页对象
}


void showPageBase(struct PageBase *pageBase) {
//    printf("\033[47;31m查询到的页数.%d\033[0m",dataPageNum);
//    printf("showPageBase start \n");
//    printf("当前页号 %d \n",pageBase->curNo);
//    printf("下一页 %d\n",pageBase->nextPageNum);
//    printf("上一页 %d\n",pageBase->prePageNum);
//    printf("父页 %d\n",pageBase->parentPageNum);
//    printf("类型 %d\n",pageBase->type);
//    printf("字节点数目 %d\n",pageBase->childNum);
//    printf("showPageBase end \n");
}

void showPageBaseall(struct PageBase *pageBase) {
//    printf("\033[47;31m查询到的页数.%d\033[0m",dataPageNum);
    printf("showPageBase start \n");
    printf("当前页号 %d \n",pageBase->curNo);
    printf("下一页 %d\n",pageBase->nextPageNum);
    printf("上一页 %d\n",pageBase->prePageNum);
    printf("父页 %d\n",pageBase->parentPageNum);
    printf("类型 %d\n",pageBase->type);
    printf("字节点数目 %d\n",pageBase->childNum);
    printf("showPageBase end \n");
}
printTreedbPage(struct FastTreeDb * db,unsigned int pagenum,int type){
    //打印对应的page
    if(pagenum <= 0){
//        pagenum = db->rootPageNum;
//        type = 1;
        return 0;
    }
    if(type == 1 || type == 2){//索引
        char indexrootname[1024]= "";
//    /* load index boot file */
        strcat(indexrootname,db->filename);
        strcat(indexrootname,DB_INDEX);
        printf("索引页打印，页号：%d \n",pagenum);
//    printf("%s %d\n",strcat(db->filename, ".info"),sizeof(*db));
        int fd = open(indexrootname,   O_CREAT | O_RDWR | O_APPEND , 0644);//读取初始化元数据
        byte b[db->indexPageSize];
        ssize_t len = pread(fd, b, db->indexPageSize, (pagenum - 1) * db->indexPageSize);
        if(db->keySize > 0 ){
            printIndexPage(db,b);
        }else{
            printDynamicIndexPage(db,b);
        }
        close(fd);
    }else{//data
        char indexrootname[1024]= "";
        strcat(indexrootname,db->filename);
        strcat(indexrootname,DB_DATA);
        printf("data页打印 页号：%ud!\n",pagenum);
        int fd = open(indexrootname,   O_CREAT | O_RDWR | O_APPEND , 0644);//读取初始化元数据
        byte b[db->pageSize];
        ssize_t len = pread(fd, b, db->pageSize, (long)(pagenum - 1) * db->pageSize);
        if(db->keySize > 0 && db->valueSize > 0){
            printDatePage(db,b);
        }else{
            printDynamicDatePage(db,b);
        }
        close(fd);
    }
}

printTreedb(struct FastTreeDb * db,int type){
    showdb(db);

    printf("打印页信息 \n");
//    //打印对应的page
//    if(pagenum <= 0){
//        pagenum = db->rootPageNum;
//        type = 1;
//    }
    printTreedbPage(db,db->rootPageNum,1);
//    if(type == 1){//索引
//        char indexrootname[1024]= "";
////    /* load index boot file */
//        strcat(indexrootname,db->filename);
//        strcat(indexrootname,DB_INDEX);
////    printf("%s %d\n",strcat(db->filename, ".info"),sizeof(*db));
//        int fd = open(indexrootname,   O_CREAT | O_RDWR | O_APPEND , 0644);//读取初始化元数据
//        byte b[db->indexPageSize];
//        ssize_t len = pread(fd, b, db->indexPageSize, (db->rootPageNum - 1) * db->indexPageSize);
//        if(db->keySize > 0 ){
//            printIndexPage(db,b);
//        }else{
//            printDynamicIndexPage(db,b);
//        }
//        printf("读取到index页!\n");
//    }
}

void printDynamicDatePage(struct FastTreeDb *db, unsigned char *b) {
    struct PageBase *pageBase = b;
    showPageBaseall(pageBase);
    int keystart = sizeof(struct PageBase);
    for(int i = 0;i<pageBase->childNum;i++){
        printf("子数目序号 %d ",i);
        unsigned int *curoff  = b+keystart + i * 12; //前4位是offset 后4位是keysize 在后四位是valuesize
        unsigned int *curkeysize  = b+keystart + i * 12 + 4; //offset 后4位是keysize
        unsigned int *curvaluesize  = b+keystart + i * 12 + 8; //keysize 后4位是valuesize

//        printf("偏移位置 %d key长度 %d value长度 %d \n",*curoff,*curkeysize,*curvaluesize );
        printf("key 为默认byte类型: ");
        for(int k = 0;k<*curkeysize;k++ ){
            printf("%c",(char)(b[(*curoff) + k]));
        }
        unsigned int numkey = getIntBychar4(&b[(*curoff)]);
        printf(" \n数字打印%d 长度%d ",numkey,*curkeysize);
        printf(" key end \n");

        printf("value : ");
        for(int v = 0;v<*curvaluesize;v++ ){//value为页号 为固定值
            printf("%c",(char)b[*curoff + *curkeysize + v]);
        }
        printf(" value end \n");
    }
}

void printDatePage(struct FastTreeDb *db, unsigned char *b) {
    struct PageBase *pageBase = b;
    showPageBaseall(pageBase);
    int keystart = sizeof(struct PageBase);
    int width = db->keySize+db->valueSize;
    for(int i = 0;i<pageBase->childNum;i++){
        printf("子数目序号 %d \n",i);
        // 0 默认byte型 1 字符串型 2 整型 3 浮点数型
        if(db->keytype == 0){
            //打印key
            printf("key 为默认byte类型: ");
            for(int k = 0;k<db->keySize;k++ ){
                printf("%c ",(char)(b[keystart + i * width + k]));
            }
            unsigned int numkey = getIntBychar4(&b[(keystart + i * width)]);
            printf(" \n数字打印%d ",numkey);
        }else if(db->keytype == 1){
            printf("key 为默认字符串类型: ");
            for(int k = 0;k<db->keySize;k++ ){
                printf("%c ",(char)(b[keystart + i * width + k]));
            }
        }else if(db->keytype == 2 && db->keySize == 4){
            printf("key 为默认int类型: ");
            printf(" int 值为：%d",*(int *)(b+keystart + i * width));
        }else if(db->keytype == 2 && db->keySize == 8){
            printf("key 为默认long类型: ");
            printf(" int 值为：%ld",*(long *)(b+keystart + i * width));
        }else if(db->keytype == 3 && db->keySize == 4){
            printf("key 为默认float类型: ");
            printf(" int 值为：%f",*(float *)(b+keystart + i * width));
        }else if(db->keytype == 3 && db->keySize == 8){
            printf("key 为默认double类型: ");
            printf(" double 值为：%f",*(double *)(b+keystart + i * width));
        }
        printf(" key end \n");
        printf("value : ");
        for(int v = 0;v<db->valueSize;v++ ){
            printf("%c",b[keystart + i * width + db->keySize+ v]);
        }
        printf(" value end \n");
    }
}

void printDynamicIndexPage(struct FastTreeDb *db, unsigned char *b) {
    struct PageBase *pageBase = b;
    showPageBaseall(pageBase);
    int keystart = sizeof(struct PageBase);
    for(int i = 0;i<pageBase->childNum;i++){
        printf("子数目序号 %d \n",i);
        unsigned int *curoff  = b+keystart + i * 8; //前4位是offset 后4位是keysize 在后四位是valuesize
        unsigned int *curkeysize  = b+keystart + i * 8 + 4; //前4位是offset 后4位是keysize
//        unsigned int curvaluesize  = *(b+keystart + i * 12 + 8); //前4位是offset 后4位是keysize
        printf("key 为默认byte类型 size 为 %d 偏移位置 %d: \n ",*curkeysize,*curoff);
        for(int k = 0;k<*curkeysize;k++ ){
            printf("%c",(byte)(b[*curoff + k]));
        }
        printf(" key byte ");
        for(int k = 0;k<*curkeysize;k++ ){
            printf("%u ",(byte)(b[*curoff + k]));
        }
        printf(" key end \n");

        printf("value : ");
//        for(int v = 0;v<4;v++ ){//value为页号 为固定值
//            printf("%d",(char)b[*curoff + *curkeysize + v]);
//        }
        unsigned int page = getIntBychar4(&b[*curoff + *curkeysize]);
        printf("page子页号 %d",page);
        if(pageBase->type == 1){//下级为索引
            printTreedbPage(db,page,1);
        }else{//下级为data页
            printTreedbPage(db,page,3);
        }
        printf(" value end \n");
    }
}

void printIndexPage(struct FastTreeDb *db, unsigned char *b) {
    struct PageBase *pageBase = b;
    showPageBaseall(pageBase);
    int keystart = sizeof(struct PageBase);
    int width = db->keySize+IndexKeySize;
    for(int i = 0;i<pageBase->childNum;i++){
        printf("子数目序号 %d \n",i);
        // 0 默认byte型 1 字符串型 2 整型 3 浮点数型
        if(db->keytype == 0){
            //打印key
            printf("key 为默认byte类型: ");
            for(int k = 0;k<db->keySize;k++ ){
                printf("%d ",(char)(b[keystart + i * width + k]));
            }
        }else if(db->keytype == 1){
            printf("key 为默认字符串类型: ");
            for(int k = 0;k<db->keySize;k++ ){
                printf("%c ",(char)(b[keystart + i * width + k]));
            }
        }else if(db->keytype == 2 && db->keySize == 4){
            printf("key 为默认int类型: ");
            printf(" int 值为：%d",*(int *)(b+keystart + i * width));
        }else if(db->keytype == 2 && db->keySize == 8){
            printf("key 为默认long类型: ");
            printf(" int 值为：%ld",*(long *)(b+keystart + i * width));
        }else if(db->keytype == 3 && db->keySize == 4){
            printf("key 为默认float类型: ");
            printf(" int 值为：%f",*(float *)(b+keystart + i * width));
        }else if(db->keytype == 3 && db->keySize == 8){
            printf("key 为默认double类型: ");
            printf(" double 值为：%f",*(double *)(b+keystart + i * width));
        }
        printf(" key end \n");

        printf("value : ");
        unsigned int page = getIntBychar4(&b[keystart + i * width + db->keySize]);
        printf("page子页号 %d",page);
        if(pageBase->type == 1){//下级为索引
            printTreedbPage(db,page,1);
        }else{//下级为data页
            printTreedbPage(db,page,3);
        }
//        printf()
        printf(" value end \n");
    }
}

void showdb(struct FastTreeDb *db) {
    printf("库文件路径名 :%s \n",db->filename);
    printf("索引页大小 :%d \n",db->indexPageSize);
    printf("数据页大小 :%d \n",db->pageSize);
    printf("key长度 :%d \n",db->keySize);
    printf("value长度 :%d \n",db->valueSize);
    printf("data页子节点最大个数 :%d \n",db->maxNodeNum);
    printf("索引页子节点最大个数 :%d \n",db->maxIndexNodeNum);
    printf("root索引页页号 :%u \n",db->rootPageNum);
    printf("最大data页页号 :%u \n",db->maxDataPageNum);
    printf("最大索引页页号 :%u \n",db->maxIndexPageNum);
};

insertTreedb(struct FastTreeDb * db, byte key[],byte value[]){
    //先查找到对应的data页 并插入此数据
    intsertDataPageByKeyValue(db,db->rootPageNum,1,key,0,value,0);//默认从root索引开始查
    //如果此data页是新创建的页 那么则需要将页号添加到上级索引

}
insertTreedbByDataAndSize(struct FastTreeDb * db, byte key[],int keysize,byte value[],int valuesize){
    //先查找到对应的data页 并插入此数据
    intsertDataPageByKeyValue(db,db->rootPageNum,1,key,keysize,value,valuesize);//默认从root索引开始查 父页为0
    //如果此data页是新创建的页 那么则需要将页号添加到上级索引
}

getDataByDbKey(struct FastTreeDb *db,byte key[],int keysize,struct ValueNode *node){
    //从root页查找 找到数据应该对应插入的data页 页号
    unsigned int dataPageNum = selectPageByDbKey(db,db->rootPageNum,1,key,keysize);
    getDataByPageAndKey(db,dataPageNum,key,keysize,node);//默认 key value都是满足长度的数据 先不考虑不定长的数据
}

//seekType 特殊SeekNode获取定义  传1代表 取第一个date页第一个值  传2 代表取最后一页最后一个值  其余的默认按key找到对应所在页的对应的值
getSeekNodeByDbKey(struct FastTreeDb *db,byte key[],int keysize,struct SeekNode *node,int seekType){
    unsigned int dataPageNum = 0;
    if(seekType == 1){
        dataPageNum = selectFirstOrLastDatePageByDb(db,db->rootPageNum,1,1);
    }else if (seekType == 2){
        dataPageNum = selectFirstOrLastDatePageByDb(db,db->rootPageNum,1,2);
    }else{
        dataPageNum = selectPageByDbKey(db,db->rootPageNum,1,key,keysize);
    }
    if(dataPageNum <= 0 ){
        node->curPageNo = 0;
        return 0;
    }else{
        node->curPageNo = dataPageNum;
        //注释掉里面的malloc 在方法外初始化 以便于后续可直接释放结构体
//        node->pagedata = (byte *)malloc(db->pageSize);
        readDataByPageNum(db,node->pagedata,dataPageNum);
        struct PageBase *pageBase = node->pagedata;
        if(seekType == 1){
            node->seekindex = 1;//第一个
        }else if (seekType == 2){
            node->seekindex = pageBase->childNum;//最后一个
        }else{
            //开始查找  如果key是固定的情况下 直接从PageBase的start位置开始查找
            if(pageBase->childNum <= 0){//如果当前页已经没有节点数据 则直接取上一页的最后一个 前提是上一页存在
                //那么设置seek位置为0
                node->seekindex = 0;
                return 0;
            }
            if(db->keySize > 0 && db->valueSize > 0){//固定key
                //key数组起始位
                int keystart = sizeof(struct PageBase);
                int width = db->keySize+db->valueSize;
                //寻找key位置
                int start = selectIndexData(node->pagedata,pageBase->childNum,key,db->keySize,keystart,width,db->keytype);
                if(start < 0){//负数表示找到相等的key
                    node->seekindex = -start;//取反得到真实的key位置
                    return 0;
                }else{//此值不存在 假设 start为2 说明1位置比他小  2位置比他大 取1的位置
                    node->seekindex = start -1;
                    return 0;
                }
            } else {//处理非固定key
                //key数组起始位
                int keystart = sizeof(struct PageBase);
                //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
                //此页有个数 则查找对应的start位置
                //查找比较的时候 取key的长度比较  但是每个值的宽度位数等于 key+value的总长度 keystart为偏移量
                int start = selectDynamicIndexData(node->pagedata,pageBase->childNum,key,keysize,keystart,12,db->keytype);
                if(start < 0){//负数表示找到相等的key
                    node->seekindex = -start;//取反得到真实的key位置
                    return 0;
                }else{//此值不存在 直接插入对应位置
                    node->seekindex = start -1;
                    return 0;
                }
            }
        }
        return 0;
    }
}

//获取下一个key  type为1 表示获取下一个key的同时  next移动到对应位置 EntryNode 的 value 和 key的指针 使用完后要记得释放
//EntryNode 在外面初始化 可以避免方法内部malloc 上层使用可以更加灵活高效
int getNextBySeekNode(struct FastTreeDb *db,struct SeekNode *node,int type,struct EntryNode *entryNode){
        struct PageBase *pageBase = node->pagedata;
        //当前索引
        int curIndex = node->seekindex;
        //查找下一个
        if(curIndex < pageBase->childNum){//如果当前序号小于总数目 则直接在当前页返回
            setEntryNodeByIndex(entryNode,curIndex+1,node->pagedata,db->keySize,db->valueSize);
            if(entryNode->keyNode.keySize == -1){
                return 0;
            }
            if(type == 1 ){
                node->seekindex = curIndex+1;
            }
            return 1;
        }else{//当前页没有节点 或已经是最后一个节点 需要找寻下一页
            int hashnextpage = -1;
            byte b[db->pageSize];
            while (1){//找寻下一页 直到找到有节点的一页 1 true
                if(pageBase->nextPageNum == 0){//为0 直接跳出
                    break;
                }
                readDataByPageNum(db,b,pageBase->nextPageNum);
                pageBase = b;//赋值到新页的信息
                if(pageBase->childNum > 0){//节点大于0 则赋值跳出
                    hashnextpage = 1;
                    break;
                }
            }
            if(hashnextpage > 0 ){//表明找到了下一页 取下一页的第一个节点
                setEntryNodeByIndex(entryNode,1,b,db->keySize,db->valueSize);
                if(entryNode->keyNode.keySize == -1){
                    return 0;
                }
                if(type == 1){
                    node->seekindex = 1;
                    node->curPageNo = pageBase->curNo;
                    memmove(&node->pagedata[0],&b[0], db->pageSize);//copy key
                }
                return 1;
            }else{//未找到 设置-1 返回
                entryNode->keyNode.keySize = -1;
                entryNode->valueNode.valueSize = -1;
                return 0;
            }
        }
        return 0;
//    }
}


int getPreBySeekNode(struct FastTreeDb *db,struct SeekNode *node,int type,struct EntryNode *entryNode){
    struct PageBase *pageBase = node->pagedata;
    //当前索引序号
    int curIndex = node->seekindex;
    //查找下一个
    if(curIndex > 1 && pageBase->childNum > 0){//如果当前序号小于总数目 则直接在当前页返回
        setEntryNodeByIndex(entryNode,curIndex-1,node->pagedata,db->keySize,db->valueSize);
        if(entryNode->keyNode.keySize == -1){
            return 0;
        }
        if(type == 1 ){
            node->seekindex = curIndex-1;
        }
        return 1;
    }else{//当前页没有节点 或已经是第一个节点 需要找寻上一页
        int hashnextpage = -1;
        byte b[db->pageSize];
        while (1){//找寻上一页 直到找到有节点的一页 1 true
            if(pageBase->prePageNum == 0){//为0说明没有上一页了 直接跳出
                break;
            }
            readDataByPageNum(db,b,pageBase->prePageNum);
            pageBase = b;//赋值到新页的信息
            if(pageBase->childNum > 0){//节点大于0 则赋值跳出
                hashnextpage = 1;
                break;
            }
        }
        if(hashnextpage > 0 ){//表明找到了下一页 取下一页的最后一个节点
            setEntryNodeByIndex(entryNode,pageBase->childNum,b,db->keySize,db->valueSize);
            if(entryNode->keyNode.keySize == -1){
                return 0;
            }
            if(type == 1){
                node->seekindex = pageBase->childNum;
                node->curPageNo = pageBase->curNo;
                memmove(&node->pagedata[0],&b[0], db->pageSize);//copy page
            }
            return 1;
        }else{//未找到 设置-1 返回
            entryNode->keyNode.keySize = -1;
            entryNode->valueNode.valueSize = -1;
            return 0;
        }
    }
    return 0;
//    }
}


int getCurNode(struct FastTreeDb *db,struct SeekNode *node,struct EntryNode *entryNode){
    struct PageBase *pageBase = node->pagedata;
    //当前索引序号
    int curIndex = node->seekindex;
    //查找下一个
    if(curIndex > 0 && curIndex <=pageBase->childNum){//如果当前序号小于总数目 则直接在当前页返回
        setEntryNodeByIndex(entryNode,curIndex,node->pagedata,db->keySize,db->valueSize);
        if(entryNode->keyNode.keySize == -1){
            return 0;
        }
        return 1;
    }else{//当前页没有节点
        entryNode->keyNode.keySize = -1;
        entryNode->valueNode.valueSize = -1;
        return 0;
    }
//    }
}


void setEntryNodeByIndex(struct EntryNode *entryNode, int index, unsigned char *data, int keysize, int valuesize) {
    struct PageBase *pageBase = data;
    int keystart = sizeof(struct PageBase);
    if(index>pageBase->childNum){//取值异常
        entryNode->keyNode.keySize = -1;
        entryNode->valueNode.valueSize = -1;
        return;
    }
    int i = index -1;
    if(keysize > 0 && valuesize > 0){//固定长度值
        int width = keysize+valuesize;
//        if(entryNode->keyNode.keySize < keysize){//避免频繁分配内存
//            free(entryNode->keyNode.key);
//            entryNode->keyNode.key = malloc(keysize);
//        }
//        if(entryNode->valueNode.valueSize < keysize){//避免频繁分配内存
//            free(entryNode->valueNode.value);
//            entryNode->valueNode.value = malloc(valuesize);
//        }
        //使用realloc重新分配内存
        void *newkey_ptr = realloc(entryNode->keyNode.key, keysize);
        void *newvalue_ptr = realloc(entryNode->valueNode.value, valuesize);
        if (!newkey_ptr || !newvalue_ptr ) {// 内存分配失败 返回-1 错误处理。
            entryNode->keyNode.keySize = -1;
            entryNode->valueNode.valueSize = -1;
            return;
        }else{
            entryNode->keyNode.key = newkey_ptr;
            entryNode->valueNode.value = newvalue_ptr;
        }
        entryNode->keyNode.keySize = keysize;
        entryNode->valueNode.valueSize = valuesize;
        memmove(&entryNode->keyNode.key[0] ,&data[keystart+ i * width], entryNode->keyNode.keySize );//copy key
        memmove(&entryNode->valueNode.value[0] ,&data[keystart+ i * width +keysize], entryNode->valueNode.valueSize );//copy value
    }else{
//            printf("子数目序号 %d ",i);
        unsigned int *curoff  = data+keystart + i * 12; //前4位是offset 后4位是keysize 在后四位是valuesize
        unsigned int *curkeysize  =data+keystart + i * 12 + 4; //offset 后4位是keysize
        unsigned int *curvaluesize  = data+keystart + i * 12 + 8; //keysize 后4位是valuesize

//        if(entryNode->keyNode.keySize < keysize){//避免频繁分配内存
//            free(entryNode->keyNode.key);
//            entryNode->keyNode.key = malloc(*curkeysize);
//        }
//        if(entryNode->valueNode.valueSize < keysize){//避免频繁分配内存
//            free(entryNode->valueNode.value);
//            entryNode->valueNode.value = malloc( *curvaluesize);
//        }
        //使用realloc重新分配内存
        void *newkey_ptr = realloc(entryNode->keyNode.key, *curkeysize);
        void *newvalue_ptr = realloc(entryNode->valueNode.value, *curvaluesize);
        if (!newkey_ptr || !newvalue_ptr ) {// 内存分配失败 返回-1 错误处理。
            entryNode->keyNode.keySize = -1;
            entryNode->valueNode.valueSize = -1;
            return;
        }else{
            entryNode->keyNode.key = newkey_ptr;
            entryNode->valueNode.value = newvalue_ptr;
        }
        entryNode->keyNode.keySize = *curkeysize;
        entryNode->valueNode.valueSize = *curvaluesize;
        memmove(&entryNode->keyNode.key[0] , &data[*curoff],entryNode->keyNode.keySize );//copy key
//        memcpy(&data[*curoff],&entryNode->keyNode.key , entryNode->keyNode.keySize );//copy key
        memmove(&entryNode->valueNode.value[0] ,&data[*curoff+ *curkeysize], entryNode->valueNode.valueSize );//copy value
    }
}

// firstOrLast 1 first 2 last
unsigned int
selectFirstOrLastDatePageByDb(struct FastTreeDb *db,unsigned int pageNum,int type,int firstOrLast) {
    if(type == 1 || type == 2){//索引  1索引 2 索引叶子节点 data   3 data节点
        byte b[db->indexPageSize];
        readIndexByPageNum(db,b,pageNum);
        struct PageBase *pageBase = b;
        if(pageBase->childNum <= 0){// 此页没有个数 直接在此页插入一个新页
            // 目前索引不存在此情况
            return 0;
        }
        int start = 1;
        if(firstOrLast == 1){//first
            start = 1 ;
        }else if (firstOrLast == 2){//last
            start = pageBase->childNum;
        }else{//此情况不存在为非法
            return 0;
        }
        int keystart = sizeof(struct PageBase);
        //开始查找  如果key是固定的情况下 直接从PageBase的start位置开始查找
        if(db->keySize > 0){
            //key数组起始位
            byte *p = b+keystart + (start - 1) * (db->keySize + IndexKeySize) + db->keySize;
            unsigned int curpage = getIntBychar4(p);
            //继续往下找
            if(pageBase->type == 2){//本身是叶子节点 直接返回查到的page页
                return curpage;//直接返回此页
            }else{//非叶子节点 查找索引数据
                //递归查找页
                return selectFirstOrLastDatePageByDb(db,curpage,pageBase->type,firstOrLast);
            }
        }else{//非固定key value的索引情况
            unsigned int * curoff  = b+keystart + (start - 1) * 8; //前4位是offset 后4位是size
            unsigned int * curkeysize  = b+keystart + (start - 1) * 8 + 4; //前4位是offset 后4位是keysize
//                byte *p = b+keystart + (start - 1) * (db->keySize + IndexKeySize) + db->keySize;//mid为中间位
            byte *p = b+(*curoff) + (*curkeysize);//去除偏移量和size 后面紧跟着的就是页号
            unsigned int curpage = getIntBychar4(p);//页号
            //继续往下找
            if(pageBase->type == 2){//本身是叶子节点 直接返回查到的page页
                return curpage;//直接返回此页
            }else{//非叶子节点 查找索引数据
                //递归查找页
                return selectFirstOrLastDatePageByDb(db,curpage,pageBase->type,firstOrLast);
            }
        }
    }else{//data
        return 0;
    }
}

void
getDataByPageAndKey(struct FastTreeDb *db, unsigned int dataPageNum, unsigned char *key, int keysize,
                    struct ValueNode *valueNode) {

    //查出某页数据 修改或插入值  然后重新写入
    byte b[db->pageSize];
    readDataByPageNum(db,b,dataPageNum);
    struct PageBase *pageBase = b;
    //开始查找  如果key是固定的情况下 直接从PageBase的start位置开始查找
    if(db->keySize > 0 && db->valueSize > 0){//1期暂时先不考虑不固定key 和 不固定value的情况
        //key数组起始位
        int keystart = sizeof(struct PageBase);
        int width = db->keySize+db->valueSize;
        //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
        //寻找插入位置
        int start = 0;//默认在0的位置插入
        if(pageBase->childNum > 0){//如果已经包含部分子节点，则搜索该节点应该插入的位置
            //此页有个数 则查找对应的start位置
            //查找比较的时候 取key的长度比较  但是每个值的宽度位数等于 key+value的总长度 keystart为偏移量
            start = selectIndexData(b,pageBase->childNum,key,db->keySize,keystart,width,db->keytype);
            if(start < 0){//负数表示找到相等的key
                start = -start;//取反得到真实的key位置
                valueNode->valueSize = db->valueSize;
                valueNode->value = (byte *)malloc(valueNode->valueSize);
                memmove(&valueNode->value[0],&db[keystart + (start-1) * width + db->keySize],  valueNode->valueSize);//32个字节
            }else{//此值不存在 直接插入对应位置
                //直接设置负一返回
                valueNode->valueSize = -1;
                return;
            }
        }else{//节点是空的 直接设置-1返回
            //直接设置负一返回
            valueNode->valueSize = -1;
            return;
        }
    }else{//处理非固定key
        //key数组起始位
        int keystart = sizeof(struct PageBase);
        //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
        //寻找插入位置
        int start = 0;//默认在0的位置插入
        if(pageBase->childNum > 0){//如果已经包含部分子节点，则搜索该节点应该插入的位置
            //此页有个数 则查找对应的start位置
            //查找比较的时候 取key的长度比较  但是每个值的宽度位数等于 key+value的总长度 keystart为偏移量
            start = selectDynamicIndexData(b,pageBase->childNum,key,keysize,keystart,12,db->keytype);
            if(start < 0){//负数表示找到相等的key
                start = -start;//取反得到真实的key位置

                //获取指定位置的key的起始位和size
                int indexstart = start -1;//前面的位数 也可以说是数组index序号
                unsigned int *curoff  = db+keystart + indexstart * 12; //前4位是offset 后4位是keysize 在后四位是valuesize
                unsigned int *curkeysize  = db+keystart + indexstart * 12 + 4; //前4位是offset 后4位是keysize
                unsigned int *curvaluesize  = db+keystart + indexstart * 12 + 8; //前4位是offset 后4位是keysize 在后四位是valuesize
                valueNode->valueSize = *curvaluesize;
                valueNode->value = (byte *)malloc(valueNode->valueSize);
                memmove(&valueNode->value[0],&db[*curoff + *curkeysize],  valueNode->valueSize);//32个字节
            }else{//此值不存在 直接插入对应位置
                //直接设置负一返回
                valueNode->valueSize = -1;
                return;
            }
        }else{//节点是空的 直接设置-1返回
            //直接设置负一返回
            valueNode->valueSize = -1;
            return;
        }
    }
    return ;
}

void opendb(struct FastTreeDb *db){
    char indexdbname[1024]= "";
    strcat(indexdbname,db->filename);
    strcat(indexdbname,DB_INDEX);
    int fd = open(indexdbname,   O_CREAT | O_RDWR | O_APPEND , 0644);
    db->indexfd = fd;

    char rootdbname[1024]= "";
    strcat(rootdbname,db->filename);
    strcat(rootdbname,DB_INFO);
    int fd2 = open(rootdbname,   O_CREAT | O_RDWR | O_APPEND , 0644);
    db->dbinfofd = fd2;

    char databname[1024]= "";
    strcat(databname,db->filename);
    strcat(databname,DB_DATA);
    int fd3 = open(databname,   O_CREAT | O_RDWR | O_APPEND , 0644);
    db->datafd = fd3;
}
void closedb(struct FastTreeDb *db){
    close(db->datafd);
    close(db->indexfd);
    close(db->dbinfofd);
}
void intsertDataPageByKeyValue(struct FastTreeDb *db,unsigned int pageNum,int type, byte key[],int keysize,byte value[],int valuesize) {
    //定义一个方法 从指定页查找 找到数据应该对应插入的data页 页号
    unsigned int dataPageNum = selectPageByDbKey(db,pageNum,type,key,keysize);
    int start = intsertDataByDataPage(db,dataPageNum,key,value,keysize,valuesize);//默认 key value都是满足长度的数据 先不考虑不定长的数据
}

int intsertDataByDataPage(struct FastTreeDb *db, unsigned int pageNum, byte key[], byte value[],int keysize,int valuesize) {
    //查出某页数据 修改或插入值  然后重新写入
    byte b[db->pageSize];
    readDataByPageNum(db,b,pageNum);
    struct PageBase *pageBase = b;
    showPageBase(pageBase);//打印，显示 正式运行去掉

    //开始查找  如果key是固定的情况下 直接从PageBase的start位置开始查找
    if(db->keySize > 0 && db->valueSize > 0){//1期暂时先不考虑不固定key 和 不固定value的情况
        //key数组起始位
        int keystart = sizeof(struct PageBase);
        int width = db->keySize+db->valueSize;
        //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
        //寻找插入位置
        int start = 0;//默认在0的位置插入
        if(pageBase->childNum > 0){//如果已经包含部分子节点，则搜索该节点应该插入的位置
            //此页有个数 则查找对应的start位置
            //查找比较的时候 取key的长度比较  但是每个值的宽度位数等于 key+value的总长度 keystart为偏移量
            start = selectIndexData(b,pageBase->childNum,key,db->keySize,keystart,width,db->keytype);
            if(start < 0){//表示找到和此key一模一样的key 则直接更新覆盖
                //覆盖的话 只需要更改对应位置的value即可
                //memmove(&b[0], &pageBase, size);//32个字节
                start = -start;
                //将key value值写入
                setkeytoDataPageByte(db,b,key,value,start,1);
                writeDataByPageNum(db,b,pageNum);
            }else{//此值不存在 直接插入对应位置
                //首先判断此页是否已经满了  如果满了，则此页需要分裂 此值需要插入到分裂后的页
                if(pageBase->childNum >= db->maxNodeNum){//这里逻辑先不动
                    //如果子页的数目已经满了 则执行分页操作 返回新增的页号
                    unsigned int splitPageNum  = splitDataPage(db,pageNum);//参数 db 和 page页  返回值返回下一页的页号
                    //如果start的位置 大于 maxNodeNum/2 的一半 则插入到下一页 否则插入本页 128 64  65
                    //第一页最后一个元素为maxNodeNum/2  第二页的第一个元素 为 maxNodeNum/2 + 1
                    if (start > db->maxNodeNum/2 +1){//start比maxNodeNum/2 + 1的元素大的时候 才插入第二页 这样不影响上级索引变化
                        return intsertDataByDataPage(db,splitPageNum,key,value,keysize,valuesize);//递归一次 插入到下页
                    }else{//否则追加到本页最后元素为maxNodeNum/2 的后面 序号为 maxNodeNum/2+1
                        return intsertDataByDataPage(db,pageNum,key,value,keysize,valuesize);//递归一次 插入本页
                    }
                }else{//数目正常
                    //将key value值写入
                    setkeytoDataPageByte(db,b,key,value,start,0);
                    writeDataByPageNum(db,b,pageNum);
                    //如果start是第一个key  且如果有上级的索引的话 还需要更改下上级索引的page对应的key值
                    if(start == 1 && pageBase->parentPageNum > 0){//当父级页不为空 且 此key是第一个元素 则需要把对应的父级页的索引key也改下 改为第一个
                        //目前的结构 一般不会出现这种情况 先预留此逻辑
                    }
                }
            }
        }else{//节点是空的 正常是不会为空的 只有为root的情况下，默认是一个空页
            setkeytoDataPageByte(db,b,key,value,1,0);
            writeDataByPageNum(db,b,pageNum);
        }
    }else{//处理非固定key
        //key数组起始位
        int keystart = sizeof(struct PageBase);
        unsigned int * maxdataoff = b+keystart + db->dynamicDateNum  * 12;//总大小偏移量
        //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
        //寻找插入位置
        int start = 0;//默认在0的位置插入
//        printf("chilenum22 %d  dynamicDateNum %d  keysize +valuesize %d maxdataoff %d \n",pageBase->childNum,db->dynamicDateNum,keysize +valuesize,(*maxdataoff));
        if(pageBase->childNum > 0){//如果已经包含部分子节点，则搜索该节点应该插入的位置
            //此页有个数 则查找对应的start位置
            //查找比较的时候 取key的长度比较  但是每个值的宽度位数等于 key+value的总长度 keystart为偏移量
            start = selectDynamicIndexData(b,pageBase->childNum,key,keysize,keystart,12,db->keytype);
            //首先判断此页是否已经满了  如果满了，则此页需要分裂 此值需要插入到分裂后的页

            //数目超过索引数 或者空间满了 都需要分裂
//            printf("chilenum %d  dynamicDateNum %d  keysize +valuesize %d maxdataoff %d \n",pageBase->childNum,db->dynamicDateNum,keysize +valuesize,(*maxdataoff));
            if(pageBase->childNum >= db->dynamicDateNum || keysize +valuesize >= db->pageSize - (*maxdataoff)){

                //如果子页的数目已经满了 则执行分页操作 返回新增的页号
                unsigned int splitPageNum  = splitDynamicDataPage(db,pageNum);//参数 db 和 page页  返回值返回下一页的页号
                //如果start的位置 大于 maxNodeNum/2 的一半 则插入到下一页 否则插入本页 128 64  65

                //第一页最后一个元素为maxNodeNum/2  第二页的第一个元素 为 maxNodeNum/2 + 1
                if(start < 0){
                    start = -start +1;//以此位置的后一位来判断
                }
                // || (-start) > pageBase->childNum/2
                if (start > pageBase->childNum/2 +1 ){//start比childNum/2 + 1的元素大的时候 才插入第二页 这样不影响上级索引变化
                    return intsertDataByDataPage(db,splitPageNum,key,value,keysize,valuesize);//递归一次 插入到下页
                }else{//否则追加到本页最后元素为maxNodeNum/2 的后面 序号为 maxNodeNum/2+1
                    return intsertDataByDataPage(db,pageNum,key,value,keysize,valuesize);//递归一次 插入本页
                }
            }else{//数目正常
                if(start < 0){//表示找到和此key一模一样的key 则直接更新覆盖
                    //覆盖的话 只需要更改对应位置的value即可
                    //memmove(&b[0], &pageBase, size);//32个字节
                    start = -start;
                    setDynamicData(db,b,key,keysize,value,valuesize,start,1);
                    writeDataByPageNum(db,b,pageNum);
                }else{//此值不存在 直接插入对应位置
                    //将key value值写入
//                    printf("\n%d节点新增后总偏移量大小 %d\n",pageBase->childNum,*(int*)(b+keystart + db->dynamicDateNum  * 12) );
                    setDynamicData(db,b,key,keysize,value,valuesize,start,0);
                    writeDataByPageNum(db,b,pageNum);
                    //如果start是第一个key  且如果有上级的索引的话 还需要更改下上级索引的page对应的key值
//                    printf("\n%d节点新增后总偏移量大小 %d\n",pageBase->childNum,*(int*)(b+keystart + db->dynamicDateNum  * 12) );
                    if(start == 1 && pageBase->parentPageNum > 0){//当父级页不为空 且 此key是第一个元素 则需要把对应的父级页的索引key也改下 改为第一个
                        //目前的结构 一般不会出现这种情况 先预留此逻辑
                    }
                }
            }
        }else{//节点是空的 正常是不会为空的 只有为刚初始化的情况下，默认是一个空页
            //如果页之前是空的 将空间使用情况设置一下，设置为初始值的起始偏移量
            *maxdataoff =  keystart + db->dynamicDateNum  * 12 + 4;//总大小更新
            setDynamicData(db,b,key,keysize,value,valuesize,1,0);
//            printDynamicDatePage(db,b);
            writeDataByPageNum(db,b,pageNum);
        }
    }

    return pageNum;
}

void setkeytoDataPageByte(struct FastTreeDb *db, byte data[],byte *key, byte *value, int start,
                 int isupdate) {
    int keystart = sizeof(struct PageBase);
    int width = db->keySize+db->valueSize;
    if(isupdate == 1){//覆盖值
        memmove(&data[keystart + (start-1) * width + db->keySize], &value[0], db->valueSize);//32个字节
    }else{//插入新值
        //首先改动最大数目+1
        struct PageBase *pageBase = data;
        showPageBase(pageBase);
        pageBase->childNum = pageBase->childNum+1;
        int startsize = keystart + (start-1) * width;
        //然后将start后面的元素集体后移
        if(start < pageBase->childNum){//start的位置不是最后一位  才需要后移
            memmove(&data[startsize + width], &data[startsize], (pageBase->childNum-start) * width);// 0 1 2   3
        }
        memmove(&data[startsize], &key[0], db->keySize);//设置当前值
        memmove(&data[startsize+db->keySize], &value[0], db->valueSize);//设置当前值
    }
}
void setDynamicData(struct FastTreeDb *db, byte data[],byte *key, int keysize,byte *value,int valuesize, int start,
                          int isupdate) {
    int keystart = sizeof(struct PageBase);
    //获取指定位置的key的起始位和size
    int indexstart = start -1;//索引位为start-1
    if(isupdate == 1){//覆盖值
        //判断移动的位数大小 覆盖的位数为两个值的差值
        unsigned int *curoff  = data+keystart + indexstart * 12; //前4位是offset 后4位是keysize 在后四位是valuesize
        unsigned int *curkeysize  = data+keystart + indexstart * 12 + 4; //前4位是offset 后4位是keysize
        unsigned int *curvaluesize  = data+keystart + indexstart * 12 + 8; //前4位是offset 后4位是keysize

        int chazhi =  keysize + valuesize - (*curkeysize) - (*curvaluesize);
        //根据差值 和 起始位 开始移动位置  从偏移量到当前位置开始copy
        // 移动的位数 = 全长 - 移动copy位  覆盖操作，移动后从下一个key的偏移量开始更改，本偏移量不变
        unsigned int * maxdataoff = data+keystart + db->dynamicDateNum  * 12;
        moveDataPage(data,chazhi,indexstart +1 ,(int)(*curoff) + keysize + valuesize,(int)*maxdataoff);

        //设置当前数目
        *curkeysize = keysize;//设置当前key值
        *curvaluesize = valuesize;//设置当前value值
        //设置当前值
        memmove(&data[*curoff], &key[0], keysize);//设置当前key值
        memmove(&data[(*curoff)+keysize], &value[0],valuesize);//设置当前value值
        *maxdataoff = *(maxdataoff) + chazhi;//总大小更新
    }else{//插入新值
        //首先改动最大数目+1
        struct PageBase *pageBase = data;
        showPageBase(pageBase);
        //移动偏移量
        unsigned int * maxdataoff = data+keystart + db->dynamicDateNum  * 12;
        unsigned int *curoff = data+keystart + db->dynamicDateNum  * 12;//默认偏移量设为空间存储的最大值 不要+4
        int startsize = keystart + indexstart * 12;
        unsigned int curoffnum = *curoff;
        if(indexstart < pageBase->childNum){//start的位置不是当前位置之后  才需要后移
            curoff = data+startsize;//start位置原值的偏移量 设为当前值存储的偏移量位置
            curoffnum = *curoff;
            moveDataPage(data,keysize +valuesize,indexstart,(int)(*curoff) + keysize + valuesize,(int)(*maxdataoff));
            //这里移动start后面的索引位
            memmove(&data[startsize + 12], &data[startsize], (db->dynamicDateNum-start) * 12);//0 1 2   1
        }
        pageBase->childNum = pageBase->childNum+1;
        //设置当前索引偏移量 12位
        memmove(&data[startsize], &curoffnum, 4);//设置当前索引偏移量
        memmove(&data[startsize+4], &keysize,4);//设置当前key size值
        memmove(&data[startsize+8], &valuesize,4);//设置当前value size值

        //设置当前值
        memmove(&data[*curoff], &key[0], keysize);//设置当前key值
        memmove(&data[*curoff+keysize], &value[0],valuesize);//设置当前value值

        *maxdataoff = *(maxdataoff) + keysize +valuesize;//总大小更新
    }
}

//此方法一定是在childNum变更前调用
void moveDataPage(unsigned char *data, int chazhi, int start,int startoff,int maxdataoff) {
    if(chazhi == 0){
        return;
    }
    //chazhi<0 往前移动 >0  往后移动
    if(chazhi > 0){//后移  后移的位数 = 最大偏移位置 - 被copy起始位置 = maxdataoff - (startoff - chazhi)
        memmove(&data[startoff], &data[startoff - chazhi], maxdataoff - startoff + chazhi);//移动差值的位数
    }else if(chazhi < 0){//前移
        //if理论上可以去掉了
        memmove(&data[startoff], &data[startoff-chazhi], maxdataoff - startoff +chazhi);//迁移
    }
    //移动后 更改start位置后面的所有key value的偏移量
    struct PageBase *pageBase = data;
    int keystart = sizeof(struct PageBase);
    for(int i = start;i<pageBase->childNum;i++){//移动偏移量
        //偏移量也得改
        int * curoff = data+keystart + i * 12;
        *curoff = *curoff + chazhi;//curkeysize  只更改起始偏移量  key valuesize 都没有变化
//        int * curkeysize = data+keystart + start * 12+4;
//        *curkeysize = *curkeysize + chazhi;//curkeysize
//        int * curvaluesize = data+keystart + start * 12+8;
//        *curvaluesize = *curvaluesize + chazhi; //curvaluesize
    }
}

static void moveIndexPage(unsigned char *data, int chazhi, int start,int startoff,int maxdataoff) {
    if(chazhi == 0){
        return;
    }
    //chazhi<0 往前移动 >0  往后移动
    if(chazhi > 0){//后移  后移的位数 = 最大偏移位置 - 被copy起始位置 = maxdataoff - (startoff - chazhi)
        memmove(&data[startoff], &data[startoff - chazhi], maxdataoff - startoff + chazhi);//移动差值的位数
    }else if(chazhi < 0){//前移
        memmove(&data[startoff], &data[startoff - chazhi], maxdataoff - startoff + chazhi);//迁移
    }
    //移动后 更改start位置后面的所有key value的偏移量
    struct PageBase *pageBase = data;
    int keystart = sizeof(struct PageBase);
    for(int i = start;i<pageBase->childNum;i++){//移动偏移量
        //偏移量也得改
        int * curoff = data+keystart + i * 8;
        *curoff = *curoff + chazhi;//curkeysize  只更改起始偏移量  key valuesize 都没有变化
    }
}

static unsigned int splitDataPage(struct FastTreeDb *db, unsigned int pageNum) {
    unsigned int nextpage = getNextDataPage(db);//获取下一页的页号
    byte b[db->pageSize];
    //读取待分裂页的数据
    readDataByPageNum(db,b,pageNum);
    struct PageBase *pageBase = b;
    //设置分裂后的本页
    int zongshu = pageBase->childNum;
    pageBase->childNum = zongshu/2;//数目设置为相对应的一半
    int oldnextpage =  pageBase->nextPageNum;
    pageBase->nextPageNum = nextpage;
    if(pageBase->parentPageNum <= 0){//首先检查父级页号是否存在   目前的设计是必定会存在父级页 理论上不会走这里
        pageBase->parentPageNum =  getNextIndexPage(db);//获取父级页
    }
    //设置完毕 写入当前页
    writeDataByPageNum(db,b,pageNum);

    //然后更改byte的的值为分裂后的页值
    //先设置页号  本页 上一页 和 下一页  父页的
    pageBase->prePageNum = pageBase->curNo;//上一页设置为原本的页号
    pageBase->curNo = nextpage;//本页设置为新生成的页号
    pageBase->nextPageNum = oldnextpage;//下一页设置为原本的的下页号
    pageBase->childNum = zongshu - pageBase->childNum;
    //将childNum/2节点之后的节点数据 复制到起始节点
    int keystart = sizeof(struct PageBase);
    int width = db->keySize+db->valueSize;
    int startsize = keystart + (zongshu - pageBase->childNum) * width;//需要移动多少个key 起始位就跳过多少
    //然后将start后面的元素集体指定迁移
    memmove(&b[keystart], &b[startsize], (pageBase->childNum) * width);//设置data值
    writeDataByPageNum(db,b,nextpage);

    //oldnextpage 这一页变成了分裂后的下一页了，那么他的上一页也得修改一下
    if(oldnextpage != 0) {
        byte oldnextpagedata[db->pageSize];
        readDataByPageNum(db, oldnextpagedata, oldnextpage);
        struct PageBase *opagebase = oldnextpagedata;
        opagebase->prePageNum = pageBase->curNo;
        writeDataByPageNum(db, oldnextpagedata, oldnextpage);
    }
    //写入完成后，开始设置父级页索引的字节点，将新增的页添加进去
    byte * key = &b[keystart];
    insertPageAndKeyIndex(db,pageBase->parentPageNum,key,nextpage,db->keySize);//type 为2 新加入的key和page页
    return nextpage;
}


unsigned int splitDynamicDataPage(struct FastTreeDb *db, unsigned int pageNum) {
    unsigned int nextpage = getNextDataPage(db);//获取下一页的页号
    byte b[db->pageSize];
    int keystart = sizeof(struct PageBase);
    //读取待分裂页的数据
    readDataByPageNum(db,b,pageNum);
    struct PageBase *pageBase = b;
    //设置分裂后的本页
    int zongshu = pageBase->childNum;
    pageBase->childNum = zongshu/2;//数目设置为相对应的一半
    int oldnextpage =  pageBase->nextPageNum;
    pageBase->nextPageNum = nextpage;
    if(pageBase->parentPageNum <= 0){//首先检查父级页号是否存在   目前的设计是必定会存在父级页 理论上不会走这里
        pageBase->parentPageNum =  getNextIndexPage(db);//获取父级页
    }
    //获取截取对象起始位置的偏移量 将page的偏移量设置为此节点的起始偏移量
    unsigned int *maxoff = b+keystart + db->dynamicDateNum  * 12;//获取空间存储的最大值 不要+4
    int maxvalue = ( *maxoff );//保存一下原值的总偏移量
    unsigned int *curoff  = b + keystart + (pageBase->childNum) * 12;//获取下一个key的起始偏移量
//    for(int a = 0;a<24;a++){
//        unsigned int *aoff  = b + keystart + a * 12;
//        printf("第%d个偏移量遍历%d\n",a,*aoff );
//    }
    *maxoff = (*curoff);//设置一下当前页的使用偏移量
    int curvalue = (*curoff);//保存一下现在值的使用偏移量
    //设置完毕 写入当前页
    writeDataByPageNum(db,b,pageNum);
//    printf("分裂后的偏移量占用%d",*maxoff );
    //然后更改byte的的值为分裂后的页值
    //先设置页号  本页 上一页 和 下一页  父页的
    pageBase->prePageNum = pageBase->curNo;//上一页设置为原本的页号
    pageBase->curNo = nextpage;//本页设置为新生成的页号
    pageBase->nextPageNum = oldnextpage;//下一页设置为原本的的下页号
    pageBase->childNum = zongshu - pageBase->childNum;

    //将childNum/2节点之后的节点数据 复制到起始节点

    //1 先移动索引位置 假设14个元素 起始位为0位置 copy位7位置 copy 7个元素 前7后7  假设15个元素 起始位为0位置 copy位8位置 copy 8个元素 前7后8
//    printf("keystart %d qishiwei %d shuzhi %d \n",keystart,keystart + (zongshu - pageBase->childNum)*12 ,pageBase->childNum * 12);
    memmove(&b[keystart], &b[keystart + (zongshu - pageBase->childNum) * 12], pageBase->childNum * 12);
    //前移  memmove(&b[keystart], &b[keystart + (zongshu - pageBase->childNum) * width], (pageBase->childNum) * width);
    //2 在移动偏移量数值位置 复制起始位为上一页已经使用的偏移量  迁移位数为总长度减去上一页的长度
    int chazhi = maxvalue - curvalue;
    int dataheadSize = keystart + db->dynamicDateNum  * 12+4;
    memmove(&b[dataheadSize], &b[curvalue],chazhi );

    //移动后 更改所有索引位置后面的 所有key value的偏移量
//    printf("maxvalue %d curvalue %d chazhi %d \n",maxvalue,curvalue,chazhi);
    for(int i = 0;i<pageBase->childNum;i++){//移动偏移量
        //偏移量也得改
        int * curkeyoff = b+keystart + i * 12;//原偏移量
//        printf("*curkeyoff  %d nowoff %d \n",*curkeyoff , *curkeyoff - curvalue);
        *curkeyoff = *curkeyoff - curvalue + dataheadSize;//curkeysize  只更改起始偏移量  key valuesize 都没有变化
    }
    *maxoff = chazhi + dataheadSize;//设置一下当前页的使用偏移量
//    printf("分裂后的下一页偏移量占用%d",*maxoff );
    //写入此页
    writeDataByPageNum(db,b,nextpage);

    //oldnextpage 这一页变成了分裂后的下一页了，那么他的上一页也得修改一下
    if(oldnextpage != 0) {
        byte oldnextpagedata[db->pageSize];
        readDataByPageNum(db, oldnextpagedata, oldnextpage);
        struct PageBase *opagebase = oldnextpagedata;
        opagebase->prePageNum = pageBase->curNo;
        writeDataByPageNum(db, oldnextpagedata, oldnextpage);
    }

    //写入完成后，开始设置父级页索引的字节点，将新增的页添加进去
    unsigned int *startkeyoff  = b+keystart; //起始key的偏移量
    int *curkeysize  = b+keystart + 4; //前4位是offset 后4位是keysize
    byte * key = b+(*startkeyoff);//这个是key真是值的起始位
    insertPageAndKeyIndex(db,pageBase->parentPageNum,key,nextpage,(*curkeysize));//type 为2 新加入的key和page页
    return nextpage;
}

static unsigned int splitDynamicIndexPage(struct FastTreeDb *db, unsigned int indexnum) {

    unsigned int nextpage = getNextIndexPage(db);//获取下一页的页号
    byte b[db->indexPageSize];
    int keystart = sizeof(struct PageBase);
    //读取待分裂页的数据
    readIndexByPageNum(db,b,indexnum);
    struct PageBase *pageBase = b;

    //设置分裂后的本页
    int zongshu = pageBase->childNum;
    pageBase->childNum = zongshu/2;//数目设置为相对应的一半
    unsigned int oldnextpage =  pageBase->nextPageNum;
    pageBase->nextPageNum = nextpage;
    if(pageBase->parentPageNum <= 0){//首先检查父级页号是否存在
        //如果不存在  那说明原本的页为父级root页
        pageBase->parentPageNum =  getNextIndexPage(db);//获取父级页
        //此时需要对其进行初始化 即将第一个key和本页页号作为第一个元素放入到新生成的对应的root页中
        //写入完成后，开始设置父级页索引的字节点，将新增的页添加进去
        unsigned int *startkeyoff  = b+keystart; //起始key的偏移量
        int *curkeysize  = b+keystart + 4; //前4位是offset 后4位是keysize
        byte * key = b+(*startkeyoff);//这个是key真是值的起始位
        initRootIndexBykeyPage(db,pageBase->parentPageNum,key,indexnum,(*curkeysize));//加入并初始化新root页
    }
    //获取截取对象起始位置的偏移量 将page的偏移量设置为此节点的起始偏移量
    unsigned int *maxoff = b+keystart + db->dynamicDateNum  * 8;//获取空间存储的最大值 不要+4
    int maxvalue = ( *maxoff );//保存一下原值的总偏移量
    unsigned int *curoff  = b + keystart + (pageBase->childNum) * 8;//获取下一个key的起始偏移量
    *maxoff = (*curoff);//设置一下当前页的使用偏移量
    int curvalue = (*curoff);//保存一下现在值的使用偏移量
    //设置完毕 写入当前页
    writeIndexByPageNum(db,b,indexnum);


    //然后更改byte的的值为分裂后的页值
    //先设置页号  本页 上一页 和 下一页  父页的
    pageBase->prePageNum = pageBase->curNo;//上一页设置为原本的页号
    pageBase->curNo = nextpage;//本页设置为新生成的页号
    pageBase->nextPageNum = oldnextpage;//下一页设置为原本的下页号
    pageBase->childNum = zongshu - pageBase->childNum;


    //将childNum/2节点之后的节点数据 复制到起始节点
    //1 先移动索引位置 假设14个元素 起始位为0位置 copy位7位置 copy 7个元素 前7后7  假设15个元素 起始位为0位置 copy位8位置 copy 8个元素 前7后8
    memmove(&b[keystart], &b[keystart + (zongshu - pageBase->childNum) * 8], pageBase->childNum * 8);
    //前移  memmove(&b[keystart], &b[keystart + (zongshu - pageBase->childNum) * width], (pageBase->childNum) * width);
    //2 在移动偏移量数值位置 复制起始位为上一页已经使用的偏移量  迁移位数为总长度减去上一页的长度
    int chazhi = maxvalue - curvalue;
    int dataheadSize = keystart + db->dynamicDateNum  * 8+4;
    memmove(&b[dataheadSize], &b[curvalue],chazhi );

    //移动后 更改所有索引位置后面的 所有key value的偏移量
    for(int i = 0;i<pageBase->childNum;i++){//移动偏移量
        //偏移量也得改
        int * curkeyoff = b+keystart + i * 8;//原偏移量
        *curkeyoff = *curkeyoff - curvalue + dataheadSize;//curkeysize  只更改起始偏移量  key valuesize 都没有变化
    }
    *maxoff = chazhi + dataheadSize;//设置一下当前页的使用偏移量
    writeIndexByPageNum(db,b,nextpage);

    if(oldnextpage != 0){
        byte oldnextdata[db->indexPageSize];
        readIndexByPageNum(db,oldnextdata,oldnextpage);
        struct PageBase *opagebase = oldnextdata;
        opagebase->prePageNum =  pageBase->curNo;
        writeIndexByPageNum(db,oldnextdata,oldnextpage);
    }

    //写入完成后，开始设置父级页索引的字节点，将新增的页添加进去
    unsigned int *startkeyoff  = b+keystart; //起始key的偏移量
    int *curkeysize  = b+keystart + 4; //前4位是offset 后4位是keysize
    byte * key = b+(*startkeyoff);//这个是key真是值的起始位
    insertPageAndKeyIndex(db,pageBase->parentPageNum,key,nextpage,(*curkeysize));//type 为2 新加入的key和page页

    //添加进去后 需要把新页的所有子child的父节点设置处理下
    for(int i = 0;i<pageBase->childNum;i++){//每个子节点都需要重新设置父页
        int * childkeyoff = b+keystart + i * 8;//获取指向当前节点的偏移量
        int *childkeysize  = b+keystart + i * 8 + 4;//keysize
        byte * pagestart = b+(*childkeyoff) + (*childkeysize);//当前key的页号值存储的真正起始位指针
        unsigned int childPage= getIntBychar4(pagestart);
        if(pageBase->type == 1){//下级为索引
            byte childdata[db->indexPageSize];
            readIndexByPageNum(db,childdata,childPage);
            struct PageBase *cpagebase = childdata;
            cpagebase->parentPageNum = nextpage;
            writeIndexByPageNum(db,childdata,childPage);
        }else{//下级为data页
            byte childdata[db->pageSize];
            readDataByPageNum(db,childdata,childPage);
            struct PageBase *cpagebase = childdata;
            cpagebase->parentPageNum = nextpage;
            writeDataByPageNum(db,childdata,childPage);
        }
    }
    return nextpage;
}
static unsigned int splitIndexPage(struct FastTreeDb *db, unsigned int indexnum) {
    unsigned int nextpage = getNextIndexPage(db);//获取下一页的页号
    byte b[db->indexPageSize];
    readIndexByPageNum(db,b,indexnum);
    struct PageBase *pageBase = b;
    //设置分裂后的本页
    int zongshu = pageBase->childNum;
    pageBase->childNum = zongshu/2;//数目设置为相对应的一半
    unsigned int oldnextpage =  pageBase->nextPageNum;
    pageBase->nextPageNum = nextpage;
    if(pageBase->parentPageNum <= 0){//首先检查父级页号是否存在
        //如果不存在  那说明原本的页为父级root页
        pageBase->parentPageNum =  getNextIndexPage(db);//获取父级页
        //此时需要对其进行初始化 即将第一个key和本页页号作为第一个元素放入到对应的root页中
        //写入完成后，开始设置父级页索引的字节点，将新增的页添加进去
        int keystart = sizeof(struct PageBase);
        byte * key = &b[keystart];
        initRootIndexBykeyPage(db,pageBase->parentPageNum,key,indexnum,db->keySize);//type 为2 新加入的key和page页
    }
    //设置完毕 写入当前页
    writeIndexByPageNum(db,b,indexnum);
    //然后更改byte的的值为分裂后的页值
    //先设置页号  本页 上一页 和 下一页  父页的
    pageBase->prePageNum = pageBase->curNo;//上一页设置为原本的页号
    pageBase->curNo = nextpage;//本页设置为新生成的页号
    pageBase->nextPageNum = oldnextpage;//下一页设置为原本的下页号
    pageBase->childNum = zongshu - pageBase->childNum;

    //将childNum/2节点之后的节点数据 复制到起始节点
    int keystart = sizeof(struct PageBase);
//    printf("第一个key %d \n",getIntBychar4( &b[keystart]));
    int width = db->keySize+IndexKeySize;
//    printf("最后一个key %d \n",getIntBychar4(&b[keystart+(zongshu-1) * width]));
    int startsize = keystart + (zongshu - pageBase->childNum) * width;//需要移动多少个key 起始位就跳过多少 407 203 204 203
    //然后将start后面的元素集体指定后移
    memmove(&b[keystart], &b[startsize], (pageBase->childNum) * width);//设置data值
    writeIndexByPageNum(db,b,nextpage);

    //oldnextpage 这一页变成了分裂后的下一页了，那么他的上一页也得修改一下
    if(oldnextpage != 0){
        byte oldnextdata[db->indexPageSize];
        readIndexByPageNum(db,oldnextdata,oldnextpage);
        struct PageBase *opagebase = oldnextdata;
        opagebase->prePageNum =  pageBase->curNo;
        writeIndexByPageNum(db,oldnextdata,oldnextpage);
    }
    //写入完成后，开始设置父级页索引的子节点，将新增的页添加进去
    byte * key = &b[keystart];
    insertPageAndKeyIndex(db,pageBase->parentPageNum,key,nextpage,db->keySize);//type 为2 新加入的key和page页

    //添加进去后 需要把新页的所有子child的父节点设置处理下
    for(int i = 0;i<pageBase->childNum;i++){//每个子节点都需要重新设置父页
        byte * pagestart = &b[keystart +  (db->keySize + IndexKeySize) * i + db->keySize];//当前节点页号起始位
        unsigned int childPage= getIntBychar4(pagestart);
        if(pageBase->type == 1){//下级为索引
            byte childdata[db->indexPageSize];
            readIndexByPageNum(db,childdata,childPage);
            struct PageBase *cpagebase = childdata;
            cpagebase->parentPageNum = nextpage;
            writeIndexByPageNum(db,childdata,childPage);
        }else{//下级为data页
            byte childdata[db->pageSize];
            readDataByPageNum(db,childdata,childPage);
            struct PageBase *cpagebase = childdata;
            cpagebase->parentPageNum = nextpage;
            writeDataByPageNum(db,childdata,childPage);
        }
    }
    return nextpage;
}

static unsigned int getNextDataPage(struct FastTreeDb *db) {
    db->maxDataPageNum = db->maxDataPageNum + 1;
    ssize_t t =pwrite(db->dbinfofd,db,sizeof(struct FastTreeDb),0);//从0的位置开始写
    return db->maxDataPageNum;
}

static unsigned int insertPageAndKeyIndex(struct FastTreeDb *db, unsigned int indexnum,byte *key, unsigned int childpage,int keysize) {
    //首先读取此索引页数据
    byte b[db->indexPageSize];
    readIndexByPageNum(db,b,indexnum);
    struct PageBase *pageBase = b;
    showPageBase(pageBase);//调试使用，正式运行去掉
    //key数组起始位
    int keystart = sizeof(struct PageBase);

    //开始查找  如果key是固定的情况下 直接从PageBase的start位置开始查找
    if(db->keySize > 0) {//1期暂时先不考虑不固定key情况
        //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
        if (pageBase->childNum <= 0) {// 此页没有个数 直接在此页插入一个新页
            // 目前索引不存在此情况
            return 0;
        } else {
            //此页有个数 则查找对应的start位置 此值比下标大1
            int start = selectIndexData(b, pageBase->childNum, key, db->keySize, keystart, db->keySize + IndexKeySize,db->keytype);
            if (start < 0) {//理论上新增子节点不可能出现此情况
                return 0;
//                start = -start;
            }else{//start为本key插入的位置 等于下标+1
                //首先看页数目是否满了 如果满了 需要拆分索引 然后重新插入
//                printf("数目 %d 最大数目 %d",pageBase->childNum ,db->maxIndexNodeNum);
                if(pageBase->childNum >= db->maxIndexNodeNum){
                    //@todo 如果子页的数目已经满了 则执行分页操作 返回新增的页号
                    unsigned int splitPageNum  = splitIndexPage(db,indexnum);//参数 db 和 page页  返回值返回下一页的页号
                    //如果start的位置 大于 maxNodeNum/2 的一半 则插入到下一页 否则插入本页 128 64  65  207  203 204
                    //第一页最后一个元素为maxNodeNum/2  第二页的第一个元素 为 maxNodeNum/2 + 1
                    //key 0 34104 中间位 17052  34104
                    if (start > db->maxNodeNum/2 +1){//start比maxNodeNum/2 + 1的元素大的时候 才插入第二页 这样不影响上级索引变化
                        //这里将childpage的父级页重新更新一下
                        if(pageBase->type == 2){//索引叶子节点
                            byte childdata[db->pageSize];
                            readDataByPageNum(db,childdata,childpage);
                            struct PageBase *cpagebase = childdata;
                            cpagebase->parentPageNum = splitPageNum;
                            writeDataByPageNum(db,childdata,childpage);
                        }else{
                            byte childdata[db->indexPageSize];
                            readIndexByPageNum(db,childdata,childpage);
                            struct PageBase *cpagebase = childdata;
                            cpagebase->parentPageNum = splitPageNum;
                            writeIndexByPageNum(db,childdata,childpage);
                        }
                        return insertPageAndKeyIndex(db,splitPageNum,key,childpage,keysize);//递归一次 插入到下页
                    }else{//否则追加到本页最后元素为maxNodeNum/2 的后面 序号为 maxNodeNum/2+1
                        return insertPageAndKeyIndex(db,indexnum,key,childpage,keysize);//递归一次 插入本页
                    }
                }else{//正常未满的情况下索引插入
                    setkeytoIndexPageByte(db,b,key,childpage,start,0);
                    writeIndexByPageNum(db,b,indexnum);
                    //如果start是第一个key  且如果有上级的索引的话 还需要更改下上级索引的page对应的key值
                    if(start == 1 && pageBase->parentPageNum > 0){//当父级页不为空 且 此key是第一个元素 则需要把对应的父级页的索引key也改下 改为第一个
                        //目前的结构 一般不会出现这种情况 先预留此逻辑
                    }
                }

            }
        }
    }else{//key为非固定值
        //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
        if (pageBase->childNum <= 0) {// 此页没有个数 直接在此页插入一个新页
            // 目前索引不存在此情况
            return 0;
        } else {
            unsigned int * maxdataoff = b+keystart + db->dynamicIndexNum  * 8;//总大小偏移量
            //此页有个数 则查找对应的start位置 此值比下标大1
            int start = selectDynamicIndexData(b,pageBase->childNum,key,keysize,keystart,8,db->keytype);
            //首先判断此页是否已经满了  如果满了，则此页需要分裂 此值需要插入到分裂后的页
            //数目超过索引数 或者空间满了 都需要分裂
            if(pageBase->childNum >= db->dynamicIndexNum || keysize + 4 >= db->pageSize - (*maxdataoff)){
                //如果子页的数目已经满了 则执行分页操作 返回新增的页号
                unsigned int splitPageNum  = splitDynamicIndexPage(db,indexnum);//参数 db 和 page页  返回值返回下一页的页号
//                unsigned int splitPageNum  = splitIndexPage(db,indexnum);//参数 db 和 page页  返回值返回下一页的页号
                //第一页最后一个元素为maxNodeNum/2  第二页的第一个元素 为 maxNodeNum/2 + 1
                if(start < 0){
                    start = -start +1;//以此位置的后一位来判断
                }
                // || (-start) > pageBase->childNum/2
                if (start > pageBase->childNum/2 +1 ){//start比childNum/2 + 1的元素大的时候 才插入第二页 这样不影响上级索引变化
                    //这里将childpage的父级页重新更新一下
                    if(pageBase->type == 2){//索引叶子节点
                        byte childdata[db->pageSize];
                        readDataByPageNum(db,childdata,childpage);
                        struct PageBase *cpagebase = childdata;
                        cpagebase->parentPageNum = splitPageNum;
                        writeDataByPageNum(db,childdata,childpage);
                    }else{
                        byte childdata[db->indexPageSize];
                        readIndexByPageNum(db,childdata,childpage);
                        struct PageBase *cpagebase = childdata;
                        cpagebase->parentPageNum = splitPageNum;
                        writeIndexByPageNum(db,childdata,childpage);
                    }
                    return insertPageAndKeyIndex(db,splitPageNum,key,childpage,keysize);//递归一次 插入到下页
                }else{//否则追加到本页最后元素为maxNodeNum/2 的后面 序号为 maxNodeNum/2+1
                    return insertPageAndKeyIndex(db,indexnum,key,childpage,keysize);//递归一次 插入本页
                }
            }else {//数目正常
                 if(start < 0){//理论上新增子节点不可能出现此情况
                     return 0;
                }else{//此值不存在 直接插入对应位置
                    //将key value值写入
//                     setDynamicData(db,b,key,keysize,value,valuesize,start,0);
                     setDynamicIndex(db,b,key,keysize,childpage,start,0);
                     writeIndexByPageNum(db,b,indexnum);
                     //如果start是第一个key  且如果有上级的索引的话 还需要更改下上级索引的page对应的key值
                     if(start == 1 && pageBase->parentPageNum > 0){//当父级页不为空 且 此key是第一个元素 则需要把对应的父级页的索引key也改下 改为第一个
                         //目前的结构 一般不会出现这种情况 先预留此逻辑
                     }
                }
            }
        }
    }
    return indexnum;
}


static void setDynamicIndex(struct FastTreeDb *db,  byte data[], byte *key, int keysize,
                            unsigned int childpage, int start, int isupdate) {
    int keystart = sizeof(struct PageBase);
    //获取指定位置的key的起始位和size
    int indexstart = start -1;//索引位为start-1

//    int width = db->keySize + IndexKeySize;
//    int startsize = keystart + (start - 1) * width;
    if(isupdate == 1){//覆盖值
        //判断移动的位数大小 覆盖的位数为两个值的差值
        unsigned int *curoff  = data+keystart + indexstart * 12; //前4位是offset 后4位是keysize
        unsigned int *curkeysize  = data+keystart + indexstart * 12 + 4; //前4位是offset 后4位是keysize
        int chazhi =  keysize  - (*curkeysize);
        //根据差值 和 起始位 开始移动位置  从偏移量到当前位置开始copy

        // 移动的位数 = 全长 - 移动copy位  覆盖操作，移动后从下一个key的偏移量开始更改，本偏移量不变
        unsigned int * maxdataoff = data+keystart + db->dynamicIndexNum  * 8;

        moveIndexPage(data,chazhi,indexstart +1 ,(int)(*curoff) + keysize + 4,(int)*maxdataoff);

        //设置当前数目
        *curkeysize = keysize;//设置当前key值
        //设置当前值
        memmove(&data[*curoff], &key[0], keysize);//设置当前key值
        byte p[4];
        setByteByint(childpage,p);
        memmove(&data[(*curoff)+keysize],p,4);//设置当前key后面跟的页号
        *maxdataoff = *(maxdataoff) + chazhi;//总大小更新
    }else{//插入新值
        //首先改动最大数目+1
        struct PageBase *pageBase = data;
        showPageBase(pageBase);
        //移动偏移量
        unsigned int * maxdataoff = data+keystart + db->dynamicIndexNum  * 8;
        unsigned int *curoff = data+keystart + db->dynamicIndexNum  * 8;//默认偏移量设为空间存储的最大值 不要+4
        int startsize = keystart + indexstart * 8;
        unsigned int curoffnum = *curoff;
        if(indexstart < pageBase->childNum){//start的位置不是当前位置之后  才需要后移
            curoff = data+startsize;//start位置原值的偏移量 设为当前值存储的偏移量位置
            curoffnum = *curoff;
            moveIndexPage(data,keysize +4,indexstart,(int)(*curoff) + keysize + 4,(int)(*maxdataoff));
            //这里移动start后面的索引位
            memmove(&data[startsize + 8], &data[startsize], (db->dynamicIndexNum-start) * 8);//0 1 2   1
        }
        pageBase->childNum = pageBase->childNum+1;
        //设置当前索引偏移量 8位
        memmove(&data[startsize],  &curoffnum, 4);//设置当前索引偏移量
        memmove(&data[startsize+4], &keysize,4);//设置当前key size值
        //设置当前值
        memmove(&data[*curoff], &key[0], keysize);//设置当前key值
        byte p[4];
        setByteByint(childpage,p);
        memmove(&data[*curoff+keysize], p,4);//设置当前value值
        *maxdataoff = *(maxdataoff) + keysize +4;//总大小更新
    }
}


static void setkeytoIndexPageByte(struct FastTreeDb *db,  byte data[], byte *key, unsigned int childpage,
                      int start, int isupdate) {
    int keystart = sizeof(struct PageBase);
    int width = db->keySize + IndexKeySize;
    int startsize = keystart + (start - 1) * width;
    if(isupdate == 1){//覆盖值
        //逻辑待定
        byte p[4];
        setByteByint(childpage,p);
        memmove(&data[startsize+db->keySize], p, 4);//页号只写4位
    }else{//插入新值
        //首先改动最大数目+1
        struct PageBase *pageBase = data;
        showPageBase(pageBase);
        pageBase->childNum = pageBase->childNum+1;
        //然后将start后面的元素集体指定后移
        if(start < pageBase->childNum){//start的位置不是最后一位  才需要后移
            memmove(&data[startsize + width], &data[startsize], (pageBase->childNum-start) * width);//0 1 2   1
        }
        memmove(&data[startsize], &key[0], db->keySize);//设置当前值
        byte p[4];
        setByteByint(childpage,p);
        memmove(&data[startsize+db->keySize], p, 4);//设置当前值 IndexKeySize 默认页号只写4位
        //IndexKeySize-4 多余的位待定
    }
}

static void
initRootIndexBykeyPage(struct FastTreeDb *db, unsigned int rootNum, byte *key, unsigned int indexnum,int keysize) {
    byte indexb[db->indexPageSize];
    int prefixsize  = sizeof(struct PageBase);
    //初始化一个root页描述信息
    struct PageBase pageBase = {
            curNo : rootNum,//当前页序号
            //下一页
            nextPageNum:0,//页初始化的时候会变动 默认使用4个字节存储 具体看编译器
            //上一页
            prePageNum:0,//页初始化的时候会变动 默认使用4个字节存储 具体看编译器
            //父页
            parentPageNum:0,//root节点的父级为0
            type:1,//type 类型 索引还是data 1代表索引 2 代表叶子节点索引 默认为data类型的页
            childNum:1,//当前数目默认为0
    };
    memmove(&indexb[0], &pageBase, prefixsize);//24个字节 设置前缀
    //然后设置第一个key信息 这个key
//    byte *p =  getByteByint(indexnum);
    byte p[4];
    setByteByint(indexnum,p);
    if(db->keySize > 0){
        memmove(&indexb[prefixsize],key, db->keySize);//keysize个字节 设置key个字符
        memmove(&indexb[prefixsize + db->keySize],p, 4);//4个字节 设置key对应的页号
    }else{//非固定长度key处理
        //先预留N位的索引空间  每个索引占8个字节 前4位是起始位，后4位是key的长度 用 unsigned int 类型
        //末尾追加的4位是当前page的未使用空间的起始偏移量 常用数字为128*8+4 = 1028位
        int  startoff = prefixsize + db->dynamicIndexNum * 8 +4;//初始偏移位置
        memmove(&indexb[prefixsize],&startoff, 4);//设置起始第一个key偏移量
        memmove(&indexb[prefixsize + 4],&keysize, 4);//设置第一个key的长度为keysize
        //设置偏移量位置的已占用值
        unsigned int space = startoff + keysize + 4;//已占用空间的偏移量  第一个key是keysieze个字节，加上页号4字节 共占用keysize + 4字节
        memmove(&indexb[startoff - 4],&space,4);//设置占用的空间

        memset(&indexb[startoff],key[0], keysize);//4个字节 设置前缀key 1位 值位0
        memmove(&indexb[startoff+ keysize],p, 4);//4个字节 设置data页号
    }
    writeIndexByPageNum(db,indexb,rootNum);
    db->rootPageNum = rootNum;
    ssize_t t =pwrite(db->dbinfofd,db,sizeof(struct FastTreeDb),0);//从0的位置开始写
//    free(p);//释放p
}

static unsigned int getNextIndexPage(struct FastTreeDb *pb) {
    pb->maxIndexPageNum = pb->maxIndexPageNum + 1;
    ssize_t t =pwrite(pb->dbinfofd,pb,sizeof(struct FastTreeDb),0);//从0的位置开始写
    return pb->maxIndexPageNum;
}

/**
 * 此方法返回最终应该落地的data page页号
 * 参数解释
 * data 表示一页数据的数组
 * zongliang 代表当前key的个数
 * key[] 代表需要查找的key
 * keySize 查找key的位数大小
 * offset 代表前缀位的字节 在查找的时候跳过此位置算起始位
 * width 每个节点的占用宽度
 */
unsigned int selectPageByDbKey(struct FastTreeDb *db,unsigned int pageNum,int type, byte key[],int keysize){
    if(type == 1 || type == 2){//索引  1索引 2 索引叶子节点 data   3 data节点
        byte b[db->indexPageSize];
        readIndexByPageNum(db,b,pageNum);
//        printf("读取偏移量 %d",(pageNum - 1) * db->indexPageSize);
        struct PageBase *pageBase = b;
        showPageBase(pageBase);
        //开始查找  如果key是固定的情况下 直接从PageBase的start位置开始查找
        if(db->keySize > 0){//1期暂时先不考虑不固定key情况
            //key数组起始位
            int keystart = sizeof(struct PageBase);
            //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
            if(pageBase->childNum <= 0){// 此页没有个数 直接在此页插入一个新页
                // 目前索引不存在此情况
                return 0;
            }else{
                //此页有个数 则查找对应的start位置 此值比下标大1
                int start = selectIndexData(b,pageBase->childNum,key,db->keySize,keystart,db->keySize+IndexKeySize,db->keytype);
                if(start < 0){//表示找到和此key一模一样的key 则继续往下找
                    start = -start;
                }else{
                    //这里没查到相等key需要注意 假设start为2  那么说明自己本身的index位置是1
                    // 但是并不是在1的位置页找，而是在前一位的页中寻找
                    if(start >= 2){
                        start = start -1;
                    }
                    //如果本身返回1 说明本身页是第一个,索引位置为0，因为第一个页前面没有页了 所以还是插入到本身页 等分裂的时候在重写索引key
//                    start = start -1;
                }
//                byte *p = &b[keystart + (start - 1) * (db->keySize + IndexKeySize) + db->keySize];//mid为中间位
                byte *p = b+keystart + (start - 1) * (db->keySize + IndexKeySize) + db->keySize;//mid为中间位
                unsigned int curpage = getIntBychar4(p);
                //继续往下找
                if(pageBase->type == 2){//本身是叶子节点 直接返回查到的page页
                    return curpage;//直接返回此页
                }else{//非叶子节点 查找索引数据
                    //递归查找页
                    return selectPageByDbKey(db,curpage,pageBase->type,key,keysize);
                }
            }
        }else{//非固定key value的索引情况
            //key数组起始位
            int keystart = sizeof(struct PageBase);
            //根据key的数目开始对比 data数据页可以没有个数 但是索引页只要创建必须含有1个或以上data
            if(pageBase->childNum <= 0){// 此页没有个数 直接在此页插入一个新页
                // 目前索引不存在此情况
                return 0;
            }else{
                //此页有个数 则查找对应的start位置 此值比下标大1
                int start = selectDynamicIndexData(b,pageBase->childNum,key,keysize,keystart,8,db->keytype);
                if(start < 0){//表示找到和此key一模一样的key 则继续往下找
                    start = -start;
                }else{
                    //这里没查到相等key需要注意 假设start为2  那么说明自己本身的index位置是1
                    // 但是并不是在1的位置页找，而是在前一位的页中寻找
                    if(start >= 2){
                        start = start -1;
                    }
                    //如果本身返回1 说明本身页是第一个,索引位置为0，因为第一个页前面没有页了 所以还是插入到本身页 等分裂的时候在重写索引key
//                    start = start -1;
                }
//                byte *p = &b[keystart + (start - 1) * (db->keySize + IndexKeySize) + db->keySize];//mid为中间位
                unsigned int * curoff  = b+keystart + (start - 1) * 8; //前4位是offset 后4位是size
                unsigned int * curkeysize  = b+keystart + (start - 1) * 8 + 4; //前4位是offset 后4位是keysize
//                byte *p = b+keystart + (start - 1) * (db->keySize + IndexKeySize) + db->keySize;//mid为中间位
                byte *p = b+(*curoff) + (*curkeysize);//去除偏移量和size 后面紧跟着的就是页号
                unsigned int curpage = getIntBychar4(p);//页号
                //继续往下找
                if(pageBase->type == 2){//本身是叶子节点 直接返回查到的page页
                    return curpage;//直接返回此页
                }else{//非叶子节点 查找索引数据
                    //递归查找页
                    return selectPageByDbKey(db,curpage,pageBase->type,key,keysize);
                }
            }

        }
    }else{//data
        return pageNum;
    }
    return  0;
}

int readIndexByPageNum(struct FastTreeDb *db, void *data, unsigned int num) {
    ssize_t len = pread(db->indexfd, data, db->indexPageSize, (long)(num - 1) * db->indexPageSize);
    return 1;
}

int readDataByPageNum(struct FastTreeDb *db, void *data, unsigned int num) {
    ssize_t len = pread(db->datafd, data, db->pageSize, (long)(num - 1) * db->pageSize);
    return 1;
}
int writeDataByPageNum(struct FastTreeDb *db, void *data, unsigned int num) {
//    printf("写入页：%d",num);
    ssize_t len = pwrite(db->datafd,data,db->pageSize,(long)(num - 1) * db->pageSize);//写入
    return 1;
}
static int writeIndexByPageNum(struct FastTreeDb *db,  void *data, unsigned int indexnum) {
    ssize_t len = pwrite(db->indexfd,data,db->indexPageSize,(long)(indexnum - 1) * db->indexPageSize);//写入
    return 1;
}

/**
 * 此方法返回某个key返回的位置 下标+1
 * 负数表示找到某个下标的完全相同的key
 * 正数表示没有找到，返回当前的key排序插入的位置
 *
 * 参数解释
 * data 表示一页数据的数组
 * zongliang 代表当前key的个数
 * key[] 代表需要查找的key
 * keySize 查找key的位数大小
 * offset 代表前缀位的字节 在查找的时候跳过此位置算起始位
 * width 每个节点的占用宽度
 */
int selectIndexData(byte data[],int zongliang,byte key[],int keySize,int offset,int width,int keytype) {//keySize 代表key的位数
//    printf("结构体size：%d\n", sizeof(data));
    int start = 0;
    int end = zongliang - 1;//搜索是从 倒数第二位开始  当总量为1时 表示之前没有元素  则不搜索
    //值比元素大的 往右移动
    //比元素小的 往左移动
    //跟中间的元素比较
    while (end >= start) {//结束位置大于start位置
        int mid = (end + start) / 2;// mid  中间元素
        unsigned char *p = (data + offset + mid * width);//mid为中间位
//        printf("位置1 start %d  目标位置 %d \n",start,280+data[start]*(datasize));
        int num = compare(key, p, keySize,keytype);
//         printf("比较 %d\n",num);
        if (num > 0) {//比选中的大 右移动
            start = mid + 1;
        } else if (num < 0) {//key比较小 往左移动查找
            end = mid - 1;
        } else if (num == 0) {//等于的情况 跳出
//            isSameNode = mid;
            return -(mid+1);
        }
    }
    return start+1;
}


/**
 * 此方法和selectIndexData相似，主要针对不定长key
 * 返回某个key返回的位置 下标+1
 * 负数表示找到某个下标的完全相同的key
 * 正数表示没有找到，返回当前的key排序插入的位置
 *
 * 参数解释
 * data 表示一页数据的数组
 * zongliang 代表当前页key的个数
 * key[] 代表需要查找的key
 * keySize 查找key的位数大小
 * offset 代表前缀位的字节 在查找的时候跳过此位置算起始位
 */
int selectDynamicIndexData(byte data[],int zongliang,byte key[],int keySize,int offset,int width,int keytype) {//keySize 代表key的位数
//    printf("结构体size：%d\n", sizeof(data));
    int start = 0;
    int end = zongliang - 1;//搜索是从 倒数第二位开始  当总量为1时 表示之前没有元素  则不搜索
    //值比元素大的 往右移动
    //比元素小的 往左移动
    //跟中间的元素比较
//    printf("end %d start %d\n",end,start);
    while (end >= start) {//结束位置大于start位置
        int mid = (end + start) / 2;// mid  中间元素
        //获取指定位置的key的起始位和size
        unsigned int * curoff  = data+offset + mid * width; //前4位是offset 后4位是size
        unsigned int * curkeysize  = data+offset + mid * width + 4; //前4位是offset 后4位是keysize
        unsigned char *p = (data + (*curoff) );//mid的起始位 去除起始偏移量
//        printf("位置1 start %d  目标位置 %d \n",start,280+data[start]*(datasize));
        int num = compareDynamic(key, p, keySize,(int)(*curkeysize),keytype);
//         printf("比较 %d\n",num);
        if (num > 0) {//比选中的大 右移动
            start = mid + 1;
        } else if (num < 0) {//key比较小 往左移动查找
            end = mid - 1;
        } else if (num == 0) {//等于的情况 跳出
//            isSameNode = mid;
            return -(mid+1);
        }
    }
    return start+1;
}

int compareDynamic(unsigned char key[],  unsigned char data[], int keysize, int datakeysize, int keytype) {
    //获取最小的size
    int minsize = keysize;
    if(keysize > datakeysize){
        minsize = datakeysize;
    }
    for(int i=0 ;i < minsize;i++){
        //因为涉及到正负值的问题 所以默认以有符号的char来比较
        if((byte)key[i]>(byte)data[i]){
            return 1;
        }else if ((byte)key[i]<(byte)data[i]){
            return -1;
        }
    }
    //如果都相等 则看谁的长度更长
    return keysize - datakeysize;
}

//0 默认byte型 1 字符串型 2 整型 3 浮点数型
static int compare(unsigned char key[], unsigned char data[], int keysize,int keytype) {
//    printf("key:");
//    for(int i = 0 ;i<  keysize ;i++ ){
//        printf("%d",key[i]);
//    }
//    printf("\ndata:");
//    for(int i = 0 ;i<  keysize ;i++ ){
//        printf("%d",data[i]);
//    }
//    printf("\n");
    if(keytype == 0 || keytype == 1){// 因为\0的存在 字符串和byte型 目前默认使用同样类似的比较
        for(int i=0 ;i < keysize;i++){
            //因为涉及到正负值的问题 所以默认以有符号的char来比较
            if((byte)key[i]>(byte)data[i]){
                return 1;
            }else if ((byte)key[i]<(byte)data[i]){
                return -1;
            }
        }
    }else if(keytype == 2 && keysize == 8){//整型 8位
        if(*(long *)&key[0]>*(long *)&data[0]){
            return 1;
        }else if (*(long *)&key[0]<*(long *)&data[0]){
            return -1;
        }
    }else if(keytype == 2 && keysize == 4){//整型 4位
        if(*(int *)&key[0]>*(int *)&data[0]){
            return 1;
        }else if (*(int *)&key[0]<*(int *)&data[0]){
            return -1;
        }
    }else if(keytype == 3 && keysize == 8){//浮点型 8位
        if(*(double *)&key[0]>*(double *)&data[0]){
            return 1;
        }else if (*(double *)&key[0]<*(double *)&data[0]){
            return -1;
        }
    }else if(keytype == 3 && keysize == 4){//浮点型 4位
        if(*(float *)&key[0]>*(float *)&data[0]){
            return 1;
        }else if (*(float *)&key[0]<*(float *)&data[0]){
            return -1;
        }
    }

    return 0;
}










