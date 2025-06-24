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

// 文件处理任务类
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
    logFile(nullptr), logStream(nullptr), // 初始化日志对象
    locked(true)
{
    ui.setupUi(this);

    //隐藏关闭窗口按钮
    setWindowFlags(Qt::CustomizeWindowHint |
                    Qt::WindowMinimizeButtonHint |
                    Qt::WindowMaximizeButtonHint
    );
    //加载设置中线程数
    int threads = settings->value("thread", 1).toInt();
    ui.spinBox_3->setValue(threads);
    //初始化线程
    int maxThreads = QThread::idealThreadCount() - 1; // 保留一个线程给UI
    if (maxThreads < 1) maxThreads = 1;
    if (maxThreads > 2) maxThreads = threads; // 限制最大线程数
    threadPool.setMaxThreadCount(maxThreads);

    //从设置加载根文件夹路径
    rootFolderPath = settings->value("rootFolderPath",QDir::homePath()).toString();
    ui.label->setText(rootFolderPath);

    //从设置中加载自动处理时间
    selectedTime = QTime::fromString((settings->value("time", "08:00").toString()), "HH:mm");
    ui.dateTimeEdit->setTime(selectedTime);

    // 设置默认质量
    int defaultQuality = settings->value("quality", 50).toInt();
    ui.horizontalSlider->setValue(defaultQuality);
    ui.label_2->setText(QString(u8"质量：%1%").arg(defaultQuality));
    currentQuality = defaultQuality;

    // 连接质量滑块
    connect(ui.horizontalSlider, &QSlider::valueChanged, [this](int value) {
        ui.label_2->setText(QString(u8"质量：%1%").arg(value));
        currentQuality = value;
        });

    // 创建并配置定时器
    dailyTimer = new QTimer(this);
    connect(dailyTimer, &QTimer::timeout, this, &QtWidgetsApplication2::onDailyTimer);
    // 设置定时器，每分钟检查一次
    dailyTimer->start(30*1000);
    // 加载设置
    logMessage(u8"请启动程序，每天按照设置时间点自动处理 1天前 文件夹");
    writeToLogFile("请启动程序，每天按照设置时间点自动处理 1天前 文件夹");

    //连接文件处理完成信号
    connect(this, &QtWidgetsApplication2::fileProcessed, this, [this](bool success) {
        if (success) {
            successCount.fetchAndAddRelaxed(1);
        }
        else {
            errorCount.fetchAndAddRelaxed(1);
        }
        processedCount.fetchAndAddRelaxed(1);
        });

    //手动执行
    connect(ui.pushButton, &QPushButton::clicked, this, &QtWidgetsApplication2::manualOperation);
    // 选择根文件夹选择按钮
    connect(ui.pushButton_2, &QPushButton::clicked, this, &QtWidgetsApplication2::onRootFolderSelected);
    // 取消处理按钮
    connect(ui.pushButton_5, &QPushButton::clicked, this, &QtWidgetsApplication2::cancelProcessing);
    // 启动/停止
    connect(ui.pushButton_6, &QPushButton::clicked, this, &QtWidgetsApplication2::toggleRunState);
    // 暂停
    connect(ui.pushButton_3, &QPushButton::clicked, this, &QtWidgetsApplication2::pauseProcessing);
    // 恢复
    connect(ui.pushButton_4, &QPushButton::clicked, this, &QtWidgetsApplication2::resumeProcessing);

    // 锁定/解锁
    connect(ui.pushButton_7, &QPushButton::clicked, this, &QtWidgetsApplication2::onLockButtonClicked);
    // 将所有需要保护的控件加入到列表中
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
    // 设置定时器
    lockTimer = new QTimer(this);
    connect(lockTimer, &QTimer::timeout, this, [this]() {QtWidgetsApplication2::lockControls(true); });
    resetTimer();

    // 连接每个控件的信号以重置定时器
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

    //// 初始化按钮状态
    QtWidgetsApplication2::toggleRunState();
    // 按钮不可用
    ui.pushButton_3->setEnabled(false); // 暂停按钮
    ui.pushButton_4->setEnabled(false); // 启动按钮
    ui.pushButton_5->setEnabled(false); // 取消按钮
    ui.pushButton->setEnabled(false); // 选择文件路径按钮

    ui.checkBox->setChecked(settings->value("checkBox1State", false).toBool());
    ui.checkBox_2->setChecked(settings->value("checkBox2State", false).toBool());
    ui.checkBox_3->setChecked(settings->value("checkBox3State", false).toBool());
    ui.checkBox_4->setChecked(settings->value("checkBox4State", false).toBool());
    //加载设置中自动转存几天前文件夹
    int saveDays = settings->value("saveDay", 1).toInt();
    ui.spinBox->setValue(saveDays);
    //加载设置中自动删除几天前文件夹
    int deleteDays = settings->value("deleteDay", 60).toInt();
    ui.spinBox_2->setValue(deleteDays);
    
    ui.comboBox->setCurrentIndex(settings->value("process", 0).toInt());

    // 设置日志显示最大行数
    ui.textEdit->document()->setMaximumBlockCount(1000);

    // 初始化日志系统
    initLogFile();
}

