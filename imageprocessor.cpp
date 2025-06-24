#include "imageprocessor.h"
#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QDir>
#include <stdexcept>
#include <QRegularExpression>
#include <QImageReader>
#include <QThread>

bool ImageProcessor::processFile_0(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback)
{
    QFileInfo fileInfo(filePath);
    QString jpgPath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-a.jpg";
    try {
        // ʹ��Qt����ͼ��
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("�޷�����ͼ��.");
            throw std::runtime_error("�޷�����ͼ��");
        }
        // ����ΪJPEG
        if (!image.save(jpgPath, "JPEG", currentQuality)) {
            writeLogCallback("�޷�����JPGͼ��");
            throw std::runtime_error("�޷�����JPGͼ��");
        }
        // ��֤�ļ�
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("���ɵ��ļ���Ч");
            throw std::runtime_error("���ɵ��ļ���Ч");
        }
        // ɾ��ԭʼ�ļ�
        if (!QFile::remove(filePath)) {
            writeLogCallback("�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
        }
        // �����ɹ��ź�
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
        return false;
    }
}

bool ImageProcessor::processFile_1(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    QFileInfo fileInfo(filePath);
    QString outputFormat;
    QString suffix;
    QString baseName = fileInfo.completeBaseName();

    // ����ļ����Ƿ���������ַ�������
    if (baseName.contains(QRegularExpression(u8"��"))) {
        outputFormat = "PNG";
        suffix = "-a.png";
    }
    else {
        outputFormat = "Webp";
        suffix = ".webp";
    }

    QString jpgPath = fileInfo.path() + "/" + baseName + suffix;

    try {

        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("�޷�����ͼ��.");
            throw std::runtime_error("�޷�����ͼ��");
        }

        bool saveSuccess;
        if (outputFormat == "Webp") {
            saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data()); // ����Webp��������
        }
        else {
            saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data()); // PNG��ʹ��quality����
        }

        // ����ΪWebp
        if (!saveSuccess) {
            writeLogCallback("�޷�����ͼ��");
            throw std::runtime_error("�޷�����ͼ��");
        }

        // ��֤�ļ�
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("���ɵ��ļ���Ч");
            throw std::runtime_error("���ɵ��ļ���Ч");
        }
        // ɾ��ԭʼ�ļ�
        if (!QFile::remove(filePath)) {
            writeLogCallback("�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
        }
        // �����ɹ��ź�
        return(true);
    }
    catch (const std::exception& e) {
            writeLogCallback("�������");
            logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
        return(false);
    }
}

bool ImageProcessor::processFile_2(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    QFileInfo fileInfo(filePath);
    QString jpgPath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-a.jpg";
    try {
        // ʹ��Qt����ͼ��
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("�޷�����ͼ��");
            throw std::runtime_error("�޷�����ͼ��");
        }
        // ����ΪJPEG
        if (!image.save(jpgPath, "JPEG", currentQuality)) {
            writeLogCallback("�޷�����JPGͼ��");
            throw std::runtime_error("�޷�����JPGͼ��");
        }
        // ��֤�ļ�
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("���ɵ��ļ���Ч");
            throw std::runtime_error("���ɵ��ļ���Ч");
        }
        // ɾ��ԭʼ�ļ�
        if (!QFile::remove(filePath)) {
            writeLogCallback("�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
        }
        // �����ɹ��ź�
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
        return false;
    }
}

