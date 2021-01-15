#include "reportform.h"
#include "ui_reportform.h"

ReportForm::ReportForm(QWidget *parent):
    QWidget(parent),ui(new Ui::ReportForm)
{
    ui->setupUi(this);

    initZCharts();

    blinkId = new QQueue<qlonglong>();
    textId = new QQueue<qlonglong>();
    pointId = new QQueue<qlonglong>();

    if(DataBaseConfigure::getDataBaseConfigure()){
        db = new ConnectDataBase;
    }

    miningAreaMap = MiningAreaMap::getMiningAreaMapConfigure();
    ui->miningAreaBox->addItems(miningAreaMap.keys());
    miningAreaValue = miningAreaMap.value(ui->miningAreaBox->currentText());

    ui->startDate->setMinimumDate(QDate::currentDate().addDays(-3650));   // -3650天
    ui->startDate->setCalendarPopup(true);                                // 日历弹出
    ui->endDate->setMaximumDate(QDate::currentDate().addDays(3650));      // +3650天
    ui->endDate->setCalendarPopup(true);

    ui->axWidget->dynamicCall("EnableGripPoint(bool)",false);
    ui->axWidget->dynamicCall("DoCommand(const qint32&)",1); //执行控件自定义命令函数，命令的id为2，该值为命令编号，可任取.

    ui->csvList->setEditTriggers(QAbstractItemView::NoEditTriggers);  //禁止编辑
    ui->csvList->setSelectionBehavior(QAbstractItemView::SelectRows); //整行选中的方式
    ui->csvList->setSelectionMode(QAbstractItemView::SingleSelection);//单击选中整行
    ui->csvList->verticalHeader()->setVisible(true); //列表头
    ui->csvList->horizontalHeader()->setVisible(true); //行表头
    ui->csvList->setColumnCount(2); //设置列数
    ui->csvList->setRowCount(30);//行数
    QStringList sListHeader;
    sListHeader << "路径" << "台站";
    ui->csvList->setHorizontalHeaderLabels(sListHeader); //设置每一列的表头
//    int iRow = ui->csvList->rowCount();
//    ui->csvList->setRowCount(iRow + 1);//总行数增加1

    isManual = true;

    connect(ui->moveCAD,SIGNAL(clicked()),this,SLOT(moveCADButtonClicked()));
    connect(ui->restoreCAD,SIGNAL(clicked()),this,SLOT(restoreCADButtonClicked()));
    connect(ui->captureByRobot,SIGNAL(clicked()),this,SLOT(captureByRobotClicked()));
    connect(ui->captureByManual,SIGNAL(clicked()),this,SLOT(captureByManualClicked()));
    connect(ui->saveCapturedImageByManual,SIGNAL(clicked()),this,SLOT(saveCapturedImageByManual()));
    connect(ui->queryButton,SIGNAL(clicked()),this,SLOT(queryButtonClicked()));
    connect(ui->clearTwinkeButton,SIGNAL(clicked()),SLOT(clearTwinkeButtonClickled()));
    //connect(ui->generateDOC,SIGNAL(clicked()),this,SLOT(generateDOCClicked()));
    connect(ui->generateWebDOC,SIGNAL(clicked()),this,SLOT(generateWebDOCClicked()));
    connect(ui->readCSVInList,SIGNAL(clicked()),this,SLOT(readCSVInListClicked()));
    qDebug()<<"checkpoint4";


    // 日期发生改变时执行
    connect(ui->startDate, SIGNAL(dateChanged(QDate)), this, SLOT(onStartDateChanged(QDate)));
    connect(ui->endDate, SIGNAL(dateChanged(QDate)), this, SLOT(onEndDateChanged(QDate)));

    // 无论日期还是时间发生改变，都会执行
    //connect(ui->startDate, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onDateTimeChanged(QDateTime)));
    // 时间发生改变时执行
    //connect(ui->startDate, SIGNAL(timeChanged(QTime)), this, SLOT(onTimeChanged(QTime)));

    connect(ui->csvList,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),this, SLOT(getDoubleClickedItem(QTableWidgetItem*)));
    connect(ui->csvList,SIGNAL(itemClicked(QTableWidgetItem*)),this, SLOT(getSingleClickedItem(QTableWidgetItem*)));
    connect(ui->miningAreaBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
          [=](const QString &text){
        miningAreaValue = miningAreaMap.value(text);
        ui->axWidget->dynamicCall("DoCommand(const qint32&)",1);
    });
}

ReportForm::~ReportForm(){
    delete blinkId;
    delete textId;
    delete db;
    qDebug()<<"distructed ReporForm";
}

