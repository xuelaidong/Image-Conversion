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
        // 使用Qt加载图像
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("无法加载图像.");
            throw std::runtime_error("无法加载图像");
        }
        // 保存为JPEG
        if (!image.save(jpgPath, "JPEG", currentQuality)) {
            writeLogCallback("无法保存JPG图像");
            throw std::runtime_error("无法保存JPG图像");
        }
        // 验证文件
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("生成的文件无效");
            throw std::runtime_error("生成的文件无效");
        }
        // 删除原始文件
        if (!QFile::remove(filePath)) {
            writeLogCallback("无法删除原始文件,可能为NG文件，跳过此步骤");
            logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
        }
        // 发出成功信号
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
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

    // 检测文件名是否包含中文字符“部”
    if (baseName.contains(QRegularExpression(u8"部"))) {
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
            writeLogCallback("无法加载图像.");
            throw std::runtime_error("无法加载图像");
        }

        bool saveSuccess;
        if (outputFormat == "Webp") {
            saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data()); // 传递Webp质量参数
        }
        else {
            saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data()); // PNG不使用quality参数
        }

        // 保存为Webp
        if (!saveSuccess) {
            writeLogCallback("无法保存图像");
            throw std::runtime_error("无法保存图像");
        }

        // 验证文件
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("生成的文件无效");
            throw std::runtime_error("生成的文件无效");
        }
        // 删除原始文件
        if (!QFile::remove(filePath)) {
            writeLogCallback("无法删除原始文件,可能为NG文件，跳过此步骤");
            logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
        }
        // 发出成功信号
        return(true);
    }
    catch (const std::exception& e) {
            writeLogCallback("处理错误：");
            logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
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
        // 使用Qt加载图像
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("无法加载图像");
            throw std::runtime_error("无法加载图像");
        }
        // 保存为JPEG
        if (!image.save(jpgPath, "JPEG", currentQuality)) {
            writeLogCallback("无法保存JPG图像");
            throw std::runtime_error("无法保存JPG图像");
        }
        // 验证文件
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("生成的文件无效");
            throw std::runtime_error("生成的文件无效");
        }
        // 删除原始文件
        if (!QFile::remove(filePath)) {
            writeLogCallback("无法删除原始文件,可能为NG文件，跳过此步骤");
            logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
        }
        // 发出成功信号
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
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

    // 只处理OK路径文件夹
    if (jpgPath.contains("/OK/")) {
        try {
            QImage image(filePath);
            if (image.isNull()) {
                if (writeLogCallback) writeLogCallback("无法加载图像");
                throw std::runtime_error("无法加载图像");
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
                if (writeLogCallback) writeLogCallback("无法保存图像");
                throw std::runtime_error("无法保存图像");
            }

            if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
                if (writeLogCallback) writeLogCallback("生成的文件无效");
                throw std::runtime_error("生成的文件无效");
            }

            if (!QFile::remove(filePath)) {
                if (logMessageCallback) logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
            }
            return true;
        }
        catch (const std::exception& e) {
            if (logMessageCallback) logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
            return false;
        }
    }
    else {
        if (logMessageCallback) logMessageCallback(u8"NG文件夹中图片跳过");
        return true; // 标记为成功跳过
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
                if (writeLogCallback) writeLogCallback("无法加载图像");
                throw std::runtime_error("无法加载图像");
            }

            bool saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data(), currentQuality);

            if (!saveSuccess) {
                if (writeLogCallback) writeLogCallback("无法保存图像");
                throw std::runtime_error("无法保存图像");
            }

            if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
                if (writeLogCallback) writeLogCallback("生成的文件无效");
                throw std::runtime_error("生成的文件无效");
            }

            if (!QFile::remove(filePath)) {
                if (logMessageCallback) logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
            }
            return true;
        }
        catch (const std::exception& e) {
            if (logMessageCallback) logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
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
            if (writeLogCallback) writeLogCallback("无法加载图像");
            throw std::runtime_error("无法加载图像");
        }

        bool saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data(), currentQuality);

        if (!saveSuccess) {
            if (writeLogCallback) writeLogCallback("无法保存图像");
            throw std::runtime_error("无法保存图像");
        }

        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            if (writeLogCallback) writeLogCallback("生成的文件无效");
            throw std::runtime_error("生成的文件无效");
        }

        if (!QFile::remove(filePath)) {
            if (logMessageCallback) logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
        }
        return true;
    }
    catch (const std::exception& e) {
        if (logMessageCallback) logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
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

    // 只处理OK路径文件夹
    if (jpgPath.contains("/Result_OK/")) {
        try {
            QImage image(filePath);
            if (image.isNull()) {
                if (writeLogCallback) writeLogCallback("无法加载图像");
                throw std::runtime_error("无法加载图像");
            }

            bool saveSuccess = image.save(jpgPath, outputFormat.toLatin1().data(), currentQuality);

            if (!saveSuccess) {
                if (writeLogCallback) writeLogCallback("无法保存图像");
                throw std::runtime_error("无法保存图像");
            }

            if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
                if (writeLogCallback) writeLogCallback("生成的文件无效");
                throw std::runtime_error("生成的文件无效");
            }

            if (!QFile::remove(filePath)) {
                if (logMessageCallback) logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
            }
            return true;
        }
        catch (const std::exception& e) {
            if (logMessageCallback) logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
            return false;
        }
    }
    else {
        if (logMessageCallback) logMessageCallback(u8"NG文件夹中图片跳过");
        return true; // 标记为成功跳过
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
        // 使用Qt加载图像
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("无法加载图像");
            throw std::runtime_error("无法加载图像");
        }
        // 保存为JPEG
        if (!image.save(jpgPath, "Webp")) {
            writeLogCallback("无法保存webp图像");
            throw std::runtime_error("无法保存webp图像");
        }
        // 验证文件
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("生成的文件无效");
            throw std::runtime_error("生成的文件无效");
        }
        // 删除原始文件
        if (!QFile::remove(filePath)) {
            writeLogCallback("无法删除原始文件,可能为NG文件，跳过此步骤");
            logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
        }
        // 发出成功信号
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
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
        // 使用Qt加载图像
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("无法加载图像");
            throw std::runtime_error("无法加载图像");
        }
        // 保存为JPEG
        if (!image.save(jpgPath, "JPEG", currentQuality)) {
            writeLogCallback("无法保存JPG图像");
            throw std::runtime_error("无法保存JPG图像");
        }
        // 验证文件
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("生成的文件无效");
            throw std::runtime_error("生成的文件无效");
        }
        // 删除原始文件
        if (!QFile::remove(filePath)) {
            writeLogCallback("无法删除原始文件,可能为NG文件，跳过此步骤");
            logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
        }
        // 发出成功信号
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
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
        // 使用Qt加载图像
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("无法加载图像");
            throw std::runtime_error("无法加载图像");
        }
        // 保存为JPEG
        if (!image.save(jpgPath, "png", currentQuality)) {
            writeLogCallback("无法保存png图像");
            throw std::runtime_error("无法保存png图像");
        }
        // 验证文件
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("生成的文件无效");
            throw std::runtime_error("生成的文件无效");
        }
        // 删除原始文件
        if (!QFile::remove(filePath)) {
            writeLogCallback("无法删除原始文件,可能为NG文件，跳过此步骤");
            logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
        }
        // 发出成功信号
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
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
        // 使用Qt加载图像
        QImage image(filePath);
        if (image.isNull()) {
            writeLogCallback("无法加载图像");
            throw std::runtime_error("无法加载图像");
        }
        // 保存为JPEG
        if (!image.save(jpgPath, "bmp", currentQuality)) {
            writeLogCallback("无法保存bmp图像");
            throw std::runtime_error("无法保存bmp图像");
        }
        // 验证文件
        if (!QFile::exists(jpgPath) || QFileInfo(jpgPath).size() == 0) {
            writeLogCallback("生成的文件无效");
            throw std::runtime_error("生成的文件无效");
        }
        // 删除原始文件
        if (!QFile::remove(filePath)) {
            writeLogCallback("无法删除原始文件,可能为NG文件，跳过此步骤");
            logMessageCallback(u8"无法删除原始文件,可能为NG文件，跳过此步骤");
        }
        // 发出成功信号
        return true;
    }
    catch (const std::exception& e) {
        logMessageCallback(QString(u8"处理错误：%1 - %2").arg(filePath).arg(e.what()));
        return false;
    }
}