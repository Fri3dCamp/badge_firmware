#pragma once

#include <map>
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
    CVersion(const CVersion &other);
    explicit CVersion(const char *version);

    ~CVersion();

    [[nodiscard]] CVersion simplify() const;
    bool empty() const;

    CVersion &operator=(const CVersion &other);
    CVersion &operator=(CVersion &&other) noexcept;
    friend bool operator<(const CVersion &l, const CVersion &r);
    friend bool operator>(const CVersion &l, const CVersion &r);
    bool operator==(const CVersion &other) const;
};

} // namespace Fri3d::Apps::Ota
