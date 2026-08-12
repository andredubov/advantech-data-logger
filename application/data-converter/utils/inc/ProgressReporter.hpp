#ifndef PROGRESS_REPORTER_HPP
#define PROGRESS_REPORTER_HPP

#include <string>
#include <iostream>
#include <mutex>

namespace app {
namespace utils {

// Класс для отображения прогресса выполнения
class ProgressReporter {
public:
    ProgressReporter();
    ~ProgressReporter() = default;

    // Начать новый прогресс
    void start(size_t total, const std::string& message = "");

    // Обновить прогресс
    void update(size_t current);

    // Увеличить прогресс на шаг
    void step(size_t increment = 1);

    // Завершить прогресс
    void finish(const std::string& message = "");

    // Получить текущий прогресс (0-100)
    int getProgress() const;

    // Включить/отключить вывод в консоль
    void setEnabled(bool enabled);
    void setOutput(std::ostream& output);

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