void ReportForm::initZCharts()
{
    splineSeries = new QSplineSeries[9];
    lineSeries = new QLineSeries[9];
    chart = new Chart[9];
    axisX = new QValueAxis[9];
    axisY = new QValueAxis[9];
    view = new ChartView [9];

    QMargins margin(1,1,1,1);   //设置chart rectangle（图表矩形）的边距
    //QRectF recf(5,5,550,50);  //显示设置plot area（图表区域）的大小

    for (int i=0;i<9;i++) {
        splineSeries[i].setColor(QColor(Qt::black));
        splineSeries[i].setUseOpenGL(true);
        splineSeries[i].setPointsVisible(true);
        lineSeries[i].setColor(QColor(255,0,0));
        lineSeries[i].setPointLabelsVisible(true);
        lineSeries[i].setPointLabelsFormat("(@xPoint, @yPoint)");
        chart[i].legend()->hide();
        chart[i].addSeries(&splineSeries[i]);              //为图表添加曲线序列
        chart[i].addSeries(&lineSeries[i]);                //为图标添加折线序列

        chart[i].setBackgroundVisible(true);
        chart[i].setMargins(margin);
        //chart[i].setPlotArea(recf);

        axisX[i].setRange(0, 9000);                       //设置坐标轴范围
        axisX[i].setTickCount(10);                         //9区域，10个刻度
        axisX[i].setMinorTickCount(1);                     //单位刻度中间再加一个副刻度

        axisY[i].setRange(-20000, 20000);
        axisY[i].setTickCount(5);

        chart[i].addAxis(&axisX[i], Qt::AlignBottom);      //把坐标轴添加到chart中，第二个参数是设置坐标轴的位置，
        splineSeries[i].attachAxis(&axisX[i]);             //把曲线关联到坐标轴
        lineSeries[i].attachAxis(&axisX[i]);

        chart[i].addAxis(&axisY[i], Qt::AlignLeft);
        splineSeries[i].attachAxis(&axisY[i]);
        lineSeries[i].attachAxis(&axisY[i]);

        view[i].setChart(&chart[i]);
        view[i].setRubberBand(QChartView::RectangleRubberBand);
        view[i].installEventFilter(this);                  //注册部件事件过滤
        QSize s(1000,500);

        view[i].setMaximumSize(s);
    }

    for(int i=0;i<3;i++){
        ui->gridLayout1->addWidget(&view[i],i,1);
    }
    for(int i=3;i<6;i++){
        ui->gridLayout2->addWidget(&view[i],i-3,1);
    }
    for(int i=6;i<9;i++){
        ui->gridLayout3->addWidget(&view[i],i-6,1);
    }
}


//读取CSV文件数据
bool ReportForm::readCSVFileOfZ(QString fileName)
{
    if(fileName.isEmpty()||fileName.split('/').size() == 0){
        //路径为空
        qDebug()<<"fileName is empty or filepath is wrong";
        ui->statusOfProgramme->setText("文件读取失败，请检查文件路径");
        QMessageBox::warning(this,tr("警告"),tr("请检查文件路径"),QMessageBox::Yes);
        return false;
    }

    QFile file(fileName);
    qDebug()<<"fileName="<<fileName<<'\n';
    QStringList line;
    QStringList item;
    QString block ;
    int k=0;

    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        //文件打开失败
        qDebug()<<"open file failed!";
        //QMessageBox::warning(this,tr("警告"),tr("此文件读取失败"),QMessageBox::Yes);
        ui->statusOfProgramme->setText("此文件打开失败");

        QMessageBox *box = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("此文件读取失败"));
        QTimer::singleShot(3000,box,SLOT(accept())); //也可将accept改为close
        box->exec();//box->show();都可以
        return false;
    }

    QTextStream stream(&file);
    while (!stream.atEnd())
    {
        block = stream.readAll();
        line = block.split('\t');
    }
    int count = line.size();
    qDebug()<<"COUNT ="<<count;
    //限制读取的行数 最多为90000行，如果超过90000行，则舍去多余数据
    if(count > 90000){
        count = 90000;
    }

    //先获取第一行的内容，然后看看该行中有多少个台站，因为文件路径中的盘符所对应的台站名是有激发的
    //而某些CSV文件中存在不激发但是有数据的台站，所以不能根据路径中的盘符来判断
    QStringList valueOfOneLine = line.at(0).split(",");
    int valueOfOneLineSize = valueOfOneLine.size();
    int sennumInRealCSV = (valueOfOneLineSize-1)/8;
//    QStringList panfuListInRealCSV;
//    QString panfuInRealCSV="";
//    for(QString temp:panfuListInRealCSV){
//        panfuInRealCSV+=temp;
//    }

    QStringList fileSplits = fileName.split('/');
    QString panfu = fileSplits[fileSplits.length()-1].split(' ')[0];
    int sennum = panfu.length();

    qDebug()<<"SENNUM="<<sennum<<" sennumInRealCSV="<<sennumInRealCSV;
    qDebug()<<"panfu="<<panfu<<" panfuInRealCSV="<<panfu<<"+"<<sennumInRealCSV-sennum;

