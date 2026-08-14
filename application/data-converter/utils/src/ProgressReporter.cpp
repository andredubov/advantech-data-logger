#include "ProgressReporter.hpp"
#include <iomanip>
#include <algorithm>

namespace app {
namespace utils {

ProgressReporter::ProgressReporter()
    : m_current(0)
    , m_total(1)
    , m_lastProgress(-1)
    , m_message()
    , m_enabled(true)
    , m_output(&std::cout)
    , m_mutex()
{
}

void ProgressReporter::start(size_t total, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_total = total > 0 ? total : 1;
    m_current = 0;
    m_lastProgress = -1;
    m_message = message;
    
    if (m_enabled && !m_message.empty()) {
        (*m_output) << m_message << std::endl;
    }
    
    display();
}

void ProgressReporter::update(size_t current)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current = std::min(current, m_total);
    display();
}

void ProgressReporter::step(size_t increment)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current = std::min(m_current + increment, m_total);
    display();
}

void ProgressReporter::display()
{
    if (!m_enabled) {
        return;
    }

    int progress = static_cast<int>((static_cast<double>(m_current) / m_total) * 100);
    
    if (progress != m_lastProgress) {
        m_lastProgress = progress;
        
        // Используем символ возврата каретки для обновления строки
        (*m_output) << "\rProgress: " << std::setw(3) << progress << "% "
                    << "(" << m_current << "/" << m_total << ")"
                    << std::flush;
    }
}

void ProgressReporter::finish(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current = m_total;
    
    if (m_enabled) {
        display();
        (*m_output) << std::endl;
        
        if (!message.empty()) {
            (*m_output) << message << std::endl;
        }
    }
    
    m_lastProgress = 100;
}

int ProgressReporter::getProgress() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>((static_cast<double>(m_current) / m_total) * 100);
}

void ProgressReporter::setEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_enabled = enabled;
}

void ProgressReporter::setOutput(std::ostream& output)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_output = &output;
}

} // namespace utils
} // namespace app
