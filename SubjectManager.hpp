#ifndef SUBJECT_MANAGER_HPP
#define SUBJECT_MANAGER_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <chrono>
#include <random>
#include <optional>
#include <functional>
#include <iostream>
#include "models/Subject.hpp"
#include "nlohmann/json.hpp"

class SubjectManager {
private:
    std::vector<Subject> subjects;
    std::unordered_map<std::string, size_t> idToIndex;
    std::string filePath;

    void rebuildIndex() {
        idToIndex.clear();
        for (size_t i = 0; i < subjects.size(); ++i) {
            idToIndex[subjects[i].getId()] = i;
        }
    }

    std::string generateId() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        static std::mt19937 rng(1337);
        std::uniform_int_distribution<int> dist(1000, 9999);
        return "sub_" + std::to_string(ms) + "_" + std::to_string(dist(rng));
    }

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    }

public:
    explicit SubjectManager(std::string filePath = "backend/data/subjects.json")
        : filePath(std::move(filePath)) {}

    bool loadFromFile() {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cout << "[SubjectManager] No existing data file found at " << filePath << ". Starting fresh." << std::endl;
            return false;
        }

        try {
            nlohmann::json j;
            file >> j;
            if (!j.is_array()) {
                std::cerr << "[SubjectManager] Expected an array in " << filePath << std::endl;
                return false;
            }
            subjects.clear();
            for (const auto& item : j) {
                subjects.push_back(Subject::fromJson(item));
            }
            rebuildIndex();
            std::cout << "[SubjectManager] Loaded " << subjects.size() << " subjects from " << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[SubjectManager] Error parsing " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }

    bool saveToFile() const {
        try {
            nlohmann::json j = nlohmann::json::array();
            for (const auto& s : subjects) {
                j.push_back(s.toJson());
            }

            std::ofstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "[SubjectManager] Failed to open " << filePath << " for writing." << std::endl;
                return false;
            }
            file << j.dump(2) << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[SubjectManager] Error saving to " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }

    const std::vector<Subject>& getAllSubjects() const {
        return subjects;
    }

    std::optional<Subject> getSubjectById(const std::string& id) const {
        auto it = idToIndex.find(id);
        if (it != idToIndex.end() && it->second < subjects.size()) {
            return subjects[it->second];
        }
        return std::nullopt;
    }

    bool hasSubject(const std::string& id) const {
        return idToIndex.find(id) != idToIndex.end();
    }

    std::optional<Subject> addSubject(const std::string& name, const std::string& code,
                                      const std::string& semester, const std::string& examDate,
                                      std::string& errorMsg) {
        std::string trimmedName = trim(name);
        if (trimmedName.empty()) {
            errorMsg = "Subject name is required.";
            return std::nullopt;
        }

        // Exam Date validation: required and valid Gregorian date YYYY-MM-DD
        std::string trimmedDate = trim(examDate);
        if (trimmedDate.empty()) {
            errorMsg = "Exam date is required for each subject (format: YYYY-MM-DD).";
            return std::nullopt;
        }
        if (!Subject::isValidDate(trimmedDate)) {
            errorMsg = "Invalid subject exam date. Expected valid Gregorian date format YYYY-MM-DD (e.g. 2026-11-20).";
            return std::nullopt;
        }

        // Check for duplicate subject name
        for (const auto& s : subjects) {
            if (s.getName() == trimmedName) {
                errorMsg = "A subject with this name already exists.";
                return std::nullopt;
            }
        }

        std::string newId = generateId();
        Subject newSubject(newId, trimmedName, trim(code), trim(semester), trimmedDate);
        subjects.push_back(newSubject);
        rebuildIndex();

        saveToFile();
        return newSubject;
    }

    std::optional<Subject> updateSubject(const std::string& id,
                                         const nlohmann::json& payload,
                                         std::string& errorMsg) {
        auto it = idToIndex.find(id);
        if (it == idToIndex.end()) {
            errorMsg = "Subject not found.";
            return std::nullopt;
        }

        Subject& sub = subjects[it->second];

        if (payload.contains("name") && !payload["name"].is_null()) {
            std::string newName = trim(payload["name"].get<std::string>());
            if (newName.empty()) {
                errorMsg = "Subject name cannot be empty.";
                return std::nullopt;
            }
            sub.setName(newName);
        }

        if (payload.contains("code") && !payload["code"].is_null()) {
            sub.setCode(trim(payload["code"].get<std::string>()));
        }

        if (payload.contains("semester") && !payload["semester"].is_null()) {
            sub.setSemester(trim(payload["semester"].get<std::string>()));
        }

        if (payload.contains("examDate") && !payload["examDate"].is_null()) {
            if (!payload["examDate"].is_string()) {
                errorMsg = "Subject exam date must be a string in YYYY-MM-DD format.";
                return std::nullopt;
            }
            std::string newDate = trim(payload["examDate"].get<std::string>());
            if (!sub.setExamDate(newDate)) {
                errorMsg = "Invalid subject exam date format. Expected valid YYYY-MM-DD date.";
                return std::nullopt;
            }
        }

        saveToFile();
        return sub;
    }

    bool deleteSubject(const std::string& id,
                       const std::function<bool(const std::string&)>& hasTopicsPredicate,
                       std::string& errorMsg) {
        auto it = idToIndex.find(id);
        if (it == idToIndex.end()) {
            errorMsg = "Subject not found.";
            return false;
        }

        // Safety check: Prevent deletion if topics exist for this subject
        if (hasTopicsPredicate && hasTopicsPredicate(id)) {
            errorMsg = "Cannot delete subject: it still has associated topics. Please delete its topics first.";
            return false;
        }

        size_t index = it->second;
        subjects.erase(subjects.begin() + index);
        rebuildIndex();

        saveToFile();
        return true;
    }

    size_t count() const {
        return subjects.size();
    }
};

#endif // SUBJECT_MANAGER_HPP