//    if(sennumInRealCSV>sennum){
//        sennum = sennumInRealCSV;
//        panfu = panfuInRealCSV;
//    }


    //存储每个事件触发台站的X，Y，Z轴的数据
    pointBuffer = new QVector<QPointF>[sennumInRealCSV];
    //用于绘制P波到时（红线）
    pointBuffer_P = new QVector<QPointF>[sennumInRealCSV];

    int *motiPos;                            //每个台站的激发位置
    //存储每个传感器波形的激发位置，在CSV文件中表现为第H列，P列，X列...以此类推
    motiPos = new int[sennumInRealCSV];

    QString **senChannelZ ;                  //存储每个事件后三个通道中Z轴的数据
    //存储后三个通道中Z轴的数据，在CSV文件中表现为G列，O列，W列...以此类推
    senChannelZ = new QString*[9000];
    for (int i = 0; i < 9000; i++){
        senChannelZ[i] = new QString[sennumInRealCSV];
      //  qDebug()<<"senChannel[i].size="<<senChannel[i]->size();
    }

    QString *senChannelNum ;                //存储每个事件触发台站名称
    //存储每个事件触发的台站名称，在CSV文件中表现为I列，Q列，Y列...以此类推
    senChannelNum = new QString[sennumInRealCSV];
    QChar ch;
    //panfu是按照到时顺序来的
    for(int i=0;i<sennumInRealCSV;i++){
        if(i<sennum){
            ch = panfu.at(i);
            if(ch == 'r'){
                senChannelNum[i] = R;
            }
            if(ch == 's'){
                senChannelNum[i] = S;
            }
            if(ch == 't'){
                senChannelNum[i] = T;
            }
            if(ch == 'u'){
                senChannelNum[i] = U;
            }
            if(ch == 'v'){
                senChannelNum[i] = V;
            }
            if(ch == 'w'){
                senChannelNum[i] = W;
            }
            if(ch == 'x'){
                senChannelNum[i] = X;
            }
            if(ch == 'y'){
                senChannelNum[i] = Y;
            }
            if(ch == 'z'){
                senChannelNum[i] = Z;
            }
        }
        else{
            //ch = panfuInRealCSV.at(i);
            senChannelNum[i] = Undefined;
        }
    }

    //这时候已经把盘符获取到了，同时台站名也知道了，然后就把标签设置成台站名称
    //至少有三个台站，所以前三个一定有内容
    ui->station1->setText(senChannelNum[0]);
    ui->station2->setText(senChannelNum[1]);
    ui->station3->setText(senChannelNum[2]);
    if(sennumInRealCSV == 4){
        ui->station4->setText(senChannelNum[3]);
    }else if(sennumInRealCSV == 5){
        ui->station4->setText(senChannelNum[3]);
        ui->station5->setText(senChannelNum[4]);
    }else if(sennumInRealCSV == 6){
        ui->station4->setText(senChannelNum[3]);
        ui->station5->setText(senChannelNum[4]);
        ui->station6->setText(senChannelNum[5]);
    }else if(sennumInRealCSV == 7){
        ui->station4->setText(senChannelNum[3]);
        ui->station5->setText(senChannelNum[4]);
        ui->station6->setText(senChannelNum[5]);
        ui->station7->setText(senChannelNum[6]);
    }else if(sennumInRealCSV == 8){
        ui->station4->setText(senChannelNum[3]);
        ui->station5->setText(senChannelNum[4]);
        ui->station6->setText(senChannelNum[5]);
        ui->station7->setText(senChannelNum[6]);
        ui->station8->setText(senChannelNum[7]);
    }else if(sennumInRealCSV == 9){
        ui->station4->setText(senChannelNum[3]);
        ui->station5->setText(senChannelNum[4]);
        ui->station6->setText(senChannelNum[5]);
        ui->station7->setText(senChannelNum[6]);
        ui->station8->setText(senChannelNum[7]);
        ui->station9->setText(senChannelNum[8]);
    }

    qDebug()<<"parsing the csv file completed";



    for(int i=0;i<(count-1);i++){
        item = line.at(i).split(',');
        if(0 == i%10){                                    //采样，只取十分之一的数据
            for(int j=0;j<sennumInRealCSV;j++){
                senChannelZ[k][j] = item.at(6+j*8);       //存储每个事件后三个通道中Z轴的数据
                int tempZ = senChannelZ[k][j].toInt();
                if(tempZ > maxValueOfZChannel)
                    maxValueOfZChannel = tempZ;

                if(i == 0) motiPos[j] = item.at(7+j*8).toInt();   //存储每个传感器波形激发位置，每行都一样，所以存第一行就行
                //qDebug()<<"senChannelX[i][j]="<<senChannelX[i][j]<<'\n';
            }
            //qDebug()<<"K="<<k;
            k++;
        }
        //qDebug()<<"i="<<i<<'\n';
    }
    file.close();

    qDebug()<<"maxValueOfXYZChannel"<<maxValueOfZChannel;
    qDebug()<<"read csv file successfully!";

    //若maxValueOfZChannel存在极端值，取最大值50000
    if(maxValueOfZChannel >= 50000){
        maxValueOfZChannel = 50000;
    }
    for(int i=0;i<9;i++){
        axisY[i].setRange(-maxValueOfZChannel, maxValueOfZChannel);   //设置坐标轴范围
    }

    for(int j=0;j<sennumInRealCSV;j++){
        //qDebug()<<"motivated position is "<<motiPos[j];  //打印测试站台及其激发位置
        //因为P波到时是一条垂线，所以只需要添加横坐标相同的几个点就好
        pointBuffer_P[j].append(QPointF(motiPos[j]/10,0));
        pointBuffer_P[j].append(QPointF(motiPos[j]/10,50000));
        pointBuffer_P[j].append(QPointF(motiPos[j]/10,-50000));
    }

    int listIndex = 0; 	                                      //可以理解为横坐标，因为横坐标就是每个事件（也就是CSV文件中的一行数据）
    double parseDataZ;
    for(int j=0;j<count;j++){
        if(0 == j%10){
            for(int i=0;i<sennumInRealCSV;i++){
                parseDataZ = senChannelZ[listIndex][i].toDouble();
                pointBuffer[i].append(QPointF(listIndex, parseDataZ));
                //qDebug()<<"parseDataZ="<<parseDataZ<<'\n';
                //qDebug()<<"listIndex= "<<listIndex;
            }
            listIndex++;
        }
    }
    delete[] motiPos;
    delete[] senChannelNum;
    for( int i = 0 ; i < 9000 ; i ++ )
        delete []senChannelZ[i] ;
    delete []senChannelZ;

    ui->statusOfProgramme->setText("文件读取完毕");

    //每次绘制曲线之前都需要清空上一次数据，所有台站曲线数据都要清空，因为假如一开始有5个，后来有3个台站，那么数据就会混杂在一起
    for(int i=0;i<9;i++){
        splineSeries[i].clear();
        lineSeries[i].clear();
    }
    for(int i=0;i<9;i++){
        //如果有5个台站，那么只需要替换5个台站的曲线数据
        if(i< sennumInRealCSV){
            splineSeries[i].replace(pointBuffer[i]);
            lineSeries[i].replace(pointBuffer_P[i]);
        }
    }

    //绘制曲线完毕后，把容器的数据删除，然后删除对象
    for(int i=0;i<sennumInRealCSV;i++){
        pointBuffer[i].clear();
        pointBuffer_P[i].clear();
    }
    delete [] pointBuffer;
    delete [] pointBuffer_P;

    ui->statusOfProgramme->setText("绘制完毕");
    if(isManual){
        QMessageBox::warning(this,tr("完成"),tr("该文件读取完毕"),QMessageBox::Yes);
    }
    return true;
}

