#ifndef CHARTVIEW_H
#define CHARTVIEW_H

/*************************************************************************************
** 描述：ChartView类继承自QChartView，可以直接使用父类中提供的事件函数和非私有功能，使用继承主要
**     考虑到后期功能的扩充和具体化
** 功能：显示波形图
** 作者：于海友
** ***********************************************************************************/


#include <QtCharts/QChartView>

class Chart;

QT_CHARTS_USE_NAMESPACE

class ChartView : public QChartView
{
public:
    ChartView();
    ~ChartView();
};

#endif