bool ImageProcessor::processFile_3(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback)
{
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.completeBaseName();

    QString outputFormat = "jpg";
    QString suffix = "-a.jpg";
    QString jpgPath = fileInfo.path() + "/" + baseName + suffix;

    QString outputFormat_2 = "png";
    QString suffix_2 = "-b.png";
    QString jpgPath_2 = fileInfo.path() + "/" + baseName + suffix_2;

    // ֻ����OK·���ļ���
    if (jpgPath.contains("/OK/")) {
        try {
            QImage image(filePath);
            if (image.isNull()) {
                if (writeLogCallback) writeLogCallback("�޷�����ͼ��");
                throw std::runtime_error("�޷�����ͼ��");
            }

            bool saveSuccess;
            if (baseName.contains("Color")) {
                saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data(), currentQuality);
            }
            else if (baseName.contains("Gray")) {
                saveSuccess = image.save(jpgPath_2, outputFormat_2.toLatin1().data(), currentQuality / 10);
            }
            else {
                saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data(), currentQuality);
            }

            if (!saveSuccess) {
                if (writeLogCallback) writeLogCallback("�޷�����ͼ��");
                throw std::runtime_error("�޷�����ͼ��");
            }

            if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
                if (writeLogCallback) writeLogCallback("���ɵ��ļ���Ч");
                throw std::runtime_error("���ɵ��ļ���Ч");
            }

            if (!QFile::remove(filePath)) {
                if (logMessageCallback) logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            }
            return true;
        }
        catch (const std::exception& e) {
            if (logMessageCallback) logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
            return false;
        }
    }
    else {
        if (logMessageCallback) logMessageCallback(u8"NG�ļ�����ͼƬ����");
        return true; // ���Ϊ�ɹ�����
    }
}

bool ImageProcessor::processFile_4(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.completeBaseName();

    QString outputFormat = "jpg";
    QString suffix = "-a.jpg";
    QString jpgPath = fileInfo.path() + "/" + baseName + suffix;

        try {
            QImage image(filePath);
            if (image.isNull()) {
                if (writeLogCallback) writeLogCallback("�޷�����ͼ��");
                throw std::runtime_error("�޷�����ͼ��");
            }

            bool saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data(), currentQuality);

            if (!saveSuccess) {
                if (writeLogCallback) writeLogCallback("�޷�����ͼ��");
                throw std::runtime_error("�޷�����ͼ��");
            }

            if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
                if (writeLogCallback) writeLogCallback("���ɵ��ļ���Ч");
                throw std::runtime_error("���ɵ��ļ���Ч");
            }

            if (!QFile::remove(filePath)) {
                if (logMessageCallback) logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            }
            return true;
        }
        catch (const std::exception& e) {
            if (logMessageCallback) logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
            return false;
        }
}

bool ImageProcessor::processFile_5(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.completeBaseName();

    QString outputFormat = "jpg";
    QString suffix = "-a.jpg";
    QString jpgPath = fileInfo.path() + "/" + baseName + suffix;

    try {
        QImage image(filePath);
        if (image.isNull()) {
            if (writeLogCallback) writeLogCallback("�޷�����ͼ��");
            throw std::runtime_error("�޷�����ͼ��");
        }

        bool saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data(), currentQuality);

        if (!saveSuccess) {
            if (writeLogCallback) writeLogCallback("�޷�����ͼ��");
            throw std::runtime_error("�޷�����ͼ��");
        }

        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            if (writeLogCallback) writeLogCallback("���ɵ��ļ���Ч");
            throw std::runtime_error("���ɵ��ļ���Ч");
        }

        if (!QFile::remove(filePath)) {
            if (logMessageCallback) logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
        }
        return true;
    }
    catch (const std::exception& e) {
        if (logMessageCallback) logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
        return false;
    }
}

bool ImageProcessor::processFile_6(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.completeBaseName();

    QString outputFormat = "jpg";
    QString suffix = "-a.jpg";
    QString jpgPath = fileInfo.path() + "/" + baseName + suffix;

    // ֻ����OK·���ļ���
    if (jpgPath.contains("/Result_OK/")) {
        try {
            QImage image(filePath);
            if (image.isNull()) {
                if (writeLogCallback) writeLogCallback("�޷�����ͼ��");
                throw std::runtime_error("�޷�����ͼ��");
            }

            bool saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data(), currentQuality);

            if (!saveSuccess) {
                if (writeLogCallback) writeLogCallback("�޷�����ͼ��");
                throw std::runtime_error("�޷�����ͼ��");
            }

            if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
                if (writeLogCallback) writeLogCallback("���ɵ��ļ���Ч");
                throw std::runtime_error("���ɵ��ļ���Ч");
            }

            if (!QFile::remove(filePath)) {
                if (logMessageCallback) logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            }
            return true;
        }
        catch (const std::exception& e) {
            if (logMessageCallback) logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
            return false;
        }
    }
    else {
        if (logMessageCallback) logMessageCallback(u8"NG�ļ�����ͼƬ����");
        return true; // ���Ϊ�ɹ�����
    }
}

