#include <algorithm>

#include "fri3d_private/firmware.hpp"

namespace Fri3d::Apps::Ota
{

CImage::StringToTypeMap CImage::jsonStringToType = {
    {"main", Main},
    {"micropython", MicroPython},
    {"retro-launcher", RetroGoLauncher},
    {"retro-core", RetroGoCore},
    {"retro-prboom", RetroGoPRBoom},
    {"vfs", VFS}};

CImage::TypeToStringMap CImage::typeToUIString = {
    {Main, "Main firmware"},
    {MicroPython, "MicroPython"},
    {RetroGoLauncher, "Retro-Go Launcher"},
    {RetroGoCore, "Retro-Go Gaming"},
    {RetroGoPRBoom, "Doom"},
    {VFS, "User Data (VFS)"},
};

bool operator<(const CImage &l, const CImage &r)
{
    return l.version < r.version;
}

bool operator<(const CFirmware &l, const CFirmware &r)
{
    return l.version < r.version;
}

} // namespace Fri3d::Apps::Ota
