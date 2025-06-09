// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace rpmcmplib {

namespace utils {

bool contains(const std::string& str, const std::string& substr) {
    return (str.find(substr) != std::string::npos);
}

} // namespace utils

class RpmVer {
public:
    
    /**
     * @throw invalid_argument if there is invalid version value
     */
    RpmVer(const std::string& version);
    
    /**
     * Check label (Version or Release tag) for validity.
     * 
     * @param label - label that is checked for validity
     * @return String which is empty in case when label is valid
     * and non empty when label is invalid.
     * If label is invalid, than returned sting contains description of invalidity.
     */
    static const std::string isValid(const std::string &label);
    
    /**
     * Compare the labels.
     * 
     * @param lhs - first label to compare
     * @param rhs - second label to compare
     * @return comparison result:
     *   1  if lhs > rhs
     *   0  if lhs == rhs
     *  -1  if lhs < rhs
     * @throw invalid_argument if there is invalid lhs or rhs value
     */
    static int cmp(const std::string& lhs, const std::string& rhs);
    
    /**
     * Split label into segments.
     */
    static const std::vector<std::string_view> segments(std::string_view label);
    
    std::string version() const;
    
    bool operator>(const RpmVer& other);
    bool operator<(const RpmVer& other);
    bool operator==(const RpmVer& other);
    
    bool operator>=(const RpmVer& other) = delete;
    bool operator<=(const RpmVer& other) = delete;

private:
    int cmp_impl(const RpmVer& other);

    std::string m_version;
};

class RpmEvr {
public:

    /**
     * @throw invalid_argument if there is invalid evr value
     */
    RpmEvr(const std::string& evr);
    
    /**
     * Check EVR for validity.
     * 
     * @param evr - evr that is checked for validity
     * @return String which is empty in case when EVR is valid
     * and non empty when EVR is invalid.
     * If EVR is invalid, than returned sting contains description of invalidity.
     */
    static const std::string isValid(const std::string& evr);
    
    /**
     * Compare the EVR.
     * 
     * @param lhs - first EVR to compare
     * @param rhs - second EVR to compare
     * @return comparison result:
     *   1  if lhs > rhs
     *   0  if lhs == rhs
     *  -1  if lhs < rhs
     * @throw invalid_argument if there is invalid lhs or rhs value
     */
    static int cmp(const std::string& lhs, const std::string& rhs);
    
    unsigned long long int epoch() const;
    std::string version() const;
    std::string release() const;
    
    bool operator>(const RpmEvr& other);
    bool operator<(const RpmEvr& other);
    bool operator==(const RpmEvr& other);
    
    bool operator>=(const RpmEvr& other) = delete;
    bool operator<=(const RpmEvr& other) = delete;

private:
    int cmp_impl(const RpmEvr& other);
    void parseEvr(const std::string& evr);

    unsigned long long int m_epoch = 0;
    std::string m_version;
    std::string m_release;
};

/* ======================================== VER ======================================== */
RpmVer::RpmVer(const std::string& version) {
    auto isValidCheckResult = rpmcmplib::RpmVer::isValid(version);
    if (!isValidCheckResult.empty()) {
        throw std::invalid_argument(isValidCheckResult);
    }

    m_version = version;
}

const std::string RpmVer::isValid(const std::string &label) {
    if (label.find("-") != std::string::npos) {
        return "Label can't have hyphen symbol!";
    }

    return "";
}

int RpmVer::cmp(const std::string& lhs, const std::string& rhs) {
    RpmVer lhsEvr = RpmVer(lhs);
    RpmVer rhsEvr = RpmVer(rhs);

    if (lhsEvr < rhsEvr) {
        return -1;
    } else if (lhsEvr == rhsEvr) {
        return 0;
    }

    return 1;
}

const std::vector<std::string_view> RpmVer::segments(std::string_view label) {
    std::vector<std::string_view> segmentsVector;
    std::string_view segment;
    for(size_t i = 0; i < label.size(); ++i) {
        if ( std::isdigit(label.at(i)) || std::isalpha(label.at(i)) ) {
            if (segment.empty()) {
                segment = label.substr(i, 1);
            } else if ( (std::isdigit(label.at(i)) && std::isdigit(segment.back())) ||
                        (std::isalpha(label.at(i)) && std::isalpha(segment.back())) ) {
                segment = {segment.data(), segment.size() + 1};
            } else {
                segmentsVector.push_back(segment);
                segment = label.substr(i, 1);
            }
        } else {
            if (!segment.empty()) {
                segmentsVector.push_back(segment);
                segment = {"", 0};
            }
        }
    }

    if (!segment.empty()) {
        segmentsVector.push_back(segment);
    }

    return segmentsVector;
}

std::string RpmVer::version() const {
    return m_version;
}

bool RpmVer::operator>(const RpmVer& other) {
    if (cmp_impl(other) == 1) {
        return true;
    }
    return false;
}

bool RpmVer::operator<(const RpmVer& other) {
    if (cmp_impl(other) == -1) {
        return true;
    }
    return false;
}

bool RpmVer::operator==(const RpmVer& other) {
    if (cmp_impl(other) == 0) {
        return true;
    }
    return false;
}

