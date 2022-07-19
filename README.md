# treeplus
key value

# 一个简单的key value文件存储

##### 有固定长度和非固定长度模式。  
固定长度本来是考虑数据mysql这种很多定长字段定义使用的。实际上可能不定长更实用。

test下面是一些测试，上面演示了重要功能的使用方式   

##### 1 数据库初始化 
    方法 ： treedb_init 
##### 2 打开数据库 
    方法 opendb 
##### 3 插入数据 
    方法 ： insertTreedb （定长）or insertTreedbByDataAndSize （不定长）
##### 4 查看数据 
    getSeekNodeByDbKey 定位 没有匹配到的元素会定位到缝隙间左边元素 即前一个元素 
    getCurNode 获取定位当前的key 
    getNextBySeekNode 查找返回下一个元素 
    getPreBySeekNode 查找返回上一个元素 
##### 5 关闭库 
方法 ： closedb

#### 综合使用示例参考 TreeTest （定长）
##### 1 testdbinsert 定长元素插入 
##### 2 testPrintTreeAll(db); 所有元素打印
##### 3 testSeekTree(db); 
#### 元素获取遍历 TreeTestDy（不定长）
##### 1 testdbinsertDy(db,1*10000,0); 不定长元素插入
##### 2 testPrintTreeAll(db); 所有元素打印 
##### 3 testSeekTree(db); 元素获取遍历

插入效率10万次大概一秒不到吧，读的话也差不多，没有使用缓存机制，使用缓存应该可以加大效率  

目前是在mac os上测试，等空了用linux机器在跑下。