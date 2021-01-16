#ifndef DATABASECONFIGURE_H
#define DATABASECONFIGURE_H

/************************************************************************************
 * 说明：该文件用于读取json格式配置文件，主要考虑到不同环境中数据库对应的配置可能不同，所以
 *     写了该功能用来读取数据库中的一些配置字段
 * ***********************************************************************************/


#include <QString>
#include <QFile>
#include <QJsonParseError>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QDir>

class DataBaseConfigure{
public:
    static QString driver;
    static QString hostName;
    static int port;
    static QString dataBaseName;
    //static QString eventTable;
    static QString userName;
    static QString password;
    static QString table;
    static bool getDataBaseConfigure();
};


#endif // DATABASECONFIGURE_H
