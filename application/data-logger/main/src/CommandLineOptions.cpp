#include "CommandLineOptions.hpp"
#include <cctype>
#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>

app::CommandLineOptions::CommandLineOptions() :
    m_options(),
    m_help_string(),
    m_version_string("1.0.6"),
    m_error_message(),
    m_device_description(),
    m_output_file_path(),
    m_start_channel(0),
    m_end_channel(15),
    m_sampling_rate(),
    m_samples_per_channel()
{
    setup();
}

void app::CommandLineOptions::setup()
{
    po::options_description general_options("General options");
    po::options_description specific_options("Specific options");

    general_options.add_options()
        ("version", "Show version")
        ("help", "Show help");

    specific_options.add_options()
        ("device", po::value<std::string>(&m_device_description)->default_value("DemoDevice,BID#0"), "Device description (e.g., 'PCI-1716,BID#0' or 'DemoDevice,BID#0')")
        ("start-channel", po::value<long>(&m_start_channel)->default_value(0), "Start channel (0-15)")
        ("end-channel", po::value<long>(&m_end_channel)->default_value(15), "End channel (0-15)")
        ("rate", po::value<double>(&m_sampling_rate)->default_value(250000.0), "Sampling rate in Hz (max 250000)")
        ("samples-per-channel", po::value<long>(&m_samples_per_channel)->default_value(25000), "Number of samples per channel in the buffer")
        ("output", po::value<std::string>(&m_output_file_path)->default_value("daq_data.bin"), "Output binary file name")
        ("input-mode", po::value<std::string>(&m_input_mode)->default_value("unipolar"), "Input mode: bipolar or unipolar")
        ("input-range", po::value<std::string>(&m_input_range)->default_value("10V"), "Input range: 10V, 5V, 2.5V, 1.25V");

    m_options.add(general_options);
    m_options.add(specific_options);
}

app::CommandLineOptions::state app::CommandLineOptions::parse(int argc, char* argv[])
{
    CommandLineOptions::state state = state::success;

    try 
    {
        po::variables_map variable_map;
        po::parsed_options parsed = po::parse_command_line(argc, argv, m_options);
        po::store(parsed, variable_map);
        po::notify(variable_map);

        if (variable_map.count("help"))
        {
            std::stringstream ss;
            ss << m_options;
            m_help_string = ss.str();

            return state::help;
        }

        if (variable_map.count("version"))
        {
            return state::version;
        }

        if (variable_map.count("start-channel") || variable_map.count("end-channel"))
        {
            if (m_start_channel < 0 || m_start_channel > 15 || m_end_channel < 0 || m_end_channel > 15) {
                m_error_message = "Error: Channels must be between 0 and 15.";
                return state::failure;
            }
            if (m_start_channel > m_end_channel) {
                m_error_message = "Error: Start channel must be less than or equal to end channel.";
                return state::failure;
            }
        }

        if (variable_map.count("rate"))
        {
            if (m_sampling_rate <= 0.0 || m_sampling_rate > 250000.0) {
                m_error_message = "Error: Sampling rate must be between 1 and 250000 Hz.";
                return state::failure;
            }
        }

        if (variable_map.count("samples-per-channel"))
        {
            if (m_samples_per_channel <= 0) {
                m_error_message ="Error: Samples per channel must be positive.";
                return state::failure;
            }
        }

        if (variable_map.count("output")) {
            state = is_file_valid(m_output_file_path) ? state::success : state::failure;
        } else {
            m_error_message = "ERROR: missing an argument for the option [--output].";
            return state::failure;
        }
    }
    catch (const std::exception& e)
    {
        m_error_message = "ERROR: command line parser error [" + std::string(e.what()) + "].";
        state = state::failure;
    }
    catch (...)
    {
        m_error_message = "ERROR: command line parser error [unknown error type].";
        state = state::failure;
    }

    return state;
}

bool app::CommandLineOptions::is_file_valid(const std::string & file_path)
{
    boost::filesystem::path path(file_path);
    
    // Проверяем, что путь не является существующей директорией
    if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
    {
        m_error_message = "ERROR - path is a directory, not a file! [" +  file_path  + "]";
        return false;
    }
    
    // Проверяем, что родительская директория существует
    boost::filesystem::path parent = path.parent_path();
    if (!parent.empty() && !boost::filesystem::exists(parent))
    {
        // Создаём родительскую директорию, если её нет
        boost::system::error_code ec;
        boost::filesystem::create_directories(parent, ec);
        if (ec)
        {
            m_error_message = "ERROR - cannot create directory [" + parent.string() + "]: " + ec.message();
            return false;
        }
    }

    return true;
}

std::string app::CommandLineOptions::get_help() const
{
    return m_help_string;
}

std::string app::CommandLineOptions::get_version() const
{
    return m_version_string;
}

std::string app::CommandLineOptions::get_error_message() const
{
    return m_error_message;
}

std::string app::CommandLineOptions::get_device_description() const
{
    return m_device_description;
}

std::string app::CommandLineOptions::get_output_file_path() const
{
    return m_output_file_path;
}

int app::CommandLineOptions::get_start_channel() const
{
    return m_start_channel;
}

int app::CommandLineOptions::get_end_channel() const
{
    return m_end_channel;
}

int app::CommandLineOptions::get_channel_count() const
{
    return m_end_channel - m_start_channel + 1;
}

double app::CommandLineOptions::get_sampling_rate() const
{
    return m_sampling_rate;
}

std::string app::CommandLineOptions::get_input_mode() const
{
    return m_input_mode;
}

std::string app::CommandLineOptions::get_input_range() const
{
    return m_input_range;
}

int app::CommandLineOptions::get_samples_per_channel() const
{
    return m_samples_per_channel;
}

