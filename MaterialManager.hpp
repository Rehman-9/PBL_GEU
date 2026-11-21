#ifndef MATERIAL_MANAGER_HPP
#define MATERIAL_MANAGER_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <chrono>
#include <random>
#include <optional>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <filesystem>
#include "models/StudyMaterial.hpp"
#include "services/SubjectManager.hpp"
#include "services/TopicManager.hpp"
#include "nlohmann/json.hpp"

class MaterialManager {
private:
    std::vector<StudyMaterial> materials;
    std::unordered_map<std::string, size_t> idToIndex;
    std::string filePath;

    void rebuildIndex() {
        idToIndex.clear();
        for (size_t i = 0; i < materials.size(); ++i) {
            idToIndex[materials[i].getId()] = i;
        }
    }

    std::string generateId() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        static std::mt19937 rng(8888);
        std::uniform_int_distribution<int> dist(1000, 9999);
        return "mat_" + std::to_string(ms) + "_" + std::to_string(dist(rng));
    }

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return s;
    }

public:
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        gmtime_s(&tm, &tt);
#else
        gmtime_r(&tt, &tm);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", &tm);
        return std::string(buf);
    }

    explicit MaterialManager(std::string filePath = "backend/data/materials.json")
        : filePath(std::move(filePath)) {}

    bool loadFromFile() {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cout << "[MaterialManager] No existing data file found at " << filePath 
                      << ". Initializing empty collection." << std::endl;
            // Create parent directories if needed
            std::filesystem::path p(filePath);
            if (p.has_parent_path()) {
                std::error_code ec;
                std::filesystem::create_directories(p.parent_path(), ec);
            }
            materials.clear();
            rebuildIndex();
            return saveToFile();
        }

        try {
            nlohmann::json j;
            file >> j;
            if (!j.is_array()) {
                std::cerr << "[MaterialManager] Expected a JSON array in " << filePath << std::endl;
                return false;
            }
            materials.clear();
            for (const auto& item : j) {
                StudyMaterial m = StudyMaterial::fromJson(item);
                materials.push_back(m);
            }
            rebuildIndex();
            std::cout << "[MaterialManager] Loaded " << materials.size() << " study materials from " 
                      << filePath << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[MaterialManager] Error parsing " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }

    bool saveToFile() const {
        try {
            nlohmann::json j = nlohmann::json::array();
            for (const auto& m : materials) {
                j.push_back(m.toJson());
            }

            std::filesystem::path p(filePath);
            if (p.has_parent_path()) {
                std::error_code ec;
                std::filesystem::create_directories(p.parent_path(), ec);
            }

            std::ofstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "[MaterialManager] Failed to open " << filePath << " for writing." << std::endl;
                return false;
            }
            file << j.dump(2) << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[MaterialManager] Error saving to " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }

    size_t count() const {
        return materials.size();
    }

    const std::vector<StudyMaterial>& getAllMaterials() const {
        return materials;
    }

    std::vector<StudyMaterial> getFilteredMaterials(const std::string& subjectId = "",
                                                   const std::string& topicId = "",
                                                   const std::string& query = "") const {
        std::string lowerQ = toLower(trim(query));
        std::vector<StudyMaterial> result;
        result.reserve(materials.size());

        for (const auto& m : materials) {
            if (!subjectId.empty() && m.getSubjectId() != subjectId) {
                continue;
            }
            if (!topicId.empty() && m.getTopicId() != topicId) {
                continue;
            }
            if (!lowerQ.empty()) {
                std::string lowerTitle = toLower(m.getTitle());
                std::string lowerDesc = toLower(m.getDescription());
                if (lowerTitle.find(lowerQ) == std::string::npos &&
                    lowerDesc.find(lowerQ) == std::string::npos) {
                    continue;
                }
            }
            result.push_back(m);
        }
        return result;
    }

    std::optional<StudyMaterial> getMaterialById(const std::string& id) const {
        auto it = idToIndex.find(id);
        if (it != idToIndex.end()) {
            return materials[it->second];
        }
        return std::nullopt;
    }

    std::vector<StudyMaterial> getMaterialsBySubject(const std::string& subjectId) const {
        return getFilteredMaterials(subjectId, "", "");
    }

    std::vector<StudyMaterial> getMaterialsByTopic(const std::string& topicId) const {
        return getFilteredMaterials("", topicId, "");
    }

    bool hasMaterialsForSubject(const std::string& subjectId) const {
        for (const auto& m : materials) {
            if (m.getSubjectId() == subjectId) return true;
        }
        return false;
    }

    bool hasMaterialsForTopic(const std::string& topicId) const {
        for (const auto& m : materials) {
            if (m.getTopicId() == topicId) return true;
        }
        return false;
    }

    // Add new material
    std::optional<StudyMaterial> addMaterial(
        const std::string& title,
        const std::string& description,
        const std::string& subjectId,
        const std::string& topicId,
        const std::string& originalFilename,
        const std::string& storedFilename,
        const std::string& filePath,
        long long fileSize,
        const SubjectManager& subjectManager,
        const TopicManager& topicManager,
        std::string& errorMsg
    ) {
        std::string trimmedTitle = trim(title);
        std::string trimmedDesc = trim(description);
        std::string trimmedSubId = trim(subjectId);
        std::string trimmedTopicId = trim(topicId);
        std::string trimmedOrigName = trim(originalFilename);
        std::string trimmedStoredName = trim(storedFilename);

        if (trimmedTitle.empty()) {
            errorMsg = "Material title cannot be empty.";
            return std::nullopt;
        }
        if (trimmedSubId.empty()) {
            errorMsg = "Subject association is required.";
            return std::nullopt;
        }
        if (!subjectManager.hasSubject(trimmedSubId)) {
            errorMsg = "Specified subject does not exist.";
            return std::nullopt;
        }
        if (!trimmedTopicId.empty()) {
            auto topicOpt = topicManager.getTopicById(trimmedTopicId);
            if (!topicOpt.has_value()) {
                errorMsg = "Specified topic does not exist.";
                return std::nullopt;
            }
            if (topicOpt->getSubjectId() != trimmedSubId) {
                errorMsg = "Specified topic does not belong to the selected subject.";
                return std::nullopt;
            }
        }

        std::string newId = generateId();
        std::string timestamp = getCurrentTimestamp();

        StudyMaterial mat(
            newId,
            trimmedTitle,
            trimmedDesc,
            trimmedSubId,
            trimmedTopicId,
            trimmedOrigName,
            trimmedStoredName,
            filePath,
            fileSize,
            timestamp,
            "PDF"
        );

        if (!mat.isValid(errorMsg)) {
            return std::nullopt;
        }

        materials.push_back(mat);
        rebuildIndex();

        if (!saveToFile()) {
            materials.pop_back();
            rebuildIndex();
            errorMsg = "Failed to persist study material to JSON storage.";
            return std::nullopt;
        }

        return mat;
    }

    // Update material metadata
    std::optional<StudyMaterial> updateMaterial(
        const std::string& id,
        const nlohmann::json& body,
        const SubjectManager& subjectManager,
        const TopicManager& topicManager,
        std::string& errorMsg
    ) {
        auto it = idToIndex.find(id);
        if (it == idToIndex.end()) {
            errorMsg = "Study material not found.";
            return std::nullopt;
        }

        StudyMaterial& target = materials[it->second];
        StudyMaterial backup = target;

        // Title
        if (body.contains("title")) {
            if (body["title"].is_null() || !body["title"].is_string()) {
                errorMsg = "Title must be a non-empty string.";
                return std::nullopt;
            }
            std::string t = trim(body["title"].get<std::string>());
            if (t.empty()) {
                errorMsg = "Material title cannot be empty.";
                return std::nullopt;
            }
            target.setTitle(t);
        }

        // Description
        if (body.contains("description")) {
            if (body["description"].is_null()) {
                target.setDescription("");
            } else if (body["description"].is_string()) {
                target.setDescription(trim(body["description"].get<std::string>()));
            } else {
                errorMsg = "Description must be a string.";
                return std::nullopt;
            }
        }

        // Subject & Topic relationship
        std::string newSubId = target.getSubjectId();
        if (body.contains("subjectId")) {
            if (body["subjectId"].is_null() || !body["subjectId"].is_string()) {
                errorMsg = "Subject ID must be a non-empty string.";
                return std::nullopt;
            }
            newSubId = trim(body["subjectId"].get<std::string>());
            if (newSubId.empty()) {
                errorMsg = "Subject ID cannot be empty.";
                return std::nullopt;
            }
            if (!subjectManager.hasSubject(newSubId)) {
                errorMsg = "Specified subject does not exist.";
                return std::nullopt;
            }
            target.setSubjectId(newSubId);
        }

        // Topic
        std::string newTopicId = target.getTopicId();
        if (body.contains("topicId")) {
            if (body["topicId"].is_null()) {
                newTopicId = "";
            } else if (body["topicId"].is_string()) {
                newTopicId = trim(body["topicId"].get<std::string>());
            } else {
                errorMsg = "Topic ID must be a string or null.";
                return std::nullopt;
            }
            target.setTopicId(newTopicId);
        }

        // Validate that topic belongs to subject
        if (!target.getTopicId().empty()) {
            auto topicOpt = topicManager.getTopicById(target.getTopicId());
            if (!topicOpt.has_value()) {
                target = backup;
                errorMsg = "Specified topic does not exist.";
                return std::nullopt;
            }
            if (topicOpt->getSubjectId() != target.getSubjectId()) {
                target = backup;
                errorMsg = "Specified topic does not belong to the selected subject.";
                return std::nullopt;
            }
        }

        if (!target.isValid(errorMsg)) {
            target = backup;
            return std::nullopt;
        }

        if (!saveToFile()) {
            target = backup;
            errorMsg = "Failed to persist material updates.";
            return std::nullopt;
        }

        return target;
    }

    // Update material file reference
    bool updateMaterialFile(
        const std::string& id,
        const std::string& originalFilename,
        const std::string& storedFilename,
        const std::string& filePath,
        long long fileSize,
        std::string& errorMsg
    ) {
        auto it = idToIndex.find(id);
        if (it == idToIndex.end()) {
            errorMsg = "Study material not found.";
            return false;
        }

        StudyMaterial& target = materials[it->second];
        StudyMaterial backup = target;

        target.setOriginalFilename(originalFilename);
        target.setStoredFilename(storedFilename);
        target.setFilePath(filePath);
        target.setFileSize(fileSize);
        target.setUploadedAt(getCurrentTimestamp());

        if (!target.isValid(errorMsg)) {
            target = backup;
            return false;
        }

        if (!saveToFile()) {
            target = backup;
            errorMsg = "Failed to persist updated file information.";
            return false;
        }

        return true;
    }

    // Delete material
    std::optional<StudyMaterial> deleteMaterial(const std::string& id, std::string& errorMsg) {
        auto it = idToIndex.find(id);
        if (it == idToIndex.end()) {
            errorMsg = "Study material not found.";
            return std::nullopt;
        }

        size_t index = it->second;
        StudyMaterial deleted = materials[index];

        materials.erase(materials.begin() + index);
        rebuildIndex();

        if (!saveToFile()) {
            // Rollback
            materials.insert(materials.begin() + index, deleted);
            rebuildIndex();
            errorMsg = "Failed to update storage after material deletion.";
            return std::nullopt;
        }

        return deleted;
    }
};

#endif // MATERIAL_MANAGER_HPP
