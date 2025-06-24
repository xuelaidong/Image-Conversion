#include "QtWidgetsApplication2.h"
#include <QFileInfo>
#include <QDateTime>
#include <iostream>
#include <QProgressBar>
#include <QDirIterator>
#include <QFileDialog>
#include <QTimer>
#include <QObject>
#include <QDebug>
#include <QMessageBox>
#include <QSaveFile>
#include <QThread>
#include <QRunnable>
#include <QImageReader>
#include <QImageWriter>
#include <QTextCodec>
#include <QInputDialog>
#include "imageprocessor.h"
//#include <QElapsedTimer>

// �ļ�����������
class ImageProcessingTask : public QRunnable
{
public:
    ImageProcessingTask(QtWidgetsApplication2* parent, const QString& filePath, int quality)
        : parent(parent), filePath(filePath), quality(quality) {}

    void run() override {
        parent->processFile(filePath);
    }
private:
    QtWidgetsApplication2* parent;
    QString filePath;
    int quality;
};

QtWidgetsApplication2::QtWidgetsApplication2(QWidget *parent)
    : QMainWindow(parent), settings(new QSettings("MyCompany", "PNGConverter", this)),
    processedCount(0), successCount(0), errorCount(0), totalFiles(0), cancelRequested(false),
    logFile(nullptr), logStream(nullptr), // ��ʼ����־����
    locked(true)
{
    ui.setupUi(this);

    //���عرմ��ڰ�ť
    setWindowFlags(Qt::CustomizeWindowHint |
                    Qt::WindowMinimizeButtonHint |
                    Qt::WindowMaximizeButtonHint
    );
    //�����������߳���
    int threads = settings->value("thread", 1).toInt();
    ui.spinBox_3->setValue(threads);
    //��ʼ���߳�
    int maxThreads = QThread::idealThreadCount() - 1; // ����һ���̸߳�UI
    if (maxThreads < 1) maxThreads = 1;
    if (maxThreads > 2) maxThreads = threads; // ��������߳���
    threadPool.setMaxThreadCount(maxThreads);

    //�����ü��ظ��ļ���·��
    rootFolderPath = settings->value("rootFolderPath",QDir::homePath()).toString();
    ui.label->setText(rootFolderPath);

    //�������м����Զ�����ʱ��
    selectedTime = QTime::fromString((settings->value("time", "08:00").toString()), "HH:mm");
    ui.dateTimeEdit->setTime(selectedTime);

    // ����Ĭ������
    int defaultQuality = settings->value("quality", 50).toInt();
    ui.horizontalSlider->setValue(defaultQuality);
    ui.label_2->setText(QString(u8"������%1%").arg(defaultQuality));
    currentQuality = defaultQuality;

    // ������������
    connect(ui.horizontalSlider, &QSlider::valueChanged, [this](int value) {
        ui.label_2->setText(QString(u8"������%1%").arg(value));
        currentQuality = value;
        });

    // ���������ö�ʱ��
    dailyTimer = new QTimer(this);
    connect(dailyTimer, &QTimer::timeout, this, &QtWidgetsApplication2::onDailyTimer);
    // ���ö�ʱ����ÿ���Ӽ��һ��
    dailyTimer->start(30*1000);
    // ��������
    logMessage(u8"����������ÿ�찴������ʱ����Զ����� 1��ǰ �ļ���");
    writeToLogFile("����������ÿ�찴������ʱ����Զ����� 1��ǰ �ļ���");

    //�����ļ���������ź�
    connect(this, &QtWidgetsApplication2::fileProcessed, this, [this](bool success) {
        if (success) {
            successCount.fetchAndAddRelaxed(1);
        }
        else {
            errorCount.fetchAndAddRelaxed(1);
        }
        processedCount.fetchAndAddRelaxed(1);
        });

    //�ֶ�ִ��
    connect(ui.pushButton, &QPushButton::clicked, this, &QtWidgetsApplication2::manualOperation);
    // ѡ����ļ���ѡ��ť
    connect(ui.pushButton_2, &QPushButton::clicked, this, &QtWidgetsApplication2::onRootFolderSelected);
    // ȡ������ť
    connect(ui.pushButton_5, &QPushButton::clicked, this, &QtWidgetsApplication2::cancelProcessing);
    // ����/ֹͣ
    connect(ui.pushButton_6, &QPushButton::clicked, this, &QtWidgetsApplication2::toggleRunState);
    // ��ͣ
    connect(ui.pushButton_3, &QPushButton::clicked, this, &QtWidgetsApplication2::pauseProcessing);
    // �ָ�
    connect(ui.pushButton_4, &QPushButton::clicked, this, &QtWidgetsApplication2::resumeProcessing);

    // ����/����
    connect(ui.pushButton_7, &QPushButton::clicked, this, &QtWidgetsApplication2::onLockButtonClicked);
    // ��������Ҫ�����Ŀؼ����뵽�б���
    protectedWidgets << ui.pushButton
        << ui.pushButton_2
        << ui.pushButton_3
        << ui.pushButton_4
        << ui.pushButton_5
        << ui.pushButton_6
        << ui.dateTimeEdit
        << ui.horizontalSlider
        << ui.checkBox
        << ui.checkBox_2
        << ui.checkBox_3
        << ui.checkBox_4
        << ui.spinBox
        << ui.spinBox_2
        << ui.spinBox_3
        <<ui.comboBox;
    // ���ö�ʱ��
    lockTimer = new QTimer(this);
    connect(lockTimer, &QTimer::timeout, this, [this]() {QtWidgetsApplication2::lockControls(true); });
    resetTimer();

    // ����ÿ���ؼ����ź������ö�ʱ��
    for (auto widget : protectedWidgets) {
        if (auto button = qobject_cast<QPushButton*>(widget)) {
            connect(button, &QPushButton::clicked, this, &QtWidgetsApplication2::resetTimer);
        }
        else if (auto checkBox = qobject_cast<QCheckBox*>(widget)) {
            connect(checkBox, &QCheckBox::stateChanged, this, &QtWidgetsApplication2::resetTimer);
        }
        else if (auto dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
            connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &QtWidgetsApplication2::resetTimer);
        }
        else if (auto slider = qobject_cast<QSlider*>(widget)) {
            connect(slider, &QSlider::valueChanged, this, &QtWidgetsApplication2::resetTimer);
        }
        else if (auto spinBox = qobject_cast<QSpinBox*>(widget)){
            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),this, &QtWidgetsApplication2::resetTimer);
        }
    }

    //// ��ʼ����ť״̬
    QtWidgetsApplication2::toggleRunState();
    // ��ť������
    ui.pushButton_3->setEnabled(false); // ��ͣ��ť
    ui.pushButton_4->setEnabled(false); // ������ť
    ui.pushButton_5->setEnabled(false); // ȡ����ť
    ui.pushButton->setEnabled(false); // ѡ���ļ�·����ť

    ui.checkBox->setChecked(settings->value("checkBox1State", false).toBool());
    ui.checkBox_2->setChecked(settings->value("checkBox2State", false).toBool());
    ui.checkBox_3->setChecked(settings->value("checkBox3State", false).toBool());
    ui.checkBox_4->setChecked(settings->value("checkBox4State", false).toBool());
    //�����������Զ�ת�漸��ǰ�ļ���
    int saveDays = settings->value("saveDay", 1).toInt();
    ui.spinBox->setValue(saveDays);
    //�����������Զ�ɾ������ǰ�ļ���
    int deleteDays = settings->value("deleteDay", 60).toInt();
    ui.spinBox_2->setValue(deleteDays);
    
    ui.comboBox->setCurrentIndex(settings->value("process", 0).toInt());

    // ������־��ʾ�������
    ui.textEdit->document()->setMaximumBlockCount(1000);

    // ��ʼ����־ϵͳ
    initLogFile();
}

