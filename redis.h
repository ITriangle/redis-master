#ifndef _REDIS_H
#define _REDIS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <string.h>
#include <stdio.h>

#include "hiredis.h"

using namespace std;

/**
 * 打印信息到屏幕
 */
#define  REDIS_PRINTF(format,args...)    printf("%s-%s-%d:" format "\n",__FILE__,__FUNCTION__,__LINE__,##args)

#define  GET_STRING_CMD     "GET"//获取string类型key值
#define  SET_STRING_CMD     "SET"//设置string类型key值
#define  SET_SET_CMD        "SADD"//添加到set集合中
#define  GET_SET_CMD        "SMEMBERS"//获取set集合所有的成员
#define  DEL_KEY_CMD        "DEL"//删除key值

#define  MSET_STRING_CMD    "MSET"//设置多个key值
#define  MGET_STRING_CMD    "MGET"//获取多个key值

class Redis
{
public:
    //构造函数

    Redis(std::string sIP, int iPort, std::string sAuthPassword);

    ~Redis();

    bool RedisConnect();//连接数据库

    redisContext* GetContext();//获得连接结构体

    redisReply* GetReply();//获取返回结构体

    void FreeReply();//释放返回值结构体

    void FreeConnect();//释放连接

    void PrintError(string cmd);//打印错误信息

    //设置hashs值，vDatad对应field - value
    bool hmset(const string& cmd, const string& key, const vector<string>& vData);

    bool Set_KeyString(const string& cmd, const string& key, const string& sData);//设置单个key值

    bool Get_KeyValue(const string& cmd, const string& key);//获得key值

    bool MSet_KeyString(const string& cmd, const string& key1, const string& sData1, const string& key2, const string& sData2);//设置2个key的值
    bool MGet_KeyValue(const string& cmd, const string& key1, const string& key2);//获得key值

    bool MSet_KeyString(const string& cmd, const string& key1, const string& sData1, const string& key2, const string& sData2 , const string& key3, const string& sData3);//设置3个key的值

    bool MGet_KeyValue(const string& cmd, const string& key1, const string& key2, const string& key3);//获得key值

    bool MGet_KeyValue(vector<string> vCmdData);//获得多个key值
    bool MSet_KeyString(vector<string> vCmdData);//设置多个key的值

    bool MSet_KeyString(const string cmd, map<string, string> mCmdData);//设置多个key的值


    void GetValue_ArrayType(vector<string>& vValue);//从返回结果中获取字符串数组值

    void GetValue_String(string& sValue);//从返回结果中获取单个字符串值

    long long GetValue_Integer();//从返回结果中获取整型数据的值

    //数据库命令
    bool SelectDB(int iNumDB);//选择数据库
    bool FlushDB(int iNumDB);//清除选中的数据库
    bool Dbsize(int iNumDB);//读取数据库大小

    //删除key
    bool Del_Key(const string& key);

private:

    bool CheckReply(const redisReply *reply);//检查返回值

    bool CommandArgv(vector<string> vData);//执行命令

    void addparam(vector<string>& vDes, const vector<string>& vSrc) ;//添加参数值
private:
    int mRedis_Port;//Redis Port
    std::string mRedis_IP;//Redis IP
    std::string mRedis_AuthPassWord;//Redis Auth Password
    redisContext* mConnect;//连接数据结构体
    redisReply* mReply;//返回结果结构体
};

Redis::Redis(std::string sIP, int iPort, std::string sAuthPassword)
{
    mRedis_IP = sIP;
    mRedis_Port = iPort;
    mRedis_AuthPassWord = sAuthPassword;
}

Redis::~Redis()
{

}

redisContext* Redis::GetContext()
{
    return mConnect;
}


redisReply* Redis::GetReply()
{
    return mReply;
}

bool Redis::CheckReply(const redisReply *reply)
{
    if (NULL == reply)
    {
        return false;
    }

    switch (reply->type)
    {
    case REDIS_REPLY_STRING:
    {
        return true;
    }
    case REDIS_REPLY_ARRAY:
    {
        return true;
    }
    case REDIS_REPLY_INTEGER:
    {
        return true;
    }
    case REDIS_REPLY_NIL:
    {
        return false;
    }
    case REDIS_REPLY_STATUS:
    {
        return (strcasecmp(reply->str, "OK") == 0) ? true : false;
    }
    case REDIS_REPLY_ERROR:
    {
        return false;
    }
    default:
    {
        return false;
    }
    }

    return false;
}

