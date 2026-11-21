#ifndef SUBJECT_HPP
#define SUBJECT_HPP

#include <string>
#include <utility>
#include <cctype>
#include "nlohmann/json.hpp"

class Subject {
private:
    std::string id;
    std::string name;
    std::string code;
    std::string semester;
    std::string examDate; // Required: YYYY-MM-DD valid Gregorian date

public:
    Subject() = default;

    Subject(std::string id, std::string name, std::string code, std::string semester, std::string examDate)
        : id(std::move(id)), name(std::move(name)), code(std::move(code)),
          semester(std::move(semester)), examDate(std::move(examDate)) {}

    // Getters
    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getCode() const { return code; }
    const std::string& getSemester() const { return semester; }
    const std::string& getExamDate() const { return examDate; }

    // Setters with validation
    void setId(const std::string& newId) { id = newId; }
    void setName(const std::string& newName) { name = newName; }
    void setCode(const std::string& newCode) { code = newCode; }
    void setSemester(const std::string& newSemester) { semester = newSemester; }

    bool setExamDate(const std::string& date) {
        if (!isValidDate(date)) return false;
        examDate = date;
        return true;
    }

    // Static Gregorian date validator: YYYY-MM-DD
    static bool isValidDate(const std::string& dateStr) {
        if (dateStr.length() != 10) return false;
        if (dateStr[4] != '-' || dateStr[7] != '-') return false;

        for (int i = 0; i < 10; ++i) {
            if (i == 4 || i == 7) continue;
            if (!std::isdigit(static_cast<unsigned char>(dateStr[i]))) return false;
        }

        auto parseDigits = [&dateStr](size_t start, size_t length, int& value) {
            value = 0;
            for (size_t i = start; i < start + length; ++i) {
                if (!std::isdigit(static_cast<unsigned char>(dateStr[i]))) return false;
                value = value * 10 + (dateStr[i] - '0');
            }
            return true;
        };
        int year, month, day;
        if (!parseDigits(0, 4, year) || !parseDigits(5, 2, month) || !parseDigits(8, 2, day)) return false;

        if (year < 1900 || year > 2100) return false;
        if (month < 1 || month > 12) return false;

        const int daysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        // Check for Gregorian leap year
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (month == 2 && isLeap) {
            if (day < 1 || day > 29) return false;
        } else {
            if (day < 1 || day > daysInMonth[month]) return false;
        }

        return true;
    }

    // JSON serialization
    nlohmann::json toJson() const {
        return nlohmann::json{
            {"id", id},
            {"name", name},
            {"code", code},
            {"semester", semester},
            {"examDate", examDate}
        };
    }

    static Subject fromJson(const nlohmann::json& j) {
        Subject s;
        if (j.contains("id") && !j["id"].is_null()) s.id = j["id"].get<std::string>();
        if (j.contains("name") && !j["name"].is_null()) s.name = j["name"].get<std::string>();
        if (j.contains("code") && !j["code"].is_null()) s.code = j["code"].get<std::string>();
        if (j.contains("semester") && !j["semester"].is_null()) s.semester = j["semester"].get<std::string>();
        if (j.contains("examDate") && j["examDate"].is_string()) {
            const std::string date = j["examDate"].get<std::string>();
            if (isValidDate(date)) s.examDate = date;
        }
        return s;
    }
};

#endif // SUBJECT_HPP
