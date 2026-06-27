#include "InputValidator.h"

bool InputValidator::isEmpty(const QString &str)
{
    return str.trimmed().isEmpty();
}

std::optional<double> InputValidator::parsePositiveDouble(const QString &str)
{
    bool ok;
    double value = str.toDouble(&ok);
    if (!ok || value <= 0) {
        return std::nullopt;
    }
    return value;
}

std::optional<int> InputValidator::parsePositiveInt(const QString &str)
{
    bool ok;
    int value = str.toInt(&ok);
    if (!ok || value <= 0) {
        return std::nullopt;
    }
    return value;
}

ValidationResult InputValidator::validateTitle(const QString &title)
{
    if (isEmpty(title)) {
        return ValidationResult(false, "Title cannot be empty.");
    }

    if (title.length() > 100) {
        return ValidationResult(false, "Title cannot exceed 100 characters.");
    }

    return ValidationResult(true);
}

ValidationResult InputValidator::validateAmount(const QString &amountStr)
{
    if (isEmpty(amountStr)) {
        return ValidationResult(false, "Please enter an amount.");
    }

    auto amount = parsePositiveDouble(amountStr);
    if (!amount.has_value()) {
        return ValidationResult(false, "Amount must be a positive number (e.g., 5.50).");
    }

    if (amount.value() > 999999.99) {
        return ValidationResult(false, "Amount is too large. Maximum is $999,999.99.");
    }

    if (amount.value() < 0.01) {
        return ValidationResult(false, "Amount must be at least $0.01.");
    }

    return ValidationResult(true);
}

ValidationResult InputValidator::validateGoalAmount(const QString &amountStr)
{
    if (isEmpty(amountStr)) {
        return ValidationResult(false, "Please enter a goal amount.");
    }

    auto amount = parsePositiveDouble(amountStr);
    if (!amount.has_value()) {
        return ValidationResult(false, "Goal amount must be a positive number.");
    }

    if (amount.value() > 999999.99) {
        return ValidationResult(false, "Goal amount is too large. Maximum is $999,999.99.");
    }

    return ValidationResult(true);
}

ValidationResult InputValidator::validateRangeCount(const QString &countStr)
{
    if (isEmpty(countStr)) {
        return ValidationResult(false, "Please enter a range count.");
    }

    auto count = parsePositiveInt(countStr);
    if (!count.has_value()) {
        return ValidationResult(false, "Range count must be a positive whole number.");
    }

    if (count.value() > 1000) {
        return ValidationResult(false, "Range count is too large. Maximum is 1000.");
    }

    return ValidationResult(true);
}