bool Redis::CommandArgv(vector<string> vData)
{
    bool retbl = false;

    vector<const char *> argv( vData.size() );
    vector<size_t> argvlen( vData.size() );

    unsigned int j = 0;
    for ( vector<string>::const_iterator i = vData.begin(); i != vData.end(); ++i, ++j )
    {
        argv[j] = i->c_str(), argvlen[j] = i->size();
    }

    mReply = static_cast<redisReply *>(redisCommandArgv(mConnect, argv.size(), &(argv[0]), &(argvlen[0])));

    if (CheckReply(mReply))
    {
        retbl = true;
    }
    else
    {
        freeReplyObject(mReply);
    }

    return retbl;
}

void Redis::addparam(vector<string>& vDes, const vector<string>& vSrc)
{
    for (vector<string>::const_iterator iter = vSrc.begin(); iter != vSrc.end(); ++iter)
    {
        vDes.push_back(*iter);
    }
}

void Redis::FreeReply()
{
    freeReplyObject(mReply);
    mReply = NULL;
}

void Redis::FreeConnect()
{
    redisFree(mConnect);
    mConnect = NULL;
}

bool Redis::RedisConnect()
{
    mConnect = redisConnect(mRedis_IP.c_str(), mRedis_Port);
    if (mConnect == NULL || mConnect->err)
    {
        REDIS_PRINTF("Connect error:%s\n", mConnect->errstr);
        if (mConnect != NULL)
        {
            redisFree(mConnect);
            mConnect = NULL;
        }
        return false;
    }

    /**
     * 输入Auth 的密码
     */
    mReply = static_cast<redisReply *>(redisCommand(mConnect, "auth %s", mRedis_AuthPassWord.c_str()));
    if (!CheckReply(mReply))
    {
        REDIS_PRINTF("auth error:%s\n", mConnect->errstr);
        freeReplyObject(mReply);
        redisFree(mConnect);
        mConnect = NULL;
        
        return false;
    }

    freeReplyObject(mReply);

    return true;
}

void Redis::GetValue_ArrayType(vector<string>& vValue)
{

    for (size_t i = 0; i < mReply->elements; i++)
    {
        vValue.push_back(string(mReply->element[i]->str, mReply->element[i]->len));
    }

    freeReplyObject(mReply);

}

void Redis::PrintError(string cmd)
{
    REDIS_PRINTF("%s error : %s\n", cmd.c_str(), mReply->str);
}

void Redis::GetValue_String(string& sValue)
{
    sValue.clear();
    sValue.append(mReply->str);

    freeReplyObject(mReply);
}


long long  Redis::GetValue_Integer()
{
    long long integerValue = mReply->integer;

    freeReplyObject(mReply);

    return integerValue;
}


bool Redis::hmset(const string& cmd, const string& key, const vector<string>& vData)
{
    vector<string> vCmdData;
    vCmdData.push_back(cmd);
    vCmdData.push_back(key);

    addparam(vCmdData, vData);

    return CommandArgv(vCmdData);
}


bool Redis::Set_KeyString(const string& cmd, const string& key, const string& sData)
{
    vector<string> vCmdData;
    vCmdData.push_back(cmd);
    vCmdData.push_back(key);
    vCmdData.push_back(sData);

    return CommandArgv(vCmdData);
}



bool Redis::Get_KeyValue(const string& cmd, const string& key)
{
    vector<string> vCmdData;
    vCmdData.push_back(cmd);
    vCmdData.push_back(key);

    return CommandArgv(vCmdData);
}

bool Redis::MSet_KeyString(const string& cmd, const string& key1, const string& sData1, const string& key2, const string& sData2)//设置2个key的值
{
    vector<string> vCmdData;
    vCmdData.push_back(cmd);
    vCmdData.push_back(key1);
    vCmdData.push_back(sData1);
    vCmdData.push_back(key2);
    vCmdData.push_back(sData2);

    return CommandArgv(vCmdData);
}