QtWidgetsApplication2::~QtWidgetsApplication2()
{
    cancelRequested = true;
    threadPool.clear();
    threadPool.waitForDone(); // 等待所有任务完成
    saveSettings(); // 保存设置
    // 清理日志资源
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
    logMessage(u8"应用程序关闭"); // 最后一条日志
    writeToLogFile("应用程序关闭");
}
// 手动执行
void QtWidgetsApplication2::manualOperation()
{
    // 选择文件夹
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        u8"选择要处理的文件夹",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (folderPath.isEmpty()) {
        logMessage(u8"操作已取消");
        writeToLogFile("操作已取消");
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
        u8"选择根文件夹（包含日期文件夹）",
        rootFolderPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!folder.isEmpty()) {
        rootFolderPath = folder;
        ui.label->setText(rootFolderPath);
        logMessage(QString(u8"根文件夹已设置为: %1").arg(rootFolderPath));
    }
    saveSettings(); // 立即保存设置
}

void QtWidgetsApplication2::onDailyTimer()
{
    if (!isRunning) return; // 如果程序未启动，不执行

    QTime currentTime = QTime::currentTime();
    // 检查是否是早上8点
    QTime timePart = ui.dateTimeEdit->dateTime().time();
    if (currentTime.hour() == timePart.hour() && currentTime.minute() == timePart.minute()) {
        QDate today = QDate::currentDate();
        //避免因跨午夜导致日期计算错误
        int saveDays = -ui.spinBox->value();
        int deleteDays = -ui.spinBox_2->value();
        QDate yesterday = QDate::currentDate().addDays(saveDays);
        QDate cutoffDate  = QDate::currentDate().addDays(deleteDays);

        logMessage(u8"到时间点，开始自动执行---");
        writeToLogFile("到时间点，开始自动执行---");
        
        // 检查是否已经处理过昨天的文件夹
        if (yesterday != lastProcessedDate) {
            QStringList dateFormats = {
                "yyyy-MM-dd",      // 2025-06-02
                u8"yyyy年M月d日",    // 2025年6月2日
                u8"yyyy年M月d号",    // 2025年6月2号
                u8"yyyy年MM月dd日",    // 2025年6月2日
                u8"yyyy年MM月dd号",    // 2025年6月2号
                "yyyyMMdd",        // 20250602
                "yyyy_MM_dd",      // 2025_06_02
                "dd-MM-yyyy",      // 02-06-2025
                "MM-dd-yyyy"       // 06-02-2025
            };
            
            QDate folderDate;
            QString folderDateName;
            bool validDateExits = false;

            // 获取根目录下的所有文件夹
            QStringList folders = QDir(rootFolderPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            int deletedCount = 0;

            foreach(const QString & folderName, folders) {
                QDate folderDate;
                bool validDate = false;

                // 尝试解析文件夹名称中的日期
                for (const QString& format : dateFormats) {
                    folderDate = QDate::fromString(folderName, format);
                    if (folderDate.isValid()) {
                        validDate = true;
                        break;
                    }
                }

                // 检查日期是否有效且早于截止日期
                if (validDate && folderDate < cutoffDate) {
                    QString folderPath = QDir(rootFolderPath).filePath(folderName);
                    QDir dir(folderPath);

                    if (dir.removeRecursively()) {
                        logMessage(QString(u8"已删除旧文件夹: %1").arg(folderName));
                        deletedCount++;
                    }
                    else {
                        logMessage(QString(u8"删除失败: %1").arg(folderName));
                    }
                }
            }

            logMessage(QString(u8"文件夹清理完成，共删除 %1 个文件夹").arg(deletedCount));

            QString foundFolderPath;
            bool folderExists = false;
            // 尝试所有可能的日期
            for (const QString& format : dateFormats)
            {
                QString folderName = yesterday.toString(format);
                QString folderPath = QDir(rootFolderPath).filePath(folderName);
                QDir dir(folderPath);
                if (dir.exists() && !dir.isEmpty())
                {
                    foundFolderPath = folderPath;
                    folderExists = true;
                    break; // 找到有效的文件夹后停止搜索
                }
            }

            if (folderExists)
            {
                logMessage(QString(u8"检测到%1点，自动处理文件夹: %2")
                    .arg(timePart.toString("HH:mm"))
                    .arg(foundFolderPath));
                writeToLogFile("检测到%1点，自动处理文件夹");
                // 处理昨天的文件夹
                convertFolder(foundFolderPath);

                // 更新最后处理日期
                lastProcessedDate = yesterday;
                logMessage(QString(u8"已更新最后处理日期: %1")
                    .arg(lastProcessedDate.toString("yyyy-MM-dd")));
                writeToLogFile("已更新最后处理日期");
            }
            else
            {
                logMessage(QString(u8"未找到昨天的文件夹: %1")
                    .arg(yesterday.toString("yyyy-MM-dd")));
            }
        }
        else {
            logMessage(u8"文件夹已处理过，跳过");
            writeToLogFile("文件夹已处理过，跳过");
        }
    }
}

void QtWidgetsApplication2::convertFolder(const QString& folderPath)
{
    if (!isRunning) return; // 如果程序未启动，不执行
    if (!QDir(folderPath).exists()) {
        logMessage(QString(u8"文件夹不存在:1%").arg(folderPath));
        writeToLogFile("文件夹不存在");
        return;
    }
    logMessage(QString(u8"开始处理文件夹: %1").arg(folderPath));
    writeToLogFile("开始处理文件夹");

    // 重置状态
    processedCount = 0;
    successCount = 0;
    errorCount = 0;
    cancelRequested = false;
    ui.progressBar->setValue(0);

    // 查找图片文件
    QStringList filters;
    if (ui.checkBox_2->isChecked()) {
        filters << "*.bmp";
        logMessage(u8"可处理.bmp图片");
    }
    if (ui.checkBox_3->isChecked()) {
        filters << "*.tif";
        logMessage(u8"可处理.tif图片");
    }
    if (ui.checkBox->isChecked()) {
        filters << "*.png";
        logMessage(u8"可处理.png图片");
    }
    if (ui.checkBox_4->isChecked()) {
        filters << "*.jpg";
        logMessage(u8"可处理.jpg图片");
    }
    QDirIterator countIt(folderPath, filters, QDir::Files, QDirIterator::Subdirectories);
    QStringList fileList;
    // 先计算文件总数
    while (countIt.hasNext()) {
        fileList.append(countIt.next());
    }
    totalFiles = fileList.size();
    if (totalFiles == 0) {
        logMessage(u8"没有找到相应图片文件");
        writeToLogFile("没有找到相应图片文件");
        return;
    }
    writeToLogFile("找到相应图片文件");
    logMessage(QString(u8"找到 %1 个相应图片文件").arg(totalFiles));

    // 设置进度条范围
    ui.progressBar->setRange(0, 100);
    // 提交任务到线程池
    for (const QString& filePath : fileList) {
        if (cancelRequested) {
            break;
        }
        // 创建任务并提交到线程池
        ImageProcessingTask* task = new ImageProcessingTask(this, filePath, currentQuality);
        task->setAutoDelete(true);
        threadPool.start(task);
        // 每提交100个任务，短暂休眠以控制任务提交速度
        if (threadPool.activeThreadCount() >= threadPool.maxThreadCount() * 2) {
            QThread::sleep(100);
        }
    }

    //启动进度更新定时器
    QTimer* progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &QtWidgetsApplication2::updateProgress);
    progressTimer->start(1000); // 每1s更新一次进度

    // 启动完成检查定时器
    QTimer* finishTimer = new QTimer(this);
    connect(finishTimer, &QTimer::timeout, this, [=]() {
        if (processedCount.loadAcquire() >= totalFiles.loadAcquire() || cancelRequested) {
            finishTimer->deleteLater();
            progressTimer->deleteLater();
            onProcessingFinished();
        }
        });
    finishTimer->start(1000); //每1S检查一次

    ui.statusBar->showMessage(u8"正在处理图片...");
    logMessage(u8"正在处理图片...");
    writeToLogFile("正在处理图片...");
    //避免不必要的轮询，提高效率
    if (processedCount.loadAcquire() == totalFiles.loadAcquire()) {
        QMetaObject::invokeMethod(this, &QtWidgetsApplication2::onProcessingFinished, Qt::QueuedConnection);
    }
}

