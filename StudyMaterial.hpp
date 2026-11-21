#ifndef STUDY_MATERIAL_HPP
#define STUDY_MATERIAL_HPP

#include <string>
#include <utility>
#include <algorithm>
#include "nlohmann/json.hpp"

class StudyMaterial {
private:
    std::string id;
    std::string title;
    std::string description;
    std::string subjectId;
    std::string topicId;
    std::string originalFilename;
    std::string storedFilename;
    std::string fileType{"PDF"};
    std::string filePath;
    long long fileSize{0};      // in bytes
    std::string uploadedAt;     // ISO timestamp or formatted date

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\n\r");
        return s.substr(start, end - start + 1);
    }

public:
    StudyMaterial() = default;

    StudyMaterial(std::string id,
                  std::string title,
                  std::string description,
                  std::string subjectId,
                  std::string topicId,
                  std::string originalFilename,
                  std::string storedFilename,
                  std::string filePath,
                  long long fileSize,
                  std::string uploadedAt,
                  std::string fileType = "PDF")
        : id(std::move(id)),
          title(trim(title)),
          description(trim(description)),
          subjectId(trim(subjectId)),
          topicId(trim(topicId)),
          originalFilename(trim(originalFilename)),
          storedFilename(trim(storedFilename)),
          fileType(fileType.empty() ? "PDF" : std::move(fileType)),
          filePath(std::move(filePath)),
          fileSize(fileSize),
          uploadedAt(std::move(uploadedAt)) {}

    // Getters
    const std::string& getId() const { return id; }
    const std::string& getTitle() const { return title; }
    const std::string& getDescription() const { return description; }
    const std::string& getSubjectId() const { return subjectId; }
    const std::string& getTopicId() const { return topicId; }
    const std::string& getOriginalFilename() const { return originalFilename; }
    const std::string& getStoredFilename() const { return storedFilename; }
    const std::string& getFileType() const { return fileType; }
    const std::string& getFilePath() const { return filePath; }
    long long getFileSize() const { return fileSize; }
    const std::string& getUploadedAt() const { return uploadedAt; }
    // Backward-compatibility alias
    const std::string& getUploadDate() const { return uploadedAt; }

    // Setters
    void setId(const std::string& newId) { id = trim(newId); }
    void setTitle(const std::string& newTitle) { title = trim(newTitle); }
    void setDescription(const std::string& newDesc) { description = trim(newDesc); }
    void setSubjectId(const std::string& newSubId) { subjectId = trim(newSubId); }
    void setTopicId(const std::string& newTopicId) { topicId = trim(newTopicId); }
    void setOriginalFilename(const std::string& origName) { originalFilename = trim(origName); }
    void setStoredFilename(const std::string& stored) { storedFilename = trim(stored); }
    void setFileType(const std::string& newType) { fileType = newType.empty() ? "PDF" : newType; }
    void setFilePath(const std::string& newPath) { filePath = newPath; }
    void setFileSize(long long newSize) { fileSize = newSize; }
    void setUploadedAt(const std::string& newDate) { uploadedAt = newDate; }
    void setUploadDate(const std::string& newDate) { uploadedAt = newDate; }

    // Validation
    bool isValid(std::string& errorMsg) const {
        if (title.empty()) {
            errorMsg = "Material title cannot be empty.";
            return false;
        }
        if (title.length() > 200) {
            errorMsg = "Material title exceeds maximum length of 200 characters.";
            return false;
        }
        if (subjectId.empty()) {
            errorMsg = "Subject association is required.";
            return false;
        }
        if (originalFilename.empty()) {
            errorMsg = "Original filename cannot be empty.";
            return false;
        }
        if (storedFilename.empty()) {
            errorMsg = "Stored filename cannot be empty.";
            return false;
        }
        // Safety against directory traversal in filenames
        if (storedFilename.find('/') != std::string::npos ||
            storedFilename.find('\\') != std::string::npos ||
            storedFilename.find("..") != std::string::npos) {
            errorMsg = "Stored filename contains invalid or unsafe characters.";
            return false;
        }
        if (fileSize <= 0) {
            errorMsg = "File size must be greater than 0 bytes.";
            return false;
        }
        return true;
    }

    // JSON serialization
    nlohmann::json toJson() const {
        return nlohmann::json{
            {"id", id},
            {"title", title},
            {"description", description},
            {"subjectId", subjectId},
            {"topicId", topicId},
            {"originalFilename", originalFilename},
            {"storedFilename", storedFilename},
            {"fileType", fileType},
            {"filePath", filePath},
            {"fileSize", fileSize},
            {"uploadedAt", uploadedAt},
            {"uploadDate", uploadedAt} // backward-compatibility alias
        };
    }

    static StudyMaterial fromJson(const nlohmann::json& j) {
        StudyMaterial m;
        if (j.contains("id") && !j["id"].is_null()) m.id = j["id"].get<std::string>();
        if (j.contains("title") && !j["title"].is_null()) m.title = j["title"].get<std::string>();
        if (j.contains("description") && !j["description"].is_null()) m.description = j["description"].get<std::string>();
        if (j.contains("subjectId") && !j["subjectId"].is_null()) m.subjectId = j["subjectId"].get<std::string>();
        if (j.contains("topicId") && !j["topicId"].is_null()) m.topicId = j["topicId"].get<std::string>();
        if (j.contains("originalFilename") && !j["originalFilename"].is_null()) m.originalFilename = j["originalFilename"].get<std::string>();
        if (j.contains("storedFilename") && !j["storedFilename"].is_null()) m.storedFilename = j["storedFilename"].get<std::string>();
        if (j.contains("fileType") && !j["fileType"].is_null()) m.fileType = j["fileType"].get<std::string>();
        if (j.contains("filePath") && !j["filePath"].is_null()) m.filePath = j["filePath"].get<std::string>();
        if (j.contains("fileSize") && !j["fileSize"].is_null()) m.fileSize = j["fileSize"].get<long long>();

        if (j.contains("uploadedAt") && !j["uploadedAt"].is_null()) {
            m.uploadedAt = j["uploadedAt"].get<std::string>();
        } else if (j.contains("uploadDate") && !j["uploadDate"].is_null()) {
            m.uploadedAt = j["uploadDate"].get<std::string>();
        }
        return m;
    }
};

#endif // STUDY_MATERIAL_HPP
