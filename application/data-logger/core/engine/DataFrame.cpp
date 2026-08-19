#include "DataFrame.hpp"

namespace app {

// Реализация конструкторов и методов DataFrame

DataFrame::DataFrame() : timestamp(0.0) {}

DataFrame::DataFrame(double ts, const std::vector<double>& volts)
    : timestamp(ts), voltages(volts) {}

DataFrame::DataFrame(double ts, std::vector<double>&& volts)
    : timestamp(ts), voltages(std::move(volts)) {}

size_t DataFrame::channelCount() const {
    return voltages.size();
}

bool DataFrame::isValid() const {
    return !voltages.empty();
}

} // namespace app