void ReportForm::on_axWidget_ImplementCommandEvent(int iCommandId)
{
    if(iCommandId == 1){
        QColor c(255,255,255);
        ui->axWidget->dynamicCall("SetViewColor(QColor)", c);

        QString path = QDir::currentPath();
        qDebug()<<"当前路径"<<path;
        QString tempPath;
        // 调用控件打开dwg文件命令，使用相对路径

        //QString path = QApplication::applicationDirPath();    //是获取的执行文件exe所在的路径
        tempPath=path + "/src/dwg/"+miningAreaValue;
        ui->axWidget->dynamicCall("OpenDwgFile(const QString&)",tempPath);
        qDebug()<<"tempPath:"<<tempPath;
        tempPath.clear();
    }
    if(iCommandId == 5){
        // 调用控件缩放命令。
        ui->axWidget->dynamicCall("ZoomScale(double dScale)",0.8);
    }
    if(iCommandId == 6){
        ui->axWidget->dynamicCall("ZoomScale(double dScale)",1.1);
    }
    if(iCommandId == 7){
        // 调用控件视区还原命令
        ui->axWidget->dynamicCall("ZoomAll()");
    }
    if(iCommandId == 8){
        // 调用控件移动视区命令
        ui->axWidget->dynamicCall("SendStringToExecute(P)");
    }
    if(iCommandId == 9){
        while(!blinkId->empty()){
            qlonglong temp = blinkId->dequeue();
            //ui->axWidget->dynamicCall("StopTwinkeEnt(qlonglong)",temp);
            ui->axWidget->dynamicCall("Clear(qlonglong)",temp);
        }
        while(!textId->empty()){
            qlonglong temp = textId->dequeue();
            ui->axWidget->dynamicCall("Erase(qlonglong)", temp);
        }
        while(!pointId->empty()){
            qlonglong temp = pointId->dequeue();
            ui->axWidget->dynamicCall("Erase(qlonglong)", temp);
        }
    }
}

void ReportForm::moveCADButtonClicked()
{
    ui->axWidget->dynamicCall("DoCommand(const qint32&)",8);
}

void ReportForm::restoreCADButtonClicked()
{
    ui->axWidget->dynamicCall("DoCommand(const qint32&)",7);
}

// 点击手动截图按钮开始截图;
void ReportForm::captureByManualClicked()
{
    isManual = true;
    captureHelper = new CaptureScreen();
    connect(captureHelper, SIGNAL(signalCompleteCature(QPixmap)), this, SLOT(onCompleteCature(QPixmap)));
    captureHelper->show();
}

void ReportForm::onCompleteCature(QPixmap captureImage)
{
    ui->displayCapturedImage->setPixmap(captureImage);
    delete captureHelper;
}

//保存手动截图图片
void ReportForm::saveCapturedImageByManual()
{
    QString pathsave = QFileDialog::getSaveFileName(this,"Save","../",tr("Images (*.png *.xpm *.jpg)"));
    QFile outFile(pathsave);
    if(!outFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,tr("警告"),tr("保存失败"),QMessageBox::Yes);
        return;
    }
    ui->displayCapturedImage->pixmap()->save(&outFile, "PNG");
    QMessageBox::warning(this,tr("完成"),tr("截图已保存"),QMessageBox::Yes);

}

//自动截图
void ReportForm::captureByRobotClicked()
{
    isManual = false;

    int i = 1;
    for(QString filePath:listOfFilePath){
        if(readCSVFileOfZ(filePath)){
            ui->tabWidget->setCurrentIndex(0);
            QRect rect = ui->gridLayout1->geometry();
            QPixmap p = this->grab(rect);
            QString filePathName1 = QDir::currentPath() + "/captures/"+startDate + "-"+endDate+"capture"+QString::number(i)+"_1";
            //filePathName += QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz");
            filePathName1 += ".png";
            if(!p.save(filePathName1,"png")){
                ui->statusOfProgramme->setText("自动截图"+filePathName1+"保存失败");
                qDebug()<<"save widget screen failed";
            }

            ui->tabWidget->setCurrentIndex(1);
            QRect rect2 = ui->gridLayout2->geometry();
            QPixmap p2 = this->grab(rect2);
            QString filePathName2 = QDir::currentPath() + "/captures/"+startDate + "-"+endDate+"capture"+QString::number(i)+"_2";
            filePathName2 += ".png";
            if(!p2.save(filePathName2,"png")){
                ui->statusOfProgramme->setText("自动截图"+filePathName2+"保存失败");
                qDebug()<<"save widget screen failed";
            }

            ui->tabWidget->setCurrentIndex(2);
            QRect rect3 = ui->gridLayout3->geometry();
            QPixmap p3 = this->grab(rect3);
            QString filePathName3 = QDir::currentPath() + "/captures/"+startDate + "-"+endDate+"capture"+QString::number(i)+"_3";
            filePathName3 += ".png";
            if(!p3.save(filePathName3,"png")){
                ui->statusOfProgramme->setText("自动截图"+filePathName3+"保存失败");
                qDebug()<<"save widget screen failed";
            }

            i++;
            if((i-1)== listOfFilePath.size()){
                //记得要清掉保存的路径，不然下次再查询会重复添加
                listOfFilePath.clear();
                QMessageBox::warning(this,tr("完成"),tr("自动截图已完成，保存至/captures"),QMessageBox::Yes);
            }
        }else{
            QMessageBox *msg = new QMessageBox(this);
            msg->setWindowTitle(tr("错误"));
            msg->setText(tr("在自动截图过程中，发生异常"));
            msg->setIcon(QMessageBox::Warning);
            QPushButton *conntinueButton = msg->addButton(tr("继续执行"), QMessageBox::ActionRole);
            QPushButton *abortButton = msg->addButton(tr("退出"),QMessageBox::RejectRole);
            msg->exec();
            if (msg->clickedButton() == conntinueButton) {
                continue;
            } else {
                return;
            }
        }
    }
}