QtWidgetsApplication2::~QtWidgetsApplication2()
{
    cancelRequested = true;
    threadPool.clear();
    threadPool.waitForDone(); // �ȴ������������
    saveSettings(); // ��������
    // ������־��Դ
    if (logStream) {
        logStream->flush();
        delete logStream;
        logStream = nullptr;
    }
    if (logFile) {
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }
    logMessage(u8"Ӧ�ó���ر�"); // ���һ����־
    writeToLogFile("Ӧ�ó���ر�");
}
// �ֶ�ִ��
void QtWidgetsApplication2::manualOperation()
{
    // ѡ���ļ���
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        u8"ѡ��Ҫ������ļ���",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (folderPath.isEmpty()) {
        logMessage(u8"������ȡ��");
        writeToLogFile("������ȡ��");
        return;
    }
    else
    {
        convertFolder(folderPath);
    }
}

void QtWidgetsApplication2::onRootFolderSelected()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        u8"ѡ����ļ��У����������ļ��У�",
        rootFolderPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!folder.isEmpty()) {
        rootFolderPath = folder;
        ui.label->setText(rootFolderPath);
        logMessage(QString(u8"���ļ���������Ϊ: %1").arg(rootFolderPath));
    }
    saveSettings(); // ������������
}

void QtWidgetsApplication2::onDailyTimer()
{
    if (!isRunning) return; // �������δ��������ִ��

    QTime currentTime = QTime::currentTime();
    // ����Ƿ�������8��
    QTime timePart = ui.dateTimeEdit->dateTime().time();
    if (currentTime.hour() == timePart.hour() && currentTime.minute() == timePart.minute()) {
        QDate today = QDate::currentDate();
        //���������ҹ�������ڼ������
        int saveDays = -ui.spinBox->value();
        int deleteDays = -ui.spinBox_2->value();
        QDate yesterday = QDate::currentDate().addDays(saveDays);
        QDate cutoffDate  = QDate::currentDate().addDays(deleteDays);

        logMessage(u8"��ʱ��㣬��ʼ�Զ�ִ��---");
        writeToLogFile("��ʱ��㣬��ʼ�Զ�ִ��---");
        
        // ����Ƿ��Ѿ������������ļ���
        if (yesterday != lastProcessedDate) {
            QStringList dateFormats = {
                "yyyy-MM-dd",      // 2025-06-02
                u8"yyyy��M��d��",    // 2025��6��2��
                u8"yyyy��M��d��",    // 2025��6��2��
                u8"yyyy��MM��dd��",    // 2025��6��2��
                u8"yyyy��MM��dd��",    // 2025��6��2��
                "yyyyMMdd",        // 20250602
                "yyyy_MM_dd",      // 2025_06_02
                "dd-MM-yyyy",      // 02-06-2025
                "MM-dd-yyyy"       // 06-02-2025
            };
            
            QDate folderDate;
            QString folderDateName;
            bool validDateExits = false;

            // ��ȡ��Ŀ¼�µ������ļ���
            QStringList folders = QDir(rootFolderPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            int deletedCount = 0;

            foreach(const QString & folderName, folders) {
                QDate folderDate;
                bool validDate = false;

                // ���Խ����ļ��������е�����
                for (const QString& format : dateFormats) {
                    folderDate = QDate::fromString(folderName, format);
                    if (folderDate.isValid()) {
                        validDate = true;
                        break;
                    }
                }

                // ��������Ƿ���Ч�����ڽ�ֹ����
                if (validDate && folderDate < cutoffDate) {
                    QString folderPath = QDir(rootFolderPath).filePath(folderName);
                    QDir dir(folderPath);

                    if (dir.removeRecursively()) {
                        logMessage(QString(u8"��ɾ�����ļ���: %1").arg(folderName));
                        deletedCount++;
                    }
                    else {
                        logMessage(QString(u8"ɾ��ʧ��: %1").arg(folderName));
                    }
                }
            }

            logMessage(QString(u8"�ļ���������ɣ���ɾ�� %1 ���ļ���").arg(deletedCount));

            QString foundFolderPath;
            bool folderExists = false;
            // �������п��ܵ�����
            for (const QString& format : dateFormats)
            {
                QString folderName = yesterday.toString(format);
                QString folderPath = QDir(rootFolderPath).filePath(folderName);
                QDir dir(folderPath);
                if (dir.exists() && !dir.isEmpty())
                {
                    foundFolderPath = folderPath;
                    folderExists = true;
                    break; // �ҵ���Ч���ļ��к�ֹͣ����
                }
            }

            if (folderExists)
            {
                logMessage(QString(u8"��⵽%1�㣬�Զ������ļ���: %2")
                    .arg(timePart.toString("HH:mm"))
                    .arg(foundFolderPath));
                writeToLogFile("��⵽%1�㣬�Զ������ļ���");
                // ����������ļ���
                convertFolder(foundFolderPath);

                // �������������
                lastProcessedDate = yesterday;
                logMessage(QString(u8"�Ѹ������������: %1")
                    .arg(lastProcessedDate.toString("yyyy-MM-dd")));
                writeToLogFile("�Ѹ������������");
            }
            else
            {
                logMessage(QString(u8"δ�ҵ�������ļ���: %1")
                    .arg(yesterday.toString("yyyy-MM-dd")));
            }
        }
        else {
            logMessage(u8"�ļ����Ѵ����������");
            writeToLogFile("�ļ����Ѵ����������");
        }
    }
}

void QtWidgetsApplication2::convertFolder(const QString& folderPath)
{
    if (!isRunning) return; // �������δ��������ִ��
    if (!QDir(folderPath).exists()) {
        logMessage(QString(u8"�ļ��в�����:1%").arg(folderPath));
        writeToLogFile("�ļ��в�����");
        return;
    }
    logMessage(QString(u8"��ʼ�����ļ���: %1").arg(folderPath));
    writeToLogFile("��ʼ�����ļ���");

    // ����״̬
    processedCount = 0;
    successCount = 0;
    errorCount = 0;
    cancelRequested = false;
    ui.progressBar->setValue(0);

    // ����ͼƬ�ļ�
    QStringList filters;
    if (ui.checkBox_2->isChecked()) {
        filters << "*.bmp";
        logMessage(u8"�ɴ���.bmpͼƬ");
    }
    if (ui.checkBox_3->isChecked()) {
        filters << "*.tif";
        logMessage(u8"�ɴ���.tifͼƬ");
    }
    if (ui.checkBox->isChecked()) {
        filters << "*.png";
        logMessage(u8"�ɴ���.pngͼƬ");
    }
    if (ui.checkBox_4->isChecked()) {
        filters << "*.jpg";
        logMessage(u8"�ɴ���.jpgͼƬ");
    }
    QDirIterator countIt(folderPath, filters, QDir::Files, QDirIterator::Subdirectories);
    QStringList fileList;
    // �ȼ����ļ�����
    while (countIt.hasNext()) {
        fileList.append(countIt.next());
    }
    totalFiles = fileList.size();
    if (totalFiles == 0) {
        logMessage(u8"û���ҵ���ӦͼƬ�ļ�");
        writeToLogFile("û���ҵ���ӦͼƬ�ļ�");
        return;
    }
    writeToLogFile("�ҵ���ӦͼƬ�ļ�");
    logMessage(QString(u8"�ҵ� %1 ����ӦͼƬ�ļ�").arg(totalFiles));

    // ���ý�������Χ
    ui.progressBar->setRange(0, 100);
    // �ύ�����̳߳�
    for (const QString& filePath : fileList) {
        if (cancelRequested) {
            break;
        }
        // ���������ύ���̳߳�
        ImageProcessingTask* task = new ImageProcessingTask(this, filePath, currentQuality);
        task->setAutoDelete(true);
        threadPool.start(task);
        // ÿ�ύ100�����񣬶��������Կ��������ύ�ٶ�
        if (threadPool.activeThreadCount() >= threadPool.maxThreadCount() * 2) {
            QThread::sleep(100);
        }
    }

    //�������ȸ��¶�ʱ��
    QTimer* progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &QtWidgetsApplication2::updateProgress);
    progressTimer->start(1000); // ÿ1s����һ�ν���

    // ������ɼ�鶨ʱ��
    QTimer* finishTimer = new QTimer(this);
    connect(finishTimer, &QTimer::timeout, this, [=]() {
        if (processedCount.loadAcquire() >= totalFiles.loadAcquire() || cancelRequested) {
            finishTimer->deleteLater();
            progressTimer->deleteLater();
            onProcessingFinished();
        }
        });
    finishTimer->start(1000); //ÿ1S���һ��

    ui.statusBar->showMessage(u8"���ڴ���ͼƬ...");
    logMessage(u8"���ڴ���ͼƬ...");
    writeToLogFile("���ڴ���ͼƬ...");
    //���ⲻ��Ҫ����ѯ�����Ч��
    if (processedCount.loadAcquire() == totalFiles.loadAcquire()) {
        QMetaObject::invokeMethod(this, &QtWidgetsApplication2::onProcessingFinished, Qt::QueuedConnection);
    }
}

