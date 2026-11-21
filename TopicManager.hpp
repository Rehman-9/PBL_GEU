#ifndef TOPIC_MANAGER_HPP
#define TOPIC_MANAGER_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <chrono>
#include <random>
#include <optional>
#include <iostream>
#include <unordered_map>
#include "models/Topic.hpp"
#include "services/SubjectManager.hpp"
#include "nlohmann/json.hpp"

class TopicManager {
private:
    std::vector<Topic> topics;
    std::unordered_map<std::string, size_t> idToIndex;
    std::string filePath;

    void rebuildIndex() {
        idToIndex.clear();
        for (size_t i = 0; i < topics.size(); ++i) {
            idToIndex[topics[i].getId()] = i;
        }
    }

    std::string generateId() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        static std::mt19937 rng(4242);
        std::uniform_int_distribution<int> dist(1000, 9999);
        return "top_" + std::to_string(ms) + "_" + std::to_string(dist(rng));
    }

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    }

public:
    explicit TopicManager(std::string filePath = "backend/data/topics.json")
        : filePath(std::move(filePath)) {}

    bool loadFromFile() {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cout << "[TopicManager] No existing data file found at " << filePath << ". Starting fresh." << std::endl;
            return false;
        }

        try {
            nlohmann::json j;
            file >> j;
            if (!j.is_array()) {
                std::cerr << "[TopicManager] Expected an array in " << filePath << std::endl;
                return false;
            }
            topics.clear();
            for (const auto& item : j) {
                Topic t = Topic::fromJson(item);
                topics.push_back(t);
            }
            rebuildIndex();
            std::cout << "[TopicManager] Loaded " << topics.size() << " topics from " << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[TopicManager] Error parsing " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }

    bool saveToFile() const {
        try {
            nlohmann::json j = nlohmann::json::array();
            for (const auto& t : topics) {
                j.push_back(t.toJson());
            }

            std::ofstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "[TopicManager] Failed to open " << filePath << " for writing." << std::endl;
                return false;
            }
            file << j.dump(2) << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[TopicManager] Error saving to " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }

    std::vector<Topic> getAllTopics(const std::string& subjectIdFilter = "") const {
        if (subjectIdFilter.empty()) {
            return topics;
        }

        std::vector<Topic> filtered;
        for (const auto& t : topics) {
            if (t.getSubjectId() == subjectIdFilter) {
                filtered.push_back(t);
            }
        }
        return filtered;
    }

    std::optional<Topic> getTopicById(const std::string& id) const {
        auto it = idToIndex.find(id);
        if (it != idToIndex.end() && it->second < topics.size()) {
            return topics[it->second];
        }
        return std::nullopt;
    }

    bool hasTopicsForSubject(const std::string& subjectId) const {
        for (const auto& t : topics) {
            if (t.getSubjectId() == subjectId) {
                return true;
            }
        }
        return false;
    }

    int countTopicsForSubject(const std::string& subjectId) const {
        int count = 0;
        for (const auto& t : topics) {
            if (t.getSubjectId() == subjectId) {
                count++;
            }
        }
        return count;
    }

    std::optional<Topic> addTopic(const nlohmann::json& payload,
                                  const SubjectManager& subjectManager,
                                  std::string& errorMsg) {
        // Validation: Name
        if (!payload.contains("name") || payload["name"].is_null() || !payload["name"].is_string()) {
            errorMsg = "Topic name is required and must be a string.";
            return std::nullopt;
        }
        std::string name = trim(payload["name"].get<std::string>());
        if (name.empty()) {
            errorMsg = "Topic name cannot be empty.";
            return std::nullopt;
        }

        // Validation: SubjectId
        if (!payload.contains("subjectId") || payload["subjectId"].is_null() || !payload["subjectId"].is_string()) {
            errorMsg = "Valid subjectId is required.";
            return std::nullopt;
        }
        std::string subjectId = payload["subjectId"].get<std::string>();
        if (!subjectManager.hasSubject(subjectId)) {
            errorMsg = "Referenced subject does not exist (invalid subjectId).";
            return std::nullopt;
        }

        // Validation: Difficulty (1-5)
        int difficulty = 3;
        if (payload.contains("difficulty") && !payload["difficulty"].is_null()) {
            if (!payload["difficulty"].is_number_integer()) {
                errorMsg = "Difficulty must be an integer between 1 and 5.";
                return std::nullopt;
            }
            difficulty = payload["difficulty"].get<int>();
            if (difficulty < 1 || difficulty > 5) {
                errorMsg = "Difficulty must be between 1 (Very Easy) and 5 (Very Hard).";
                return std::nullopt;
            }
        }

        // Validation: Topic must NOT contain examDate (Exam belongs to Subject)
        if (payload.contains("examDate") && !payload["examDate"].is_null()) {
            errorMsg = "Topic must not contain examDate. An exam belongs to a Subject, not an individual Topic.";
            return std::nullopt;
        }

        // Validation: SyllabusRemaining (0-100)
        double syllabusRemaining = 100.0;
        if (payload.contains("syllabusRemaining") && !payload["syllabusRemaining"].is_null()) {
            if (!payload["syllabusRemaining"].is_number()) {
                errorMsg = "Syllabus remaining must be a number between 0 and 100.";
                return std::nullopt;
            }
            syllabusRemaining = payload["syllabusRemaining"].get<double>();
            if (syllabusRemaining < 0.0 || syllabusRemaining > 100.0) {
                errorMsg = "Syllabus remaining percentage must be between 0 and 100.";
                return std::nullopt;
            }
        }

        // Validation: AcademicImportance (1-5)
        int academicImportance = 3;
        if (payload.contains("academicImportance") && !payload["academicImportance"].is_null()) {
            if (!payload["academicImportance"].is_number_integer()) {
                errorMsg = "Academic importance must be an integer between 1 and 5.";
                return std::nullopt;
            }
            academicImportance = payload["academicImportance"].get<int>();
            if (academicImportance < 1 || academicImportance > 5) {
                errorMsg = "Academic importance must be between 1 and 5.";
                return std::nullopt;
            }
        }

        // Validation: EstimatedHours (> 0)
        double estimatedHours = 2.0;
        if (payload.contains("estimatedHours") && !payload["estimatedHours"].is_null()) {
            if (!payload["estimatedHours"].is_number()) {
                errorMsg = "Estimated study hours must be a number greater than 0.";
                return std::nullopt;
            }
            estimatedHours = payload["estimatedHours"].get<double>();
            if (estimatedHours <= 0.0) {
                errorMsg = "Estimated study hours must be strictly greater than 0.";
                return std::nullopt;
            }
        }

        // Validation: Status
        std::string status = "NOT_STARTED";
        if (payload.contains("status") && !payload["status"].is_null() && payload["status"].is_string()) {
            std::string st = payload["status"].get<std::string>();
            if (st == "NOT_STARTED" || st == "IN_PROGRESS" || st == "COMPLETED") {
                status = st;
            } else {
                errorMsg = "Status must be 'NOT_STARTED', 'IN_PROGRESS', or 'COMPLETED'.";
                return std::nullopt;
            }
        }

        std::string newId = generateId();
        // Newly-created topics start at 0.0; API recalculates after persistence.
        double initialPriority = 0.0;

        Topic newTopic(newId, subjectId, name, difficulty,
                       syllabusRemaining, academicImportance, estimatedHours,
                       status, initialPriority);

        topics.push_back(newTopic);
        rebuildIndex();

        saveToFile();
        return newTopic;
    }

    std::optional<Topic> updateTopic(const std::string& id,
                                     const nlohmann::json& payload,
                                     const SubjectManager& subjectManager,
                                     std::string& errorMsg) {
        auto it = idToIndex.find(id);
        if (it == idToIndex.end()) {
            errorMsg = "Topic not found.";
            return std::nullopt;
        }

        Topic& topic = topics[it->second];

        // Name
        if (payload.contains("name") && !payload["name"].is_null()) {
            if (!payload["name"].is_string()) {
                errorMsg = "Topic name must be a string.";
                return std::nullopt;
            }
            std::string name = trim(payload["name"].get<std::string>());
            if (name.empty()) {
                errorMsg = "Topic name cannot be empty.";
                return std::nullopt;
            }
            topic.setName(name);
        }

        // SubjectId
        if (payload.contains("subjectId") && !payload["subjectId"].is_null()) {
            std::string subId = payload["subjectId"].get<std::string>();
            if (!subjectManager.hasSubject(subId)) {
                errorMsg = "Referenced subject does not exist (invalid subjectId).";
                return std::nullopt;
            }
            topic.setSubjectId(subId);
        }

        // Difficulty
        if (payload.contains("difficulty") && !payload["difficulty"].is_null()) {
            if (!payload["difficulty"].is_number_integer()) {
                errorMsg = "Difficulty must be an integer between 1 and 5.";
                return std::nullopt;
            }
            int diff = payload["difficulty"].get<int>();
            if (!topic.setDifficulty(diff)) {
                errorMsg = "Difficulty must be between 1 and 5.";
                return std::nullopt;
            }
        }

        // Topic must NOT contain examDate
        if (payload.contains("examDate") && !payload["examDate"].is_null()) {
            errorMsg = "Topic must not contain examDate. An exam belongs to a Subject, not an individual Topic.";
            return std::nullopt;
        }

        // SyllabusRemaining
        if (payload.contains("syllabusRemaining") && !payload["syllabusRemaining"].is_null()) {
            if (!payload["syllabusRemaining"].is_number()) {
                errorMsg = "Syllabus remaining must be a valid number.";
                return std::nullopt;
            }
            double rem = payload["syllabusRemaining"].get<double>();
            if (!topic.setSyllabusRemaining(rem)) {
                errorMsg = "Syllabus remaining percentage must be between 0 and 100.";
                return std::nullopt;
            }
        }

        // AcademicImportance
        if (payload.contains("academicImportance") && !payload["academicImportance"].is_null()) {
            if (!payload["academicImportance"].is_number_integer()) {
                errorMsg = "Academic importance must be an integer between 1 and 5.";
                return std::nullopt;
            }
            int imp = payload["academicImportance"].get<int>();
            if (!topic.setAcademicImportance(imp)) {
                errorMsg = "Academic importance must be between 1 and 5.";
                return std::nullopt;
            }
        }

        // EstimatedHours
        if (payload.contains("estimatedHours") && !payload["estimatedHours"].is_null()) {
            if (!payload["estimatedHours"].is_number()) {
                errorMsg = "Estimated study hours must be a number.";
                return std::nullopt;
            }
            double hours = payload["estimatedHours"].get<double>();
            if (!topic.setEstimatedHours(hours)) {
                errorMsg = "Estimated study hours must be greater than 0.";
                return std::nullopt;
            }
        }

        // Status
        if (payload.contains("status") && !payload["status"].is_null()) {
            std::string st = payload["status"].get<std::string>();
            if (!topic.setStatus(st)) {
                errorMsg = "Status must be 'NOT_STARTED', 'IN_PROGRESS', or 'COMPLETED'.";
                return std::nullopt;
            }
        }

        // Stage 2: Priority remains 0
        topic.setPriority(0.0);

        saveToFile();
        return topic;
    }

    bool deleteTopic(const std::string& id, std::string& errorMsg) {
        auto it = idToIndex.find(id);
        if (it == idToIndex.end()) {
            errorMsg = "Topic not found.";
            return false;
        }

        size_t index = it->second;
        topics.erase(topics.begin() + index);
        rebuildIndex();

        saveToFile();
        return true;
    }

    std::optional<Topic> updateTopicStatus(const std::string& id,
                                           const std::string& newStatus,
                                           std::string& errorMsg) {
        auto it = idToIndex.find(id);
        if (it == idToIndex.end()) {
            errorMsg = "Topic not found.";
            return std::nullopt;
        }

        Topic& topic = topics[it->second];
        if (!topic.setStatus(newStatus)) {
            errorMsg = "Status must be 'NOT_STARTED', 'IN_PROGRESS', or 'COMPLETED'.";
            return std::nullopt;
        }

        if (newStatus == "COMPLETED") {
            topic.setSyllabusRemaining(0.0);
        }

        saveToFile();
        return topic;
    }

    size_t count() const {
        return topics.size();
    }

    bool updatePriorities(const std::unordered_map<std::string, double>& priorities) {
        for (auto& topic : topics) {
            const auto found = priorities.find(topic.getId());
            topic.setPriority(found == priorities.end() ? 0.0 : found->second);
        }
        return saveToFile();
    }
};

#endif // TOPIC_MANAGER_HPP