void ReportForm::getDoubleClickedItem(QTableWidgetItem *item)
{
    QString text = item->text();
    qDebug()<<"getDoubleClickedItem:"<<text;
    readCSVFileOfZ(text);
}

void ReportForm::getSingleClickedItem(QTableWidgetItem *item)
{
    filePathOnTableWidget = item->text();
    qDebug()<<"getSingleClickedItem:"<<filePathOnTableWidget;
}

void ReportForm::readCSVInListClicked()
{
    readCSVFileOfZ(filePathOnTableWidget);
}

void ReportForm::queryButtonClicked()
{
     db->conDataBase();
     startDate = ui->startDate->date().toString("yyyy-MM-dd");
     endDate = ui->endDate->date().toString("yyyy-MM-dd");

     //QString strBuffer = time.toString("yyyy-MM-dd hh:mm:ss");
     //QString strBuffer = "2010-07-02 17:35:00";
     //QDateTime time = QDateTime::fromString(strBuffer, "yyyy-MM-dd hh:mm:ss");

     //以下执行相关sql语句
     QSqlQuery query;
     QString sql = "select quackTime,kind,xData,yData,zData,quackGrade,Parrival,panfu,nengliang,wenjianming from mine_quack_results where quackTime>='"+
             startDate+" 00:00:00' and quackTime<='"+endDate+" 23:59:59' ";
     query.exec(sql);            // 执行查询操作

     mapParameter.clear();           //每次查询时要把上一次查询的数据库结果清除
     ui->csvList->clearContents();   //只清除表中数据，不清除表头内容

     QString quackTime;
     QString kind;
     double xData;
     double yData;
     double zData;
     double quackGrade;
     double Parrival;
     QString panfu;
     double nengliang;
     QString wenjianming;

     QMap<QString,QList<double>> map ;
     QList<double> list;
     QList<double> coordinates;
     QList<QVariant> dataParameters;

     int index = 0;           //tablewidget的索引，用来插入wenjianming和panfu

     //query.next()指向查找到的第一条记录，然后每次后移一条记录
     while(query.next())
     {
         quackTime = query.value(0).toString();
         dataParameters.append(quackTime);

         kind = query.value(1).toString();
         dataParameters.append(kind);

         xData = query.value(2).toDouble();
         dataParameters.append(xData);

         yData = query.value(3).toDouble();
         dataParameters.append(yData);

         zData = query.value(4).toDouble();
         dataParameters.append(zData);

         quackGrade = query.value(5).toDouble();
         dataParameters.append(quackGrade);

         Parrival = query.value(6).toDouble();
         dataParameters.append(Parrival);

         panfu = query.value(7).toString();
         dataParameters.append(panfu);

         nengliang = query.value(8).toDouble();
         dataParameters.append(nengliang);

         wenjianming = query.value(9).toString();
         dataParameters.append(wenjianming);

         list.append(xData);
         list.append(yData);

         map.insertMulti(kind,list);
         list.clear();
         qDebug()<<"kind = "<<kind<<" xData="<<xData<<" yData="<<yData;

         mapParameter.insert(quackTime,dataParameters);

         for(int i=0;i<dataParameters.size();i++)
         qDebug()<<"param:"<<dataParameters.at(i);
         dataParameters.clear();

         if(!listOfFilePath.contains(wenjianming)){
             listOfFilePath.append(wenjianming);
         }
         ui->csvList->setItem(index,0,new QTableWidgetItem(wenjianming));
         ui->csvList->setItem(index,1,new QTableWidgetItem(panfu));

         index ++;           //行数加一
     }

     QString pointPath=QDir::currentPath() + "/centerpoint.png";
     QString pointPath2=QDir::currentPath() + "/centerpoint2.png";
     QString pointPath3=QDir::currentPath() + "/centerpoint3.png";

     QMap<QString,QList<double>>::Iterator it=map.begin();
     while(it!=map.end()){
         //qDebug()<<it.key()<<"\t"<<it.value();

         if(it.key().compare("three") == 0){
            QColor color1(255,0,0);
            ui->axWidget->dynamicCall("SetDrawColor(QColor)", color1);

            coordinates = it.value();
            //qDebug()<<"coordinates x:"<<coordinates.at(0)<<"coordinates y:"<<coordinates.at(1);
            //X坐标,Y坐标，文字，文字高度，旋转角度，水平对齐（0=kTextLeft,1=kTextCenter,2=kTextRight ），垂直对齐（1=kTextBottom,2=kTextVertMid,3=kTextTop ）
            qlonglong result = ui->axWidget->dynamicCall("DrawText(double, double, QString, double, double, int, int)",
                                                             coordinates.at(0),coordinates.at(1),"three",250,0,0,1).toLongLong();
            textId->enqueue(result);

            qlonglong result2 = ui->axWidget->dynamicCall("DrawCircle(double, double, double)",coordinates.at(0),coordinates.at(1),400).toLongLong();
            blinkId->enqueue(result2);

            qlonglong result3 = ui->axWidget->dynamicCall("DrawImage(double, double, double, double, QString)",
                                                          coordinates.at(0)-50,coordinates.at(1)-50,2,0,pointPath).toLongLong();
            pointId->enqueue(result3);
         }else if(it.key().compare("five") == 0){
            QColor color2(0,255,0);
            ui->axWidget->dynamicCall("SetDrawColor(QColor)", color2);
            coordinates = it.value();
            qlonglong result =ui->axWidget->dynamicCall("DrawText(double, double, QString, double, double, int, int)",
                                                             coordinates.at(0),coordinates.at(1),"five",250,0,0,1).toLongLong();
            textId->enqueue(result);

            qlonglong result2 = ui->axWidget->dynamicCall("DrawCircle(double, double, double)",coordinates.at(0),coordinates.at(1),400).toLongLong();
            blinkId->enqueue(result2);

            qlonglong result3 = ui->axWidget->dynamicCall("DrawImage(double, double, double, double, QString)",
                                                          coordinates.at(0)-50,coordinates.at(1)-50,1,0,pointPath2).toLongLong();
            pointId->enqueue(result3);
         }else if(it.key().compare("PSO") == 0){
            QColor color3(0,0,255);
            ui->axWidget->dynamicCall("SetDrawColor(QColor)", color3);
            coordinates = it.value();
            qlonglong result = ui->axWidget->dynamicCall("DrawText(double, double, QString, double, double, int, int)",
                                                             coordinates.at(0),coordinates.at(1),"PSO",250,0,0,1).toLongLong();
            textId->enqueue(result);

            qlonglong result2 = ui->axWidget->dynamicCall("DrawCircle(double, double, double)",coordinates.at(0),coordinates.at(1),400).toLongLong();
            blinkId->enqueue(result2);

            qlonglong result3 = ui->axWidget->dynamicCall("DrawImage(double, double, double, double, QString)",
                                                          coordinates.at(0)-50,coordinates.at(1)-50,1,0,pointPath3).toLongLong();
            pointId->enqueue(result3);
         }
         it++;
     }
     db->close();

//     注意 不能这样使用迭代器
//     QMap<QString,QList<QVariant>>::Iterator iterator=mapParameter.begin();
//     while(iterator != mapParameter.end()){
//         qDebug()<<iterator.key();//<<"\t"<<iterator.value()<<"-----------";
//     }

}

