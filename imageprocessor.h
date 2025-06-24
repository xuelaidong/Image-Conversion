#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QString>
#include <functional>

class ImageProcessor
{
public:

    //LY5/6����-����
    static bool processFile_0(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY4����-����
    static bool processFile_1(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY4����-Բ��
    static bool processFile_2(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY4�ܷⶤ-Բ��
    static bool processFile_3(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY1&2-XRay
    static bool processFile_4(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY3-XRay
    static bool processFile_5(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY5/6-XRay
    static bool processFile_6(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //����Ϊwebp
    static bool processFile_webp(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );
    //����Ϊjpg
    static bool processFile_jpg(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );
    //����Ϊbmp
    static bool processFile_bmp(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );
    //����Ϊpng
    static bool processFile_png(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );
};

#endif // IMAGEPROCESSOR_H