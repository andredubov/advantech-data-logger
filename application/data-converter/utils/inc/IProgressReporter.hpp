#ifndef I_PROGRESS_REPORTER_HPP
#define I_PROGRESS_REPORTER_HPP

#include <string>
#include <iostream>

namespace app {
namespace utils {

/**
 * @brief Интерфейс для отображения прогресса выполнения
 */
class IProgressReporter {
public:
    virtual ~IProgressReporter() = default;

    // Начать новый прогресс
    virtual void start(size_t total, const std::string& message = "") = 0;

    // Обновить прогресс
    virtual void update(size_t current) = 0;

    // Увеличить прогресс на шаг
    virtual void step(size_t increment = 1) = 0;

    // Завершить прогресс
    virtual void finish(const std::string& message = "") = 0;

    // Получить текущий прогресс (0-100)
    virtual int getProgress() const = 0;

    // Включить/отключить вывод в консоль
    virtual void setEnabled(bool enabled) = 0;
    virtual void setOutput(std::ostream& output) = 0;
};

} // namespace utils
} // namespace app

#endif // I_PROGRESS_REPORTER_HPP