void ReportForm::clearTwinkeButtonClickled()
{
    ui->axWidget->dynamicCall("DoCommand(const qint32&)",9);
}

//该功能是利用word模板，在模板中加标签，然后查询标签得到定位字符再替换，此方法麻烦，且为完成，舍弃
void ReportForm::generateDOCClicked()
{
    QString locationPath=QDir::currentPath() + "/locationresult.jpg";
    ui->axWidget->dynamicCall("SaveJpgFile(QString, int, int, int)",locationPath,-1,-1,0);

    QString templatePath=QDir::currentPath() + "/template.docx";
    if(templatePath.isEmpty()==false){
        QFile file(templatePath);
        QAxObject *words= new QAxObject(this);//定义一个指针
        words->setControl("word.Application");//指向com文件类型setControl
        words->setProperty("Visible",false);//设置是否提示或者显示对话框setProperty

        QAxObject *workbooks = words->querySubObject("Documents");//获取工作簿(excel文件)集合querySubObject
        workbooks->dynamicCall("Add(QString)", templatePath);//链接到具体的文件dynamicCall

        //path至关重要，获取excel文件的路径
        //打开一个word文件，获取文档
        QAxObject *workbook = words->querySubObject("ActiveDocument");//获取word中已经有数值的区域querySubObject

        /********修改一个位置的内容*********/
        QString sLabel="event";//此处必须为文档中被标记为书签的内容，下面语句的意思就是确定书签的位置：
        QAxObject *pBookmark = workbook->querySubObject("Bookmarks(QString)",sLabel);//找到活跃区域内的书签querySubObject
        QString sText;//此处为替换内容
        pBookmark->dynamicCall("Select(void)");//选中要选中的区域
        pBookmark->querySubObject("Range")->setProperty("Text",sText);//进行替换操作

        /********修改一个位置的内容*********/
        QString tLabel="time";//此处必须为文档中被标记为书签的内容，下面语句的意思就是确定书签的位置：
        QAxObject *tBookmarks = workbook->querySubObject("Bookmarks(QString)",tLabel);//找到活跃区域内的书签querySubObject
        QString tText;//此处为替换内容
        tBookmarks->dynamicCall("Select(void)");//选中要选中的区域
        tBookmarks->querySubObject("Range")->setProperty("Text",tText);//进行替换操作


        QAxObject *bookmark_pic = workbook->querySubObject("Bookmarks(QVariant)", "location");
        if ( !bookmark_pic->isNull())
        {
            bookmark_pic->dynamicCall("Select(void)");
            QAxObject *shapes = workbook->querySubObject("InlineShapes");
            shapes->dynamicCall("AddPicture(Const QString&)", locationPath);
        }

        /*****保存*******/
        QString pathsave = QFileDialog::getSaveFileName(this,"Save","../","word(*doc)");
        if(pathsave.isEmpty()==true)
        {
            return;
        }
        //document->dynamicCall("SaveAs (const QString&)", QString("F:/testword.doc"));
        workbook->dynamicCall("SaveAs(const QString&))",QDir::toNativeSeparators(pathsave));

        //words->setProperty("DisplayAlerts", true);//含有保存的功能,但是无法获得保存的路径，不容易进行下一步操作

        workbook->dynamicCall("Close(bool)", true);
        words->dynamicCall("Quit()");
        delete workbook;
        delete words;
        file.close();
//        if(QMessageBox::question(this,"完成","文件已经导出，是否需要打开",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes){
//            QDesktopServices::openUrl(QUrl("file:///"+QDir::toNativeSeparators(pathsave)));
//        }
        QMessageBox::warning(this,tr("完成"),tr("文件已经保存"),QMessageBox::Yes);
    }

}