void QtWidgetsApplication2::saveSettings()
{
    settings->setValue("rootFolderPath", rootFolderPath);
    settings->setValue("quality", ui.horizontalSlider->value());
    // ����ʱ��Ϊ�ַ���
    settings->setValue("time", ui.dateTimeEdit->time().toString("HH:mm"));
    settings->setValue("checkBox1State", ui.checkBox->isChecked());
    settings->setValue("checkBox2State", ui.checkBox_2->isChecked());
    settings->setValue("checkBox3State", ui.checkBox_3->isChecked());
    settings->setValue("checkBox4State", ui.checkBox_4->isChecked());
    settings->setValue("saveDay", ui.spinBox->value());
    settings->setValue("deleteDay", ui.spinBox_2->value());
    settings->setValue("thread", ui.spinBox_3->value());
    settings->setValue("process", ui.comboBox->currentIndex());
    settings->sync(); //ȷ��д�����
}
// �������ļ����̰߳�ȫ��
void QtWidgetsApplication2::processFile(const QString& filePath)
{
    //QElapsedTimer timer;
    //timer.start();  //��ʼ��ʱ

    if (cancelRequested) {
        return;
    }

    bool success = false;
    // �����ͣ״̬
    if (pauseRequested) {
        QMutexLocker locker(&pauseMutex);
        while (pauseRequested && !cancelRequested) {
            pauseCondition.wait(&pauseMutex, 100); // �ȴ�100����
        }
        if (cancelRequested) return;
    }

    switch (ui.comboBox->currentIndex()){
        
    case 0:     //LY5/6����-����
        success = ImageProcessor::processFile_0(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
        
    case 1:     //LY4����-����
        success = ImageProcessor::processFile_1(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
        
    case 2:     //LY4����-Բ��
        success = ImageProcessor::processFile_2(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
        
    case 3:     //LY4�ܷⶤ-Բ��
        success = ImageProcessor::processFile_3(
        filePath,
        currentQuality,
        [this](const QString& msg) {this->logMessage(msg); },
        [this](const char* msg) {this->writeToLogFile(msg); }
          ); 
        break;
        
    case 4:     //LY1&2-XRay
        success = ImageProcessor::processFile_4(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
        
    case 5:     //LY3-XRay
        success = ImageProcessor::processFile_5(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
        
    case 6:     //LY5/6-XRay
        success = ImageProcessor::processFile_6(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;

    case 7:     //����webp
        success = ImageProcessor::processFile_webp(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
    case 8:     //����jpg
        success = ImageProcessor::processFile_jpg(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
    case 9:     //����png
        success = ImageProcessor::processFile_png(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
    case 10:     //����bmp
        success = ImageProcessor::processFile_bmp(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
    default:
        break;
    }

    emit fileProcessed(success);
    
    // ������ÿ���ļ�֮����0.5s������CPUʹ����
    QThread::msleep(500);
}
// ȡ������
void QtWidgetsApplication2::cancelProcessing()
{
    if (!isRunning) return;
    cancelRequested = true;
    threadPool.clear(); // �������δ��ʼ������
    writeToLogFile("������ȡ��"); 
    logMessage(u8"������ȡ��");
    ui.statusBar->showMessage(u8"������ȡ��");
}
// ���½���
void QtWidgetsApplication2::updateProgress()
{
    int processed = processedCount.loadAcquire();
    int total = totalFiles.loadAcquire();

    if (total > 0) {
        int percentage = static_cast<int>((static_cast<double>(processed) / total) * 100);
        ui.progressBar->setValue(percentage);
        ui.statusBar->showMessage(
            QString(u8"������: %1/%2 (%3%)")
            .arg(processed)
            .arg(total)
            .arg(percentage)
        );
    }
}
// �������
void QtWidgetsApplication2::onProcessingFinished()
{
    int processed = processedCount.loadAcquire();
    int success = successCount.loadAcquire();
    int error = errorCount.loadAcquire();
    int total = totalFiles.loadAcquire();

    if (cancelRequested) {
        writeToLogFile("������ȡ��");
        logMessage(u8"������ȡ��");
        ui.statusBar->showMessage(u8"������ȡ��");
    }
    else {
        ui.statusBar->showMessage(QString(u8"�������! �ɹ�: %1, ʧ��: %2").arg(success).arg(error));
        writeToLogFile("�������");
        logMessage(QString(u8"�������! �ɹ�: %1, ʧ��: %2").arg(success).arg(error));
    }
    threadPool.clear(); // ���δ��ʼ������
    cancelRequested = false; // ����ȡ����־
}

// ����/ֹͣ
void QtWidgetsApplication2::toggleRunState()
{
    isRunning = !isRunning;

    if (isRunning) {
        // ��������
        ui.pushButton_6->setStyleSheet("background-color: green; color: white;");
        dailyTimer->start(); // ȷ����ʱ������

        // ������ͣ��ť�����ûָ���ť
        ui.pushButton_3->setEnabled(true);
        ui.pushButton_4->setEnabled(false);

        ui.pushButton->setEnabled(true); // 
        ui.pushButton_5->setEnabled(true); // 
        ui.pushButton->setEnabled(true); // 

        // �����־��¼
        logMessage(u8"Ӧ�ó�������");
        logMessage(QString(u8"���ļ���: %1").arg(rootFolderPath));
        logMessage(QString(u8"�Զ�����ʱ��: %1").arg(selectedTime.toString("HH:mm")));
        logMessage(QString(u8"ͼƬ����: %1%").arg(ui.horizontalSlider->value()));
        writeToLogFile("Ӧ�ó�������");
    }
    else {
        // ֹͣ����
        ui.pushButton_6->setStyleSheet("background-color: red; color: white;");
        dailyTimer->stop(); // ֹͣ��ʱ��
        cancelProcessing(); // ȡ����ǰ����

        // ��ť������
        ui.pushButton_3->setEnabled(false); // ��ͣ��ť
        ui.pushButton_4->setEnabled(false); // ������ť
        ui.pushButton_5->setEnabled(false); // ȡ����ť
        ui.pushButton->setEnabled(false); // ѡ���ļ�·����ť

        logMessage(u8"������ֹͣ");
        writeToLogFile("������ֹͣ");
    }
}
// ��ͣ
void QtWidgetsApplication2::pauseProcessing()
{
    if (!isRunning) return;

    pauseRequested = true;
    ui.pushButton_3->setEnabled(false);
    ui.pushButton_4->setEnabled(true);
    logMessage(u8"��������ͣ");
    writeToLogFile("��������ͣ");
}
// �ָ�
void QtWidgetsApplication2::resumeProcessing()
{
    if (!isRunning) return;

    pauseRequested = false;
    // �������еȴ����߳�
    pauseCondition.wakeAll();

    ui.pushButton_3->setEnabled(true);
    ui.pushButton_4->setEnabled(false);
    logMessage(u8"�����ѻָ�");
    writeToLogFile("�����ѻָ�");
}

// �����̰߳�ȫ����־����
void QtWidgetsApplication2::logMessage(const QString& message)
{
    // ��������־ϵͳ�����м�¼��־����ֹ�ݹ飩
    static bool inLog = false;
    if (inLog) return;
    inLog = true;
    
    //��UI����ʾ��־
    QMutexLocker locker(&logMutex);
    QMetaObject::invokeMethod(ui.textEdit, "append",
        Qt::QueuedConnection, Q_ARG(QString, QDateTime::currentDateTime().toString("[hh:mm:ss] ") + message));

    inLog = false;
}

// ��ʼ����־�ļ�
void QtWidgetsApplication2::initLogFile()
{
    // ������־Ŀ¼
    QDir logDir("logs");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }

    // ���õ�ǰ��־����Ϊ����
    currentLogDate = QDate::currentDate();

    // ������־�ļ���
    QString logFileName = QString("logs/%1.log").arg(currentLogDate.toString("yyyy-MM-dd"));

    // ����־�ļ�
    logFile = new QFile(logFileName);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        writeToLogFile("�޷�����־�ļ�");
        qWarning() << u8"�޷�����־�ļ�:" << logFileName;
        delete logFile;
        logFile = nullptr;
        return;
    }

    // ������־��
    logStream = new QTextStream(logFile);
    logStream->setCodec("ISO 8859-1");

    // д����־ͷ
    *logStream << "========================================\n";
    *logStream << "Ӧ�ó�������: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    *logStream << "�汾: 1.0\n";
    *logStream << "��־����: INFO\n";
    *logStream << "========================================\n\n";
    logStream->flush();
}

// �ֻ���־�ļ������죩
void QtWidgetsApplication2::rotateLogFile()
{
    //QMutexLocker locker(&logFileMutex);

    QDate today = QDate::currentDate();
    if (today == currentLogDate) {
        return; // ����Ҫ�ֻ�
    }

    // �رյ�ǰ��־�ļ�
    if (logStream) {
        logStream->flush();
        delete logStream;
        logStream = nullptr;
    }

    if (logFile) {
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }

    // ������־����
    currentLogDate = today;

    // �����µ���־�ļ�
    QString logFileName = QString("logs/%1.log").arg(currentLogDate.toString("yyyy-MM-dd"));

    logFile = new QFile(logFileName);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "�޷�����־�ļ�:" << logFileName;
        delete logFile;
        logFile = nullptr;
        return;
    }

    logStream = new QTextStream(logFile);
    logStream->setCodec("ISO 8859-1");

    // д������־�ļ���ͷ
    *logStream << "========================================\n";
    *logStream << "Ӧ�ó�����־ - �µ�һ��: " << currentLogDate.toString("yyyy-MM-dd") << "\n";
    *logStream << "��־�ֻ�ʱ��: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    *logStream << "========================================\n\n";
    logStream->flush();
}

// д����־���ļ�
void QtWidgetsApplication2::writeToLogFile(const char* message)
{
    QMutexLocker locker(&logFileMutex);

    // ����Ƿ���Ҫ�ֻ���־
    if (QDate::currentDate() != currentLogDate) {
        rotateLogFile();
    }
    // д����־
    if (logStream && logFile) {
        QString timestamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]");
        *logStream << timestamp << " " << message << "\n";
        logStream->flush();
    }
}

// ����/����
void QtWidgetsApplication2::onLockButtonClicked()
{
    if (locked) {
        // �������������Ҫ�������������
        bool ok;
        QString password = QInputDialog::getText(this, "Password Required",
            "Enter Password:", QLineEdit::Password, "", &ok);
        if (ok && password == "123") { // �򵥵�������
            lockControls(false);
        }
        else {
            QMessageBox::warning(this, "Error", "Incorrect password.");
        }
    }
    else {
        // ���δ��������ֱ������
        lockControls(true);
    }
}

void QtWidgetsApplication2::resetTimer()
{
    lockTimer->start(3 * 60 * 1000); // ���¿�ʼ��ʱ����λΪ����
}
void QtWidgetsApplication2::lockControls(bool lock = true)
{
    locked = lock;
    for (auto widget : protectedWidgets) {
        widget->setEnabled(!locked);
    }
}
// �̱߳仯�Զ�����
void QtWidgetsApplication2::spinBoxTextChanged()
{
    //�����������߳���
    int threads = ui.spinBox_3->value();
    //��ʼ���߳�
    int maxThreads = QThread::idealThreadCount() - 1; // ����һ���̸߳�UI
    if (maxThreads < 1) maxThreads = 1;
    if (maxThreads > 2) maxThreads = threads; // ��������߳���
    threadPool.setMaxThreadCount(maxThreads);

    logMessage(QString(u8"ʹ��%1���̴߳���ͼƬ").arg(maxThreads));
}

// ��Ӱ�ť�����������ֹ��̨����
void QtWidgetsApplication2::pushbuttonClicked()
{}
void QtWidgetsApplication2::pushbuttonClicked2()
{}
void QtWidgetsApplication2::pushbuttonClicked3()
{}
void QtWidgetsApplication2::pushbuttonClicked4()
{}
void QtWidgetsApplication2::pushbuttonClicked5()
{}
void QtWidgetsApplication2::pushbuttonClicked6()
{}
void QtWidgetsApplication2::pushbuttonClicked7()
{}