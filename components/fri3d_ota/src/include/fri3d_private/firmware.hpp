#pragma once

#include <map>
#include <string>
#include <vector>

#include "fri3d_private/version.hpp"

namespace Fri3d::Apps::Ota
{

struct CImage
{
    enum ImageType
    {
        Main,
        MicroPython,
        RetroGoLauncher,
        RetroGoCore,
        RetroGoPRBoom,
        VFS
    };

    typedef const std::map<std::string, ImageType> StringToTypeMap;
    static StringToTypeMap jsonStringToType;

    typedef const std::map<ImageType, std::string> TypeToStringMap;
    static TypeToStringMap typeToUIString;

    ImageType imageType;
    CVersion version;
    std::string url;
    int size;

    friend bool operator<(const CImage &l, const CImage &r);
};

struct CFirmware
{
    typedef std::map<CImage::ImageType, CImage> CImages;

    CVersion version;
    bool beta;
    CImages images;

    friend bool operator<(const CFirmware &l, const CFirmware &r);
};

typedef std::vector<CFirmware> CFirmwares;

} // namespace Fri3d::Apps::Ota