//该功能是利用html转word,使用简便
void ReportForm::generateWebDOCClicked()
{
    QString locationPath=QDir::currentPath() + "/locationresult.jpg";
    QString capturesPath=QDir::currentPath() + "/captures/";

    //下面这几句是过滤截图文件
    QQueue<QString> capturesOfPng ;
    QDir dir;
    QStringList filters;
    filters << "*.png";
    dir.setPath(capturesPath);
    if (!dir.exists()){
        qDebug()<<"captures目录不存在";
    }
    dir.setNameFilters(filters);
    QDirIterator iter(dir,QDirIterator::Subdirectories);
    while (iter.hasNext()){
        iter.next();
        QFileInfo info=iter.fileInfo();
        if (info.isFile()){
            //return info.absoluteFilePath().replace('/', '\\');
            QString temp = info.absoluteFilePath();
            if(temp.contains(startDate)&&temp.contains(endDate)){
                capturesOfPng.enqueue(temp);
            }
        }
    }

    //将CAD转JPG -1是白色，0是黑色
    ui->axWidget->dynamicCall("SaveJpgFile(QString, int, int, int)",locationPath,-1,-1,-1);

    QString quackTime;
    QString kind;
    double xData;
    double yData;
    double zData;
    double quackGrade;
    double Parrival;
    QString panfu;
    double nengliang;
    QString wenjianming;

    QString filename = QFileDialog::getSaveFileName(this,"Save File",startDate+"-"+endDate,"*.doc");
    QString html;
    //html += "<html xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w=\"urn:schemas-microsoft-com:office:word\" xmlns=\"http://www.w3.org/TR/REC-html40\"><head><meta http-equiv=Content-Type  content=\"text/html; charset=gb2312\" >"; //这句可加可不加。主要是因为我在word里把doc另存为html文件后，看到有这么个头标签，由此想到直接将html文档保存为doc文件。

    //如果查询日期为一天那么标题中只写当天日期，否则标题中写起始和结束日期
    if(startDate.compare(endDate)==0){
        html +="<h3 align=\"center\"><font face=\"宋体\" > ";
        html += "红阳三矿"+startDate;
        html += "矿震数据初步分析</font></h3>";
    }else{
        html += "<h3 align=\"center\"><font face=\"宋体\" >";
        html += "红阳三矿"+startDate+"到"+endDate;
        html += "矿震数据初步分析</font></h3> ";
    }

    QFile outFile(filename);
    if(!outFile.open(QIODevice::WriteOnly | QIODevice::Append )){
        return ;
    }

    QTextStream ts(&outFile);

    //先把大标题和第一个子标题写上，然后将html置为空
    html += "<h5 align=\"left\"><font face=\"宋体\" >1.基本参数：</font></h5> ";
    ts<<html<<endl;
    html = "";

    //quackTime为主键
    QList<QString> paramKeys = mapParameter.keys();
    QString compareFile ="";
    int i = 0;                                   //表示事件计数
    for(QString singleKey:paramKeys){
        qDebug()<<"singleKey:"<<singleKey;
        QList<QList<QVariant>> valueList = mapParameter.values(singleKey);
        // qDebug()<<"valueList.size():"<<valueList.size();
        for(QList<QVariant> temp:valueList){
            quackTime = temp.at(0).toString();
            kind = temp.at(1).toString();
            xData = temp.at(2).toDouble();
            yData = temp.at(3).toDouble();
            zData = temp.at(4).toDouble();
            quackGrade = temp.at(5).toDouble();
            Parrival = temp.at(6).toDouble();
            panfu = temp.at(7).toString();
            nengliang = temp.at(8).toDouble();
            wenjianming = temp.at(9).toString();

            //如果查询查到多个事件，那么将事件写为事件1：quackTime、事件2：quackTime...这样的形式
            //wenjiangming不同就认为是一次事件
            if(compareFile != wenjianming){
                compareFile = wenjianming;
                html += "<p style=\"font-size:10;\"><b>事件";
                html += QString::number(i+1)+":"+quackTime;
                html += " 分析结果如下：";
                html += "</b></p>";
                i++;
            }

            html += "<p style=\"font-size:10;\">" ;
            html +=  "类型: ";
            html +=  kind;
            html +=  "</p>";

            html += "<p style=\"font-size:10;\">" ;
            html +=  "最早到时: ";

            //D:/data/ConstructionData/5moti/syztwxu 2020-06-18 04-19-20`47.csv
            //到时为路径后边的日期
            QStringList wenjianmingDate = wenjianming.split(" ");

            QChar ch;
            for(int i=0;i<panfu.size();i++){
                ch = panfu.at(i);
                if(ch == 'r'){
                    //判断最早到时是哪个台站，因为panfu是按照到时顺序来的，所以索引为0的为最早到时
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  R+"R: s";
//                    html +=  "</p>";
                }
                if(ch == 's'){
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  S+"S: s";
//                    html +=  "</p>";
                }
                if(ch == 't'){
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  T+"T: s";
//                    html +=  "</p>";
                }
                if(ch == 'u'){
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  U+"U: s";
//                    html +=  "</p>";
                }
                if(ch == 'v'){
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  V+"V: s";
//                    html +=  "</p>";
                }
                if(ch == 'w'){
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  W+"W: s";
//                    html +=  "</p>";
                }
                if(ch == 'x'){
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  X+"X: s";
//                    html +=  "</p>";
                }
                if(ch == 'y'){
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  Y+"Y: s";
//                    html +=  "</p>";
                }
                if(ch == 'z'){
                    if(0 == i){
                        html +=  wenjianmingDate.at(1)+wenjianmingDate.at(2).split(".").at(0);
                        html +=  "</p>";
                    }
//                    html += "<p style=\"font-size:10;\">  " ;
//                    html +=  Z+"Z: s";
//                    html +=  "</p>";
                }

            }

            html += "<p style=\"font-size:10;color:#FF0000\">  定位坐标：x ：" ;
            html += QString::number(xData,'g',13)+"，y："+QString::number(yData,'g',13);
            html +=  "</p>";
            html += "<p style=\"font-size:10;color:#FF0000\">  震源深度估计：z ：" ;
            html +=  QString::number(zData,'g',13);
            html +=  "</p>";
            html += "<p style=\"font-size:10;\">  P波到时：" ;
            html +=  QString::number(Parrival,'g',13);
            html +=  "</p>";
            html += "<p style=\"font-size:10;color:#FF0000\">  震级：" ;
            html +=  QString::number(quackGrade);
            html +=  "</p>";
            html += "<p style=\"font-size:10;color:#FF0000\">  能量：" ;
            html +=  QString::number(nengliang);
            html +=  "</p>";
            html += "<p style=\"font-size:10;\">  发震时刻：" ;
            html +=  quackTime;
            html +=  "</p>";

            //每个事件的基本参数写完之后再将html置为空
            ts<<html<<endl;
            html = "";
        }

    }
    //如果每个事件的参数都写完之后，只需要在后边加一个定位图
    html += "<h5 align=\"left\">2.定位图：</h5> ";
    html += "<img align=\"middle\" src = \"" + locationPath + "\"  width=\"500\" height=\"500\" /><br>" ;

    //定位图后面加上波形图
    html += "<h5 align=\"left\">3、波形图：</h5> ";
    while(!capturesOfPng.isEmpty()){
        html += "<img align=\"middle\" src = \"" + capturesOfPng.dequeue() + "\"  width=\"500\" height=\"500\" /><br>" ;
    }

    ts<<html<<endl;
    html = "";
    outFile.close();
    QMessageBox::warning(this,tr("完成"),tr("文件已经保存"),QMessageBox::Yes);
}

