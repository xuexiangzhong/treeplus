#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <slapi-plugin.h>
//#include "BTreeFilePlus.h"
#include "../src/BTreeFile.h"
//#include <errno.h>
typedef struct storage_page_int
{
    int data[1024];
} pagedata;

void testdbinsertDy(struct FastTreeDb* db ,int num,int isrand);

void testPrintTreeAll(struct FastTreeDb* db );

void testdbinsert(struct FastTreeDb* db ,int num,int isrand);

void testSeekTree(struct FastTreeDb* db);

void TreeTest();

void TreeTestDy();

int main() {
    clock_t start, end;
    start = clock();
    //key value固定长度的测试
//    TreeTest();

    //key value不固定长度的测试
    TreeTestDy();//
    printf("\n");

    end = clock();
    printf("time=%f\n", (double)(end - start) / CLOCKS_PER_SEC);//
    printf("\n");
    return 0;
}

void TreeTestDy() {
    // 0 默认byte型 1 字符串型 2 整型 3 浮点数型
    struct FastTreeDb* db = treedb_init("./testdy",4096,4096,0,0,128,128,0);
//    testdbinsertDy(db,10*10000,0);//isrand 1为随机生成key value  为其他值表示生成顺序指定key value测试
//    testPrintTreeAll(db);//打印所有key vale值  检查上面的执行结果
    //查询使用
    testSeekTree(db);
}

void TreeTest() {
    // 0 默认byte型 1 字符串型 2 整型 3 浮点数型
    struct FastTreeDb* db = treedb_init("./test",4096,4096,4,200,0,0,0);
    testdbinsert(db,10*10000,0);//isrand 1为随机生成key value  为其他值表示生成顺序指定key value测试
    testPrintTreeAll(db);//打印所有key vale值  检查上面的执行结果
    //查询使用
    testSeekTree(db);
}

void testSeekTree(struct FastTreeDb* db) {
    //这个其实不准确 以初始化的时候为准
//    struct FastTreeDb* db = treedb_init("./test",4096,4096,0,0,128,128,0);
    opendb(db);//打开
    byte key[200] = "dsjsoijsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoi"
                    "fjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuerg";//大于测试db的size定义长度即可 这里因为固定和非固定的都调的一个方法 所以设置为大一点的数字200的长度
    setByteByint((unsigned int)8000,key);//定位到8000的位置

    //定位到指定位置后可使用 getCurNode获取当前节点值 getNextBySeekNode获取下一个节点值 getPreBySeekNode 获取上一个节点值

    //seekType 特殊SeekNode获取定义  传1代表 取第一个date页第一个值  传2 代表取最后一页最后一个值  其余的默认按key找到对应所在页的对应的值  传22代表倒叙取值
    struct SeekNode *node = calloc(1, sizeof(struct SeekNode));
    byte b[db->pageSize];
    node->pagedata = b;
    if(db->keySize > 0){
        getSeekNodeByDbKey(db,key,db->keySize,node,0);
    }else{//非固定key 传对应的长度
        getSeekNodeByDbKey(db,key,9,node,0);
    }
    struct EntryNode *entryNode = calloc(1, sizeof(struct EntryNode));
    printf("页号：%d 序号： %d",node->curPageNo,node->seekindex);
    getCurNode(db,node,entryNode);//获取key8000对应的值
    if(entryNode->keyNode.keySize != -1){
        printf("key 为默认byte类型: ");
        for(int k = 0;k<entryNode->keyNode.keySize;k++ ){
            printf("%c",(char)(entryNode->keyNode.key[k]));
        }
        unsigned int numkey = getIntBychar4(entryNode->keyNode.key);
        printf(" \n数字打印%d 长度%d ",numkey,entryNode->keyNode.keySize);
        printf(" key end \n");

        printf("value : ");
        unsigned int valuekey = getIntBychar4(entryNode->keyNode.key);
        printf("%d",valuekey);
        for(int v = 4;v<entryNode->valueNode.valueSize;v++ ){//value为页号 为固定值
            printf("%c",(char)entryNode->valueNode.value[v]);
        }
        printf(" value end \n");
    }else{
        printf("获取失败");
    }
    //传1代表 取第一个date页第一个值  传2 代表取最后一页最后一个值  其余的默认按key找到对应所在页的对应的值  传22代表倒叙取值
    for(int i = 0 ;i<100;i++){//测试值查找100个即可
        getNextBySeekNode(db,node,1,entryNode);
//        getPreBySeekNode(db,node,1,entryNode);//倒叙遍历
        if(entryNode->keyNode.keySize != -1){
            printf("key 为默认byte类型: ");
            for(int k = 0;k<entryNode->keyNode.keySize;k++ ){
                printf("%c",(char)(entryNode->keyNode.key[k]));
            }
            unsigned int numkey = getIntBychar4(entryNode->keyNode.key);
            printf(" \n数字打印%d 长度%d ",numkey,entryNode->keyNode.keySize);
            printf(" key end \n");

            printf("value : ");
            unsigned int valuekey = getIntBychar4(entryNode->keyNode.key);
            printf("%d",valuekey);
            for(int v = 4;v<entryNode->valueNode.valueSize;v++ ){//value为页号 为固定值
                printf("%c",(char)entryNode->valueNode.value[v]);
            }
            printf(" value end \n");
        }else{
            printf("获取失败");
            break;
        }

    }
    closedb(db);
}


