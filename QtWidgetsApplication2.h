#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication2.h"
#include <QDate>
#include <QSettings>
#include <QThreadPool>
#include <QMutex>
#include <QWaitCondition>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QThread>
#include <QTimer>

class QtWidgetsApplication2 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication2(QWidget *parent = nullptr);
    ~QtWidgetsApplication2();

private:
    Ui::QtWidgetsApplication2Class ui;
    QTimer* dailyTimer;  // 每天执行定时器
    QString rootFolderPath;  // 根文件夹路径
    QDate lastProcessedDate;  // 上次处理日期
    QSettings* settings;  // 设置存储对象
    QTime selectedTime; // 设置时间
    //多线程处理相关
    QThreadPool threadPool;
    QAtomicInt totalFiles; // 总文件数
    QAtomicInt processedCount; // 已处理文件数
    QAtomicInt successCount; // 成功计数
    QAtomicInt errorCount; // 失败计数
    QAtomicInt cancelRequested; // 取消标志
    QAtomicInt pauseRequested;  // 暂停标志
    QMutex logMutex; // 日志互斥锁
    int currentQuality; // 当前质量设置
    QWaitCondition pauseCondition; // 暂停条件变量
    QMutex pauseMutex;          // 暂停互斥锁
    bool isRunning = false;     // 程序是否运行中
    // 添加日志相关成员
    QFile* logFile;           //日志文件对象
    QTextStream* logStream;   //日志流
    QDate currentLogDate;     //当前日志文件的日期
    QMutex logFileMutex;      //日志文件互斥锁

    QList<QWidget*> protectedWidgets; // 使用QWidget基类来存储不同类型控件
    bool locked;
    QTimer* lockTimer;


private slots: //相应功能槽函数
    void pushbuttonClicked();
    void pushbuttonClicked2();
    void pushbuttonClicked3();
    void pushbuttonClicked4();
    void pushbuttonClicked5();
    void pushbuttonClicked6();
    void pushbuttonClicked7();
    void spinBoxTextChanged();
    void manualOperation();
    void convertFolder(const QString& folderPath);
    void onRootFolderSelected();
    void onDailyTimer();
    void saveSettings();  // 保存设置
    void cancelProcessing(); // 取消处理
    void updateProgress(); // 更新进度
    void onProcessingFinished(); //处理完成
    void toggleRunState(); // 切换运行状态
    void pauseProcessing(); // 暂停处理
    void resumeProcessing(); // 恢复处理
    void logMessage(const QString& message);
    // 添加日志方法
    void initLogFile(); //初始化日志文件
    void rotateLogFile(); //轮换日志文件（按天）
    void writeToLogFile(const char* message); // 写入日志到文件
    void onLockButtonClicked();
    void lockControls(bool lock);
    void resetTimer();

public slots:
    void processFile(const QString& filePath); // 处理单个文件的槽函数

signals:
    void fileProcessed(bool success);
};
