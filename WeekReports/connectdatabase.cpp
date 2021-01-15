#include "connectdatabase.h"


int ConnectDataBase::EVENTID = 0;

//QString ConnectDataBase::SQLTABLE = "";

int ConnectDataBase::WHICHREGION = 0;

//连接数据库
void ConnectDataBase::conDataBase()
{
    db = QSqlDatabase::addDatabase(DataBaseConfigure::driver);  //连接的MYSQL的数据库驱动
    db.setHostName(DataBaseConfigure::hostName);               //连接数据库主机名，这里需要注意（若填的为”127.0.0.1“，出现不能连接，则改为localhost)
    db.setPort(DataBaseConfigure::port);                       //端口
    db.setDatabaseName(DataBaseConfigure::dataBaseName);       //数据库名
    db.setUserName(DataBaseConfigure::userName);               //用户名
    db.setPassword(DataBaseConfigure::password);               //密码
    db.open();

    if(!db.open()){                            //打开
        qDebug()<<"the connection to database is failed";
        return ;
    }
    else{
         qDebug()<<"the connection to database is successful";
    }
}

//释放连接
void ConnectDataBase::close()
{
    db.close();
    qDebug()<<"ConnectDataBase::close()";
}

void ConnectDataBase::updateDataBase(int id,double xData,double yData,double zData,double Parrival)
{
    QSqlQuery query;
    //更新mine_quack_results表

    query.prepare("update mine_quack_results set xData=?,yData=?,zData=?,Parrival=? where id =?");
    //query.addBindValue(table);
    query.addBindValue(xData);
    query.addBindValue(yData);
    query.addBindValue(zData);
    query.addBindValue(Parrival);
    query.addBindValue(id);
    try{
        query.exec();
        qDebug()<<"update mine_quack_results at "<<id<<"successed";
    }catch(QString str){
        QMessageBox::about(nullptr,"Error",str);
    }

}