/**
 * [Redis::MGet_KeyValue description]mget获取的返回值,如果不存在,对应的字符数组会分配空间,但是长度为0,直接判空就可以了
 * @param  cmd  [description]
 * @param  key1 [description]
 * @param  key2 [description]
 * @return      [description]
 */
bool Redis::MGet_KeyValue(const string& cmd, const string& key1, const string& key2)//获得key值
{
    vector<string> vCmdData;
    vCmdData.push_back(cmd);
    vCmdData.push_back(key1);
    vCmdData.push_back(key2);

    return CommandArgv(vCmdData);
}

bool Redis::MSet_KeyString(const string& cmd, const string& key1, const string& sData1, const string& key2, const string& sData2 , const string& key3, const string& sData3)//设置3个key的值
{
    vector<string> vCmdData;
    vCmdData.push_back(cmd);
    vCmdData.push_back(key1);
    vCmdData.push_back(sData1);
    vCmdData.push_back(key2);
    vCmdData.push_back(sData2);
    vCmdData.push_back(key3);
    vCmdData.push_back(sData3);

    return CommandArgv(vCmdData);
}

bool Redis::MGet_KeyValue(const string& cmd, const string& key1, const string& key2, const string& key3)//获得key值
{
    vector<string> vCmdData;
    vCmdData.push_back(cmd);
    vCmdData.push_back(key1);
    vCmdData.push_back(key2);
    vCmdData.push_back(key3);

    return CommandArgv(vCmdData);
}

/**
 * [Redis::MSet_KeyString description]第0个为命令,第1个为key1,第2个为sData1
 * @param  vCmdData [description]
 * @return          [description]
 */
bool Redis::MSet_KeyString(vector<string> vCmdData)//设置多个key的值
{
    return CommandArgv(vCmdData);
}


bool Redis::MSet_KeyString(const string cmd, map<string, string> mCmdData)//设置多个key的值
{


    vector<string> vCmdData;
    vCmdData.push_back(cmd);

    string sKey;
    string sVal;
    map<string, string>::iterator  mIter;
    mIter = mCmdData.begin();
    for (; mIter != mCmdData.end(); ++ mIter)
    {
        sKey = mIter->first;
        sVal = mIter->second;

        vCmdData.push_back(sKey);
        vCmdData.push_back(sVal);
    }

    bool bRet = CommandArgv(vCmdData);

    freeReplyObject(mReply);
    return bRet;
}

/**
 * [Redis::MGet_KeyValue description]第0个为命令,第1个为key1,第2个为key2...
 * @param  vCmdData [description]
 * @return          [description]
 */
bool Redis::MGet_KeyValue(vector<string> vCmdData)//获得多个key值
{
    return CommandArgv(vCmdData);
}



bool Redis::SelectDB(int iNumDB)
{
    /*  vector<string> vCmdData;
      vCmdData.push_back("select 1");

      return CommandArgv(vCmdData);*/
    bool retbl = false;

    mReply = static_cast<redisReply *>(redisCommand(mConnect, "select %d", iNumDB));

    if (CheckReply(mReply))
    {
        retbl = true;
    }

    freeReplyObject(mReply);

    return retbl;

}


bool Redis::FlushDB(int iNumDB)
{
    bool retbl = false;

    SelectDB(iNumDB);

    mReply = static_cast<redisReply *>(redisCommand(mConnect, "flushdb"));

    if (CheckReply(mReply))
    {
        retbl = true;
    }

    freeReplyObject(mReply);


    return retbl;
}

bool Redis::Dbsize(int iNumDB)
{
    bool retbl = false;

    SelectDB(iNumDB);

    mReply = static_cast<redisReply *>(redisCommand(mConnect, "dbsize"));

    if (CheckReply(mReply))
    {
        //成功还需要获取当前db的大小
        retbl = true;
    }
    else
    {
        freeReplyObject(mReply);
    }

    return retbl;
}

bool Redis::Del_Key(const string& key)
{
    bool retbl = false;

    mReply = static_cast<redisReply *>(redisCommand(mConnect, "del %s", key.c_str()));

    if (CheckReply(mReply))
    {
        retbl = true;
    }

    freeReplyObject(mReply);

    return retbl;
}

#endif

