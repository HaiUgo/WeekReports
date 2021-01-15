#include "miningareamap.h"


QMap<QString,QString> MiningAreaMap::getMiningAreaMapConfigure()
{
    QString path = QDir::currentPath();
    QString tempPath;
    tempPath=path + "/config/miningareamap.json";
    qDebug()<<"json file path"<<tempPath;
    QFile file(tempPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QMap<QString,QString> map;
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
    if (!(parseJsonErr.error == QJsonParseError::NoError)) {
        qDebug()<<"json pares error";
        return map;
    }

    QJsonObject jsonObject = document.object();
    QStringList qs = jsonObject.keys();
    for(QString key:qs){
        QString v = jsonObject.value(key).toString();
        //TODO,使用正则表达式过滤掉不安全的Key,Value,待优化
        map.insert(key,v);
    }


    QMap<QString, QString>::iterator i;
     for (i = map.begin(); i != map.end(); ++i)
         qDebug()<< i.key() << ": " << i.value();
//    if (jsonObject.contains(QStringLiteral("DatabaseName"))){
//        dataBaseName = jsonObject["DatabaseName"].toString();
//    }

    return map;
}