int RpmVer::cmp_impl(const RpmVer& other) {
    // check for tilde and caret
    if (utils::contains(version(), "~")) {
        return -1;
    } else if (utils::contains(other.version(), "~")) {
        return 1;
    }

    if (utils::contains(version(), "^")) {
        return 1;
    } else if (utils::contains(other.version(), "^")) {
        return -1;
    }

    // split into segments
    auto lhsSegments = segments({version().c_str(), version().size()});
    auto rhsSegments = segments({other.version().c_str(), other.version().size()});

    // compare segments
    size_t length = std::min(lhsSegments.size(), rhsSegments.size());
    for (size_t i = 0; i < length; ++i) {
        long long int lhsNumber = 0;
        long long int rhsNumber = 0;
        bool lhsIsNumber = true;
        bool rhsIsNumber = true;

        auto [lhsPtr, lhsEc] = std::from_chars(lhsSegments.at(i).data(),
                                               lhsSegments.at(i).data() + lhsSegments.at(i).size(),
                                               lhsNumber);
        if (lhsEc != std::errc()) {
            lhsIsNumber = false;
        }

        auto [rhsPtr, rhsEc] = std::from_chars(rhsSegments.at(i).data(),
                                               rhsSegments.at(i).data() + rhsSegments.at(i).size(),
                                               rhsNumber);
        if (rhsEc != std::errc()) {
            rhsIsNumber = false;
        }

        if (lhsIsNumber && rhsIsNumber) { // compare as numeric
            if (lhsNumber > rhsNumber) {
                return 1;
            } else if (lhsNumber < rhsNumber) {
                return -1;
            }
        } else if (!lhsIsNumber && !rhsIsNumber) { // compare as alphabetic
            if (lhsSegments.at(i) != rhsSegments.at(i)) {
                if (lhsSegments.at(i).compare(rhsSegments.at(i)) < 0) {
                    return -1;
                } else {
                    return 1;
                }
            }
        } else { // numeric elements is newer than alphabetic
            if(lhsIsNumber) {
                return 1;
            } else if (rhsIsNumber) {
                return -1;
            }
        }
    }

    // if segments are equal then longer segment wins
    if(lhsSegments.size() == rhsSegments.size()) {
        return 0;
    } else if (lhsSegments.size() > rhsSegments.size()) {
        return 1;
    }

    return -1;
}

/* ======================================== EVR ======================================== */
RpmEvr::RpmEvr(const std::string& evr) {
    auto isValidCheckResult = rpmcmplib::RpmEvr::isValid(evr);
    if (!isValidCheckResult.empty()) {
        throw std::invalid_argument(isValidCheckResult);
    }

    parseEvr(evr);
}

const std::string RpmEvr::isValid(const std::string& evr) {
    if (std::count(evr.cbegin(), evr.cend(), ':') > 1) {
        return "EVR must contain only one colon symbol!";
    }

    if (utils::contains(evr, ":")) {
        if (std::stoi(evr.substr(0, evr.find(':'))) < 0) {
            return "Epoch must be a positive number!";
        }
    }

    if (std::count(evr.cbegin(), evr.cend(), '-') > 1) {
        return "EVR must contain only one hyphen symbol!";
    }

    return "";
}

int RpmEvr::cmp(const std::string& lhs, const std::string& rhs) {
    RpmEvr lhsEvr = RpmEvr(lhs);
    RpmEvr rhsEvr = RpmEvr(rhs);

    if (lhsEvr < rhsEvr) {
        return -1;
    } else if (lhsEvr == rhsEvr) {
        return 0;
    }

    return 1;
}

unsigned long long int RpmEvr::epoch() const {
    return m_epoch;
}

std::string RpmEvr::version() const {
    return m_version;
}

std::string RpmEvr::release() const {
    return m_release;
}

bool RpmEvr::operator>(const RpmEvr& other) {
    if (cmp_impl(other) == 1) {
        return true;
    }
    return false;
}

bool RpmEvr::operator<(const RpmEvr& other) {
    if (cmp_impl(other) == -1) {
        return true;
    }
    return false;
}

bool RpmEvr::operator==(const RpmEvr& other) {
    if (cmp_impl(other) == 0) {
        return true;
    }
    return false;
}

int RpmEvr::cmp_impl(const RpmEvr& other) {
    if (epoch() > other.epoch()) {
        return 1;
    } else if (epoch() < other.epoch()) {
        return -1;
    }

    int versionComparison = RpmVer::cmp(version(), other.version());
    if ( versionComparison != 0) {
        return versionComparison;
    }

    int releaseComparison = RpmVer::cmp(release(), other.release());
    if ( releaseComparison != 0) {
        return releaseComparison;
    }

    return 0;
}

void RpmEvr::parseEvr(const std::string& evr) {
    if (utils::contains(evr, ":")) {
        m_epoch = std::stoul(evr.substr(0, evr.find(':')));
    }

    if (utils::contains(evr, "-")) {
        size_t versionStart = (evr.find(':') == evr.npos) ? 0 : evr.find(':') + 1;
        size_t versionSize = evr.find('-') - versionStart;
        m_version = evr.substr(versionStart, versionSize);
        m_release = evr.substr(evr.find('-')+1, evr.size());
    } else if (utils::contains(evr, ":")) {
        m_version = evr.substr(evr.find(':')+1, evr.size());
    } else {
        m_version = evr;
    }
}

}  //namespace rpmcmplib