bool ImageProcessor::processFile_webp(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    QFileInfo fileInfo(filePath);
    QString jpgPath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-a.webp";
    try {
        // ʹ��Qt����ͼ��
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("�޷�����ͼ��");
            throw std::runtime_error("�޷�����ͼ��");
        }
        // ����ΪJPEG
        if (!image.save(jpgPath, "Webp")) {
            writeLogCallback("�޷�����webpͼ��");
            throw std::runtime_error("�޷�����webpͼ��");
        }
        // ��֤�ļ�
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("���ɵ��ļ���Ч");
            throw std::runtime_error("���ɵ��ļ���Ч");
        }
        // ɾ��ԭʼ�ļ�
        if (!QFile::remove(filePath)) {
            writeLogCallback("�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
        }
        // �����ɹ��ź�
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
        return false;
    }
}

bool ImageProcessor::processFile_jpg(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    QFileInfo fileInfo(filePath);
    QString jpgPath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-a.jpg";
    try {
        // ʹ��Qt����ͼ��
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("�޷�����ͼ��");
            throw std::runtime_error("�޷�����ͼ��");
        }
        // ����ΪJPEG
        if (!image.save(jpgPath, "JPEG", currentQuality)) {
            writeLogCallback("�޷�����JPGͼ��");
            throw std::runtime_error("�޷�����JPGͼ��");
        }
        // ��֤�ļ�
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("���ɵ��ļ���Ч");
            throw std::runtime_error("���ɵ��ļ���Ч");
        }
        // ɾ��ԭʼ�ļ�
        if (!QFile::remove(filePath)) {
            writeLogCallback("�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
        }
        // �����ɹ��ź�
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
        return false;
    }
}

bool ImageProcessor::processFile_png(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    QFileInfo fileInfo(filePath);
    QString jpgPath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-a.png";
    try {
        // ʹ��Qt����ͼ��
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("�޷�����ͼ��");
            throw std::runtime_error("�޷�����ͼ��");
        }
        // ����ΪJPEG
        if (!image.save(jpgPath, "png", currentQuality)) {
            writeLogCallback("�޷�����pngͼ��");
            throw std::runtime_error("�޷�����pngͼ��");
        }
        // ��֤�ļ�
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("���ɵ��ļ���Ч");
            throw std::runtime_error("���ɵ��ļ���Ч");
        }
        // ɾ��ԭʼ�ļ�
        if (!QFile::remove(filePath)) {
            writeLogCallback("�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
        }
        // �����ɹ��ź�
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
        return false;
    }
}

bool ImageProcessor::processFile_bmp(
    const QString& filePath,
    int currentQuality,
    std::function<void(const QString&)> logMessageCallback,
    std::function<void(const char*)> writeLogCallback) {
    QFileInfo fileInfo(filePath);
    QString jpgPath = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-a.bmp";
    try {
        // ʹ��Qt����ͼ��
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("�޷�����ͼ��");
            throw std::runtime_error("�޷�����ͼ��");
        }
        // ����ΪJPEG
        if (!image.save(jpgPath, "bmp", currentQuality)) {
            writeLogCallback("�޷�����bmpͼ��");
            throw std::runtime_error("�޷�����bmpͼ��");
        }
        // ��֤�ļ�
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("���ɵ��ļ���Ч");
            throw std::runtime_error("���ɵ��ļ���Ч");
        }
        // ɾ��ԭʼ�ļ�
        if (!QFile::remove(filePath)) {
            writeLogCallback("�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
            logMessageCallback(u8"�޷�ɾ��ԭʼ�ļ�,����ΪNG�ļ��������˲���");
        }
        // �����ɹ��ź�
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"�������%1 - %2").arg(filePath).arg(e.what()));
        return false;
    }
}