#ifndef CHART_H
#define CHART_H

/****************************************************************************
** 描述：Chart类继承自QChart，可以直接使用父类中提供的事件函数和非私有功能，使用继承主要
**     考虑到后期功能的扩充和具体化
** 功能：显示波形图
** 作者：于海友
*****************************************************************************/


#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QSplineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QLineSeries>

QT_CHARTS_USE_NAMESPACE

class Chart : public QChart
{
    Q_OBJECT
public:
    explicit Chart();
    ~Chart();

};

#endif // CHART_H
