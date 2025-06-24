#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QString>
#include <functional>

class ImageProcessor
{
public:

    //LY5/6¾íÈÆ-Ìù½º
    static bool processFile_0(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY4¾íÈÆ-·½¿Ç
    static bool processFile_1(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY4¾íÈÆ-Ô²Öù
    static bool processFile_2(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );

    //LY4ÃÜ·â¶¤-Ô²Öù
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

    //±£´æÎªwebp
    static bool processFile_webp(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );
    //±£´æÎªjpg
    static bool processFile_jpg(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );
    //±£´æÎªbmp
    static bool processFile_bmp(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );
    //±£´æÎªpng
    static bool processFile_png(
        const QString& filePath,
        int currentQuality,
        std::function<void(const QString&)> logMessageCallback,
        std::function<void(const char*)> writeLogCallback
    );
};

#endif // IMAGEPROCESSOR_H