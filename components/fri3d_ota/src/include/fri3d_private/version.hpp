#pragma once

#include <string>
#include <vector>

#include "semver.h"

namespace Fri3d::Apps::Ota
{

struct CVersion
{
    // Note that there is no synchronization between the two members
    std::string text;
    semver_t semver;

    CVersion();
    explicit CVersion(const char *version);
    ~CVersion();
    friend bool operator<(const CVersion &l, const CVersion &r);
};

struct CFirmwareVersion
{
    CVersion version;
    std::string url;
    int size;

    friend bool operator<(const CFirmwareVersion &l, const CFirmwareVersion &r);
};

typedef std::vector<CFirmwareVersion> CVersions;

class CVersionFetcher
{
private:
    CVersions versions;

    static std::string fetch();
    bool parse(const char *json);

public:
    CVersionFetcher();

    [[nodiscard]] bool refresh();
    [[nodiscard]] const CVersions &getVersions(bool beta) const;
};

} // namespace Fri3d::Apps::Ota