void testPrintTreeAll(struct FastTreeDb* db ) {
//    struct FastTreeDb* db = treedb_init("./test",4096,4096,0,0,128,128,0);
    opendb(db);
    printTreedb(db,1);
    closedb(db);
}

void testdbinsertDy(struct FastTreeDb* db ,int num,int isrand) {
    opendb(db);
    srand((unsigned)time(NULL));

    char ary[512] = "dsjsoijsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjf";//abcdefghijk55er
    byte key[200]  = "dsjsoijsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjf";//本次测试key为int数值类型，字节长度大于4即可
    if(isrand == 1){//随机key测试
        for(int i = num;i>0;i--){

            setByteByint((unsigned int)(rand()%100000),key);
            for(int i=0;i<4;i++){
                ary[i] = key[i];
            }
            //size的长度可以随意设置 这里为了方便就直接做简单取余加减
            int keysize = 10+(i%100);
            int valuesize = 100+(i%100);
//            insertTreedbByDataAndSize(db,key,200,ary,250);
            insertTreedbByDataAndSize(db,key,keysize,ary,valuesize);
        }
    }else{
        for(int i = num;i>0;i--){
            setByteByint((unsigned int)i,key);
            for(int i=0;i<4;i++){
                ary[i] = key[i];
            }
            //size的长度可以随意设置 这里为了方便就直接做简单取余加减
            int keysize = 10+(i%100);
            int valuesize = 100+(i%100);
//            insertTreedbByDataAndSize(db,key,200,ary,250);
            insertTreedbByDataAndSize(db,key,keysize,ary,valuesize);
        }
    }

    closedb(db);
}
void testdbinsert(struct FastTreeDb* db ,int num,int isrand) {
    opendb(db);//打开
    srand((unsigned)time(NULL));
    char ary[512] = "dsjsoijsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjfisoifjioerjfgiorejgiorjgiodjiogsjgsklgiuergjajfdsiofjsoijfoisejfjefiosajdokjf";//abcdefghijk55er
    if(isrand == 1){//随机key测试
        for(int i = num;i>0;i--){
            byte key[200];
            setByteByint((unsigned int)(rand()%100000),key);
            for(int i=0;i<4;i++){
                ary[i] = key[i];
            }
//            insertTreedbByDataAndSize(db,key,200,ary,250);
            insertTreedb(db,key,ary);//长度在初始化的时候设定死
        }
    }else{
        for(int i = num;i>0;i--){
            byte key[100];//本次测试key为int数值类型，字节长度大于4即可
            setByteByint((unsigned int)i,key);
            for(int i=0;i<4;i++){
                ary[i] = key[i];
            }
//            insertTreedbByDataAndSize(db,key,200,ary,250);
            insertTreedb(db,key,ary);//长度在初始化的时候设定死
        }
    }
    closedb(db);//关闭
}