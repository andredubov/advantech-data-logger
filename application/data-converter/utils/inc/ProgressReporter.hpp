#ifndef PROGRESS_REPORTER_HPP
#define PROGRESS_REPORTER_HPP

#include <string>
#include <iostream>
#include <mutex>
#include "IProgressReporter.hpp"

namespace app {
namespace utils {

// Класс для отображения прогресса выполнения
class ProgressReporter : public IProgressReporter {
public:
    ProgressReporter();
    ~ProgressReporter() = default;

    // Начать новый прогресс
    void start(size_t total, const std::string& message = "") override;

    // Обновить прогресс
    void update(size_t current) override;

    // Увеличить прогресс на шаг
    void step(size_t increment = 1) override;

    // Завершить прогресс
    void finish(const std::string& message = "") override;

    // Получить текущий прогресс (0-100)
    int getProgress() const override;

    // Включить/отключить вывод в консоль
    void setEnabled(bool enabled) override;
    void setOutput(std::ostream& output) override;

private:
    void display();

    size_t m_current;
    size_t m_total;
    int m_lastProgress;
    std::string m_message;
    bool m_enabled;
    std::ostream* m_output;
    mutable std::mutex m_mutex;
};

} // namespace utils
} // namespace app

#endif // PROGRESS_REPORTER_HPP