void QtWidgetsApplication2::saveSettings()
{
    settings->setValue("rootFolderPath", rootFolderPath);
    settings->setValue("quality", ui.horizontalSlider->value());
    // 保存时间为字符串
    settings->setValue("time", ui.dateTimeEdit->time().toString("HH:mm"));
    settings->setValue("checkBox1State", ui.checkBox->isChecked());
    settings->setValue("checkBox2State", ui.checkBox_2->isChecked());
    settings->setValue("checkBox3State", ui.checkBox_3->isChecked());
    settings->setValue("checkBox4State", ui.checkBox_4->isChecked());
    settings->setValue("saveDay", ui.spinBox->value());
    settings->setValue("deleteDay", ui.spinBox_2->value());
    settings->setValue("thread", ui.spinBox_3->value());
    settings->setValue("process", ui.comboBox->currentIndex());
    settings->sync(); //确保写入磁盘
}
// 处理单个文件（线程安全）
void QtWidgetsApplication2::processFile(const QString& filePath)
{
    //QElapsedTimer timer;
    //timer.start();  //开始计时

    if (cancelRequested) {
        return;
    }

    bool success = false;
    // 检查暂停状态
    if (pauseRequested) {
        QMutexLocker locker(&pauseMutex);
        while (pauseRequested && !cancelRequested) {
            pauseCondition.wait(&pauseMutex, 100); // 等待100毫秒
        }
        if (cancelRequested) return;
    }

    switch (ui.comboBox->currentIndex()){
        
    case 0:     //LY5/6卷绕-贴胶
        success = ImageProcessor::processFile_0(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
        
    case 1:     //LY4卷绕-方壳
        success = ImageProcessor::processFile_1(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
        
    case 2:     //LY4卷绕-圆柱
        success = ImageProcessor::processFile_2(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
        
    case 3:     //LY4密封钉-圆柱
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

    case 7:     //处理webp
        success = ImageProcessor::processFile_webp(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
    case 8:     //处理jpg
        success = ImageProcessor::processFile_jpg(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
    case 9:     //处理png
        success = ImageProcessor::processFile_png(
            filePath,
            currentQuality,
            [this](const QString& msg) {this->logMessage(msg); },
            [this](const char* msg) {this->writeToLogFile(msg); }
        );
        break;
    case 10:     //处理bmp
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
    
    // 处理完每个文件之休眠0.5s，降低CPU使用率
    QThread::msleep(500);
}
// 取消处理
void QtWidgetsApplication2::cancelProcessing()
{
    if (!isRunning) return;
    cancelRequested = true;
    threadPool.clear(); // 清除所有未开始的任务
    writeToLogFile("处理已取消"); 
    logMessage(u8"处理已取消");
    ui.statusBar->showMessage(u8"处理已取消");
}
// 更新进度
void QtWidgetsApplication2::updateProgress()
{
    int processed = processedCount.loadAcquire();
    int total = totalFiles.loadAcquire();

    if (total > 0) {
        int percentage = static_cast<int>((static_cast<double>(processed) / total) * 100);
        ui.progressBar->setValue(percentage);
        ui.statusBar->showMessage(
            QString(u8"处理中: %1/%2 (%3%)")
            .arg(processed)
            .arg(total)
            .arg(percentage)
        );
    }
}
// 处理完成
void QtWidgetsApplication2::onProcessingFinished()
{
    int processed = processedCount.loadAcquire();
    int success = successCount.loadAcquire();
    int error = errorCount.loadAcquire();
    int total = totalFiles.loadAcquire();

    if (cancelRequested) {
        writeToLogFile("处理已取消");
        logMessage(u8"处理已取消");
        ui.statusBar->showMessage(u8"处理已取消");
    }
    else {
        ui.statusBar->showMessage(QString(u8"处理完成! 成功: %1, 失败: %2").arg(success).arg(error));
        writeToLogFile("处理完成");
        logMessage(QString(u8"处理完成! 成功: %1, 失败: %2").arg(success).arg(error));
    }
    threadPool.clear(); // 清楚未开始的任务
    cancelRequested = false; // 重置取消标志
}

// 启动/停止
void QtWidgetsApplication2::toggleRunState()
{
    isRunning = !isRunning;

    if (isRunning) {
        // 启动程序
        ui.pushButton_6->setStyleSheet("background-color: green; color: white;");
        dailyTimer->start(); // 确保定时器运行

        // 启用暂停按钮，禁用恢复按钮
        ui.pushButton_3->setEnabled(true);
        ui.pushButton_4->setEnabled(false);

        ui.pushButton->setEnabled(true); // 
        ui.pushButton_5->setEnabled(true); // 
        ui.pushButton->setEnabled(true); // 

        // 添加日志记录
        logMessage(u8"应用程序启动");
        logMessage(QString(u8"根文件夹: %1").arg(rootFolderPath));
        logMessage(QString(u8"自动处理时间: %1").arg(selectedTime.toString("HH:mm")));
        logMessage(QString(u8"图片质量: %1%").arg(ui.horizontalSlider->value()));
        writeToLogFile("应用程序启动");
    }
    else {
        // 停止程序
        ui.pushButton_6->setStyleSheet("background-color: red; color: white;");
        dailyTimer->stop(); // 停止定时器
        cancelProcessing(); // 取消当前处理

        // 按钮不可用
        ui.pushButton_3->setEnabled(false); // 暂停按钮
        ui.pushButton_4->setEnabled(false); // 启动按钮
        ui.pushButton_5->setEnabled(false); // 取消按钮
        ui.pushButton->setEnabled(false); // 选择文件路径按钮

        logMessage(u8"程序已停止");
        writeToLogFile("程序已停止");
    }
}
// 暂停
void QtWidgetsApplication2::pauseProcessing()
{
    if (!isRunning) return;

    pauseRequested = true;
    ui.pushButton_3->setEnabled(false);
    ui.pushButton_4->setEnabled(true);
    logMessage(u8"处理已暂停");
    writeToLogFile("处理已暂停");
}
// 恢复
void QtWidgetsApplication2::resumeProcessing()
{
    if (!isRunning) return;

    pauseRequested = false;
    // 唤醒所有等待的线程
    pauseCondition.wakeAll();

    ui.pushButton_3->setEnabled(true);
    ui.pushButton_4->setEnabled(false);
    logMessage(u8"处理已恢复");
    writeToLogFile("处理已恢复");
}

// 创建线程安全的日志方法
void QtWidgetsApplication2::logMessage(const QString& message)
{
    // 避免在日志系统自身中记录日志（防止递归）
    static bool inLog = false;
    if (inLog) return;
    inLog = true;
    
    //在UI上显示日志
    QMutexLocker locker(&logMutex);
    QMetaObject::invokeMethod(ui.textEdit, "append",
        Qt::QueuedConnection, Q_ARG(QString, QDateTime::currentDateTime().toString("[hh:mm:ss] ") + message));

    inLog = false;
}

// 初始化日志文件
void QtWidgetsApplication2::initLogFile()
{
    // 创建日志目录
    QDir logDir("logs");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }

    // 设置当前日志日期为今天
    currentLogDate = QDate::currentDate();

    // 创建日志文件名
    QString logFileName = QString("logs/%1.log").arg(currentLogDate.toString("yyyy-MM-dd"));

    // 打开日志文件
    logFile = new QFile(logFileName);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        writeToLogFile("无法打开日志文件");
        qWarning() << u8"无法打开日志文件:" << logFileName;
        delete logFile;
        logFile = nullptr;
        return;
    }

    // 创建日志流
    logStream = new QTextStream(logFile);
    logStream->setCodec("ISO 8859-1");

    // 写入日志头
    *logStream << "========================================\n";
    *logStream << "应用程序启动: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    *logStream << "版本: 1.0\n";
    *logStream << "日志级别: INFO\n";
    *logStream << "========================================\n\n";
    logStream->flush();
}

// 轮换日志文件（按天）
void QtWidgetsApplication2::rotateLogFile()
{
    //QMutexLocker locker(&logFileMutex);

    QDate today = QDate::currentDate();
    if (today == currentLogDate) {
        return; // 不需要轮换
    }

    // 关闭当前日志文件
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

    // 更新日志日期
    currentLogDate = today;

    // 创建新的日志文件
    QString logFileName = QString("logs/%1.log").arg(currentLogDate.toString("yyyy-MM-dd"));

    logFile = new QFile(logFileName);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "无法打开日志文件:" << logFileName;
        delete logFile;
        logFile = nullptr;
        return;
    }

    logStream = new QTextStream(logFile);
    logStream->setCodec("ISO 8859-1");

    // 写入新日志文件的头
    *logStream << "========================================\n";
    *logStream << "应用程序日志 - 新的一天: " << currentLogDate.toString("yyyy-MM-dd") << "\n";
    *logStream << "日志轮换时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    *logStream << "========================================\n\n";
    logStream->flush();
}

// 写入日志到文件
void QtWidgetsApplication2::writeToLogFile(const char* message)
{
    QMutexLocker locker(&logFileMutex);

    // 检查是否需要轮换日志
    if (QDate::currentDate() != currentLogDate) {
        rotateLogFile();
    }
    // 写入日志
    if (logStream && logFile) {
        QString timestamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]");
        *logStream << timestamp << " " << message << "\n";
        logStream->flush();
    }
}

// 锁定/解锁
void QtWidgetsApplication2::onLockButtonClicked()
{
    if (locked) {
        // 如果已锁定，则要求输入密码解锁
        bool ok;
        QString password = QInputDialog::getText(this, "Password Required",
            "Enter Password:", QLineEdit::Password, "", &ok);
        if (ok && password == "123") { // 简单的密码检查
            lockControls(false);
        }
        else {
            QMessageBox::warning(this, "Error", "Incorrect password.");
        }
    }
    else {
        // 如果未锁定，则直接锁定
        lockControls(true);
    }
}

void QtWidgetsApplication2::resetTimer()
{
    lockTimer->start(3 * 60 * 1000); // 重新开始计时，单位为毫秒
}
void QtWidgetsApplication2::lockControls(bool lock = true)
{
    locked = lock;
    for (auto widget : protectedWidgets) {
        widget->setEnabled(!locked);
    }
}
// 线程变化自动更新
void QtWidgetsApplication2::spinBoxTextChanged()
{
    //加载设置中线程数
    int threads = ui.spinBox_3->value();
    //初始化线程
    int maxThreads = QThread::idealThreadCount() - 1; // 保留一个线程给UI
    if (maxThreads < 1) maxThreads = 1;
    if (maxThreads > 2) maxThreads = threads; // 限制最大线程数
    threadPool.setMaxThreadCount(maxThreads);

    logMessage(QString(u8"使用%1个线程处理图片").arg(maxThreads));
}

// 添加按钮点击函数，防止后台报错
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