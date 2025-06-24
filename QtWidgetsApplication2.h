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
    QTimer* dailyTimer;  // ÿ��ִ�ж�ʱ��
    QString rootFolderPath;  // ���ļ���·��
    QDate lastProcessedDate;  // �ϴδ�������
    QSettings* settings;  // ���ô洢����
    QTime selectedTime; // ����ʱ��
    //���̴߳������
    QThreadPool threadPool;
    QAtomicInt totalFiles; // ���ļ���
    QAtomicInt processedCount; // �Ѵ����ļ���
    QAtomicInt successCount; // �ɹ�����
    QAtomicInt errorCount; // ʧ�ܼ���
    QAtomicInt cancelRequested; // ȡ����־
    QAtomicInt pauseRequested;  // ��ͣ��־
    QMutex logMutex; // ��־������
    int currentQuality; // ��ǰ��������
    QWaitCondition pauseCondition; // ��ͣ��������
    QMutex pauseMutex;          // ��ͣ������
    bool isRunning = false;     // �����Ƿ�������
    // �����־��س�Ա
    QFile* logFile;           //��־�ļ�����
    QTextStream* logStream;   //��־��
    QDate currentLogDate;     //��ǰ��־�ļ�������
    QMutex logFileMutex;      //��־�ļ�������

    QList<QWidget*> protectedWidgets; // ʹ��QWidget�������洢��ͬ���Ϳؼ�
    bool locked;
    QTimer* lockTimer;


private slots: //��Ӧ���ܲۺ���
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
    void saveSettings();  // ��������
    void cancelProcessing(); // ȡ������
    void updateProgress(); // ���½���
    void onProcessingFinished(); //�������
    void toggleRunState(); // �л�����״̬
    void pauseProcessing(); // ��ͣ����
    void resumeProcessing(); // �ָ�����
    void logMessage(const QString& message);
    // �����־����
    void initLogFile(); //��ʼ����־�ļ�
    void rotateLogFile(); //�ֻ���־�ļ������죩
    void writeToLogFile(const char* message); // д����־���ļ�
    void onLockButtonClicked();
    void lockControls(bool lock);
    void resetTimer();

public slots:
    void processFile(const QString& filePath); // �������ļ��Ĳۺ���

signals:
    void fileProcessed(bool success);
};
