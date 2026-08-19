#ifndef DATAFRAME_HPP
#define DATAFRAME_HPP

#include <vector>
#include <cstdint>

namespace app {

/**
 * @brief Represents a single data frame from the acquisition device.
 * Contains timestamp and voltage values for all channels.
 */
struct DataFrame {
    /**
     * @brief Timestamp in seconds since the start of acquisition
     */
    double timestamp;

    /**
     * @brief Voltage values for each channel (in volts)
     * Index corresponds to channel number (starting from startChannel)
     */
    std::vector<double> voltages;

    /**
     * @brief Default constructor
     */
    DataFrame() : timestamp(0.0) {}

    /**
     * @brief Constructor with timestamp and voltages
     * @param ts Timestamp in seconds
     * @param volts Vector of voltage values
     */
    DataFrame(double ts, const std::vector<double>& volts)
        : timestamp(ts), voltages(volts) {}

    /**
     * @brief Constructor with timestamp and voltages (move semantics)
     * @param ts Timestamp in seconds
     * @param volts Vector of voltage values (moved)
     */
    DataFrame(double ts, std::vector<double>&& volts)
        : timestamp(ts), voltages(std::move(volts)) {}

    /**
     * @brief Get the number of channels in this frame
     */
    size_t channelCount() const {
        return voltages.size();
    }

    /**
     * @brief Check if the frame has valid data
     */
    bool isValid() const {
        return !voltages.empty();
    }
};

} // namespace app

#endif // DATAFRAME_HPP
