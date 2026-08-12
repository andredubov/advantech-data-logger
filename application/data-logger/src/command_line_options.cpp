#include "command_line_options.hpp"
#include <cctype>
#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>

app::command_line_options::command_line_options() :
    m_options(),
    m_help_string(),
    m_version_string("1.0.0"),
    m_error_message(),
    m_device_description(),
    m_output_file_path(),
    m_channel_count(),
    m_sampling_rate(),
    m_samples_per_channel(),
    m_use_demo_device()
{
    setup();
}

void app::command_line_options::setup()
{
    po::options_description general_options("General options");
    po::options_description specific_options("Specific options");

    general_options.add_options()
        ("version", "Show version")
        ("help", "Show help");

    specific_options.add_options()
        ("device", po::value<std::string>(&m_device_description)->default_value("DemoDevice,BID#0"), "Device description (e.g., 'PCI-1716,BID#0' or 'DemoDevice,BID#0')")
        ("channels", po::value<long>(&m_channel_count)->default_value(16), "Number of channels to acquire (1-16)")
        ("rate", po::value<double>(&m_sampling_rate)->default_value(250000.0), "Sampling rate in Hz (max 250000)")
        ("buffer", po::value<long>(&m_samples_per_channel)->default_value(25000), "Buffer size in samples per channel")
        ("output", po::value<std::string>(&m_output_file_path)->default_value("daq_data.bin"), "Output binary file name")
        ("demo", po::value<bool>(&m_use_demo_device)->default_value(true)->implicit_value(true), "Use demo device (if not specified, tries real hardware)");

    m_options.add(general_options);
    m_options.add(specific_options);
}

app::command_line_options::state app::command_line_options::parse(int argc, char* argv[])
{
    command_line_options::state state = state::success;

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

        if (variable_map.count("channels"))
        {
            if (m_channel_count < 1 || m_channel_count > 16) {
                m_error_message = "Error: Channel count must be between 1 and 16.";
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

        if (variable_map.count("buffer"))
        {
            if (m_samples_per_channel <= 0) {
                m_error_message ="Error: Buffer size must be positive.";
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

bool app::command_line_options::is_file_valid(const std::string & file_path)
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

std::string app::command_line_options::get_help() const
{
    return m_help_string;
}

std::string app::command_line_options::get_version() const
{
    return m_version_string;
}

std::string app::command_line_options::get_error_message() const
{
    return m_error_message;
}

std::string app::command_line_options::get_device_description() const
{
    return m_device_description;
}

std::string app::command_line_options::get_output_file_path() const
{
    return m_output_file_path;
}

int app::command_line_options::get_channel_count() const
{
    return m_channel_count;
}

double app::command_line_options::get_sampling_rate() const
{
    return m_sampling_rate;
}

int app::command_line_options::get_samples_per_channel() const
{
    return m_samples_per_channel;
}

bool app::command_line_options::is_use_demo_device() const
{
    return m_use_demo_device;
}