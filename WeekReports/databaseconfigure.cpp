#include "databaseconfigure.h"

QString DataBaseConfigure::driver = "";

QString DataBaseConfigure::hostName = "";

int DataBaseConfigure::port = 0;

QString DataBaseConfigure::dataBaseName = "";

//给一个默认表名
//QString DataBaseConfigure::eventTable = "mine_quack_results";

QString DataBaseConfigure::userName = "";

QString DataBaseConfigure::password = "";

bool DataBaseConfigure::getDataBaseConfigure()
{
    QString path = QDir::currentPath();
    QString tempPath;
    tempPath=path + "/config/databaseconfig.json";
    qDebug()<<"json file path"<<tempPath;
    QFile file(tempPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
    if (!(parseJsonErr.error == QJsonParseError::NoError)) {
        qDebug()<<"json pares error";
        return false;
    }

    QJsonObject jsonObject = document.object();

    if (jsonObject.contains(QStringLiteral("Driver"))){
        driver = jsonObject["Driver"].toString();
    }
    if (jsonObject.contains(QStringLiteral("HostName"))){
        hostName = jsonObject["HostName"].toString();
    }
    if (jsonObject.contains(QStringLiteral("Port"))){
        port = jsonObject["Port"].toInt();
    }
    if (jsonObject.contains(QStringLiteral("DatabaseName"))){
        dataBaseName = jsonObject["DatabaseName"].toString();
    }
//    if (jsonObject.contains(QStringLiteral("EventTable"))){
//        eventTable = jsonObject["EventTable"].toString();
//    }
    if (jsonObject.contains(QStringLiteral("UserName"))){
        userName = jsonObject["UserName"].toString();
    }
    if (jsonObject.contains(QStringLiteral("Password"))){
        password = jsonObject["Password"].toString();
    }

    return true;
}
