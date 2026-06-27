#pragma once
#include <QString>
#include <optional>

struct ValidationResult {
    bool isValid = false;
    QString errorMessage;

    ValidationResult() = default;
    ValidationResult(bool valid, const QString &msg = "") : isValid(valid), errorMessage(msg) {}
};

// ─── Input validation utilities ────────────────────────────────────────────────
class InputValidator
{
public:
    // Validate spending entry title
    static ValidationResult validateTitle(const QString &title);

    // Validate spending amount
    static ValidationResult validateAmount(const QString &amountStr);

    // Validate goal amount
    static ValidationResult validateGoalAmount(const QString &amountStr);

    // Validate range count (for goals)
    static ValidationResult validateRangeCount(const QString &countStr);

private:
    InputValidator() = default;
    
    // Helper: check if string is empty or only whitespace
    static bool isEmpty(const QString &str);
    
    // Helper: try to parse as positive double
    static std::optional<double> parsePositiveDouble(const QString &str);
    
    // Helper: try to parse as positive integer
    static std::optional<int> parsePositiveInt(const QString &str);
};