// 日期发生改变时执行
void ReportForm::onStartDateChanged(const QDate &date) {
    qDebug() << "start date : " << date;
}

// 日期发生改变时执行
void ReportForm::onEndDateChanged(const QDate &date) {
    qDebug() << "end date : " << date;
}

//void ReportForm::test()
//{   QSqlQuery query;
//    //QSqlRecord rec = query.record();
//    query.exec("select * from student");
//    qDebug() << "exec next() :";
//    //开始就先执行一次next()函数，那么query指向结果集的第一条记录
//    if(query.next())
//    {
//       //获取query所指向的记录在结果集中的编号
//       int rowNum = query.at();
//       //获取每条记录中属性（即列）的个数
//       int columnNum = query.record().count();
//       //获取"name"属性所在列的编号，列从左向右编号，最左边的编号为0
//       int fieldNo = query.record().indexOf("name");
//       //获取id属性的值，并转换为int型
//       int id = query.value(0).toInt();
//       //获取name属性的值
//       QString name = query.value(fieldNo).toString();
//       //将结果输出
//       qDebug() << "rowNum is : " << rowNum
//                 << " id is : " << id
//                 << " name is : " << name
//                 << " columnNum is : " << columnNum;
//    }
// //定位到结果集中编号为2的记录，即第三条记录，因为第一条记录的编号为0
//    qDebug() << "exec seek(2) :";
//    if(query.seek(2))
//    {
//       qDebug() << "rowNum is : " << query.at()
//                 << " id is : " << query.value(0).toInt()
//                 << " name is : " << query.value(1).toString();
//    }
//    //定位到结果集中最后一条记录
//    qDebug() << "exec last() :";
//    if(query.last())
//    {
//       qDebug() << "rowNum is : " << query.at()
//                 << " id is : " << query.value(0).toInt()
//                 << " name is : " << query.value(1).toString();
//    }


//    QSqlQuery query;
//    query.prepare("insert into student (id, name) "
//                  "values (:id, :name)");
//    query.bindValue(0, 5);
//    query.bindValue(1, "sixth");
//    query.exec();
//    query.exec("select * from student");
//    query.last();
//    int id = query.value(0).toInt();
//    QString name = query.value(1).toString();
//    qDebug() << id << name;
//}
