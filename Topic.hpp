#ifndef TOPIC_HPP
#define TOPIC_HPP

#include <string>
#include <algorithm>
#include <utility>
#include "nlohmann/json.hpp"

class Topic {
private:
    std::string id;
    std::string subjectId;
    std::string name;
    int difficulty{3};                     // 1 to 5
    double syllabusRemaining{100.0};       // 0 to 100
    int academicImportance{3};             // 1 to 5
    double estimatedHours{2.0};            // > 0
    std::string status{"NOT_STARTED"};     // "NOT_STARTED", "IN_PROGRESS", "COMPLETED"
    double priority{0.0};                  // Recalculated recommendation score, 0.0 to 100.0

public:
    Topic() = default;

    Topic(std::string id, std::string subjectId, std::string name,
          int difficulty, double syllabusRemaining,
          int academicImportance, double estimatedHours,
          std::string status = "NOT_STARTED", double priority = 0.0)
        : id(std::move(id)), subjectId(std::move(subjectId)), name(std::move(name)),
          status(std::move(status)), priority(priority) {
        setDifficulty(difficulty);
        setSyllabusRemaining(syllabusRemaining);
        setAcademicImportance(academicImportance);
        setEstimatedHours(estimatedHours);
    }

    // Getters
    const std::string& getId() const { return id; }
    const std::string& getSubjectId() const { return subjectId; }
    const std::string& getName() const { return name; }
    int getDifficulty() const { return difficulty; }
    double getSyllabusRemaining() const { return syllabusRemaining; }
    int getAcademicImportance() const { return academicImportance; }
    double getEstimatedHours() const { return estimatedHours; }
    const std::string& getStatus() const { return status; }
    double getPriority() const { return priority; }

    // Setters with Encapsulation & Validation
    void setId(const std::string& newId) { id = newId; }
    void setSubjectId(const std::string& newSubId) { subjectId = newSubId; }
    void setName(const std::string& newName) { name = newName; }

    bool setDifficulty(int diff) {
        if (diff < 1 || diff > 5) return false;
        difficulty = diff;
        return true;
    }

    bool setSyllabusRemaining(double val) {
        if (val < 0.0 || val > 100.0) return false;
        syllabusRemaining = val;
        return true;
    }

    bool setAcademicImportance(int imp) {
        if (imp < 1 || imp > 5) return false;
        academicImportance = imp;
        return true;
    }

    bool setEstimatedHours(double hours) {
        if (hours <= 0.0) return false;
        estimatedHours = hours;
        return true;
    }

    bool setStatus(const std::string& st) {
        if (st == "NOT_STARTED" || st == "IN_PROGRESS" || st == "COMPLETED") {
            status = st;
            return true;
        }
        return false;
    }

    void setPriority(double p) {
        priority = p;
    }

    // JSON serialization (Note: examDate is NOT included in Topic)
    nlohmann::json toJson() const {
        return nlohmann::json{
            {"id", id},
            {"subjectId", subjectId},
            {"name", name},
            {"difficulty", difficulty},
            {"syllabusRemaining", syllabusRemaining},
            {"academicImportance", academicImportance},
            {"estimatedHours", estimatedHours},
            {"status", status},
            {"priority", priority}
        };
    }

    static Topic fromJson(const nlohmann::json& j) {
        Topic t;
        if (j.contains("id") && !j["id"].is_null()) t.id = j["id"].get<std::string>();
        if (j.contains("subjectId") && !j["subjectId"].is_null()) t.subjectId = j["subjectId"].get<std::string>();
        if (j.contains("name") && !j["name"].is_null()) t.name = j["name"].get<std::string>();
        if (j.contains("difficulty") && !j["difficulty"].is_null()) t.setDifficulty(j["difficulty"].get<int>());
        
        if (j.contains("syllabusRemaining") && !j["syllabusRemaining"].is_null())
            t.setSyllabusRemaining(j["syllabusRemaining"].get<double>());
        else if (j.contains("syllabusRemainingPct") && !j["syllabusRemainingPct"].is_null())
            t.setSyllabusRemaining(j["syllabusRemainingPct"].get<double>());

        if (j.contains("academicImportance") && !j["academicImportance"].is_null())
            t.setAcademicImportance(j["academicImportance"].get<int>());
        if (j.contains("estimatedHours") && !j["estimatedHours"].is_null())
            t.setEstimatedHours(j["estimatedHours"].get<double>());
        if (j.contains("status") && !j["status"].is_null()) t.setStatus(j["status"].get<std::string>());
        
        if (j.contains("priority") && !j["priority"].is_null())
            t.setPriority(j["priority"].get<double>());
        else
            t.setPriority(0.0);

        return t;
    }
};

#endif // TOPIC_HPP
