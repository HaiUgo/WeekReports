#ifndef MININGAREAMAP_H
#define MININGAREAMAP_H

/************************************************************************************
 * 说明：该文件用于读取json格式配置文件，主要考虑到应用的扩展以及矿区图变化时能及时维护，所以
 *     写了该功能用来读取一些配置字段
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
#include <QMap>

class MiningAreaMap{
public:

    static QMap<QString,QString> getMiningAreaMapConfigure();
};

#endif // MININGAREAMAP_H
