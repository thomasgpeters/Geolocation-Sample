#include "BusinessInfo.h"
#include <algorithm>

namespace FranchiseAI {
namespace Models {

std::string BusinessInfo::getBusinessTypeString() const {
    return businessTypeToString(type);
}

std::string BusinessInfo::getDataSourceString() const {
    return dataSourceToString(source);
}

std::string BusinessInfo::getBBBRatingString() const {
    return bbbRatingToString(bbbRating);
}

std::string BusinessInfo::getCateringPotentialDescription() const {
    if (cateringPotentialScore >= 80) return "Excellent Prospect";
    if (cateringPotentialScore >= 60) return "High Potential";
    if (cateringPotentialScore >= 40) return "Moderate Potential";
    if (cateringPotentialScore >= 20) return "Low Potential";
    return "Minimal Potential";
}

void BusinessInfo::calculateCateringPotential() {
    int score = 0;

    // Employee count scoring (more employees = more catering potential)
    if (employeeCount >= 500) score += 25;
    else if (employeeCount >= 200) score += 20;
    else if (employeeCount >= 100) score += 15;
    else if (employeeCount >= 50) score += 10;
    else if (employeeCount >= 20) score += 5;

    // Business type scoring
    switch (type) {
        case BusinessType::CORPORATE_OFFICE:
        case BusinessType::TECH_COMPANY:
        case BusinessType::FINANCIAL_SERVICES:
            score += 20;
            break;
        case BusinessType::CONFERENCE_CENTER:
        case BusinessType::HOTEL:
            score += 25;
            break;
        case BusinessType::WAREHOUSE:
        case BusinessType::MANUFACTURING:
            score += 15;
            break;
        case BusinessType::COWORKING_SPACE:
            score += 18;
            break;
        case BusinessType::LAW_FIRM:
        case BusinessType::MEDICAL_FACILITY:
            score += 15;
            break;
        case BusinessType::EDUCATIONAL_INSTITUTION:
        case BusinessType::GOVERNMENT_OFFICE:
            score += 12;
            break;
        default:
            score += 5;
            break;
    }

    // Facility features scoring
    if (hasConferenceRoom) score += 15;
    if (hasEventSpace) score += 15;
    if (regularMeetings) score += 10;

    // Rating scoring
    if (googleRating >= 4.5) score += 5;
    if (bbbAccredited) score += 5;

    // Revenue indicator
    if (annualRevenue > 10000000) score += 5;

    // Cap at 100
    cateringPotentialScore = std::min(score, 100);
}

} // namespace Models
} // namespace FranchiseAI
