#include "miningarealocation.h"

//# pragma execution_character_set("utf-8")

QMap<QString,QString> MiningAreaLocation::getMiningAreaLocationConfigure(QString whichArea)
{
    QString path = QDir::currentPath();
    QString tempPath;
    tempPath=path + "/config/miningarealocation.json";
    qDebug()<<"json file path"<<tempPath;
    QFile file(tempPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    //QByteArray allData = file.readAll();
    QString allData = file.readAll();
    file.close();

    QMap<QString,QString> map;
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(allData.toUtf8(), &parseJsonErr);
    if (!(parseJsonErr.error == QJsonParseError::NoError)) {
        qDebug()<<"json pares error";
        return map;
    }

    QJsonObject rootObj = document.object();
    QStringList keys = rootObj.keys();
    for(int i = 0; i < keys.size(); i++)
    {
        qDebug() << "key" << i << " is:" << keys.at(i);
    }
    if(rootObj.contains(whichArea))
    {
       QJsonObject subObj = rootObj.value(whichArea).toObject();

       QStringList qs = subObj.keys();
       for(QString key:qs){
           //QString v = QLatin1String(subObj.value(key).toString().toUtf8());
          // QString v = QString::fromLocal8Bit(subObj.value(key).toString().toUtf8());
           QString v = subObj.value(key).toString();
           //TODO,使用正则表达式过滤掉不安全的Key,Value,待优化
           map.insert(key,v);
       }
    }


    QMap<QString, QString>::iterator i;
     for (i = map.begin(); i != map.end(); ++i)
         qDebug()<< i.key() << ": " << i.value();
    return map;
}
