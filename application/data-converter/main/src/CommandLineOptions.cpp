#include "CommandLineOptions.hpp"
#include <cctype>
#include <iostream>
#include <algorithm>
#include <regex>
#include <boost/filesystem.hpp>

app::CommandLineOptions::CommandLineOptions() :
    m_options(),
    m_help_string(),
    m_version_string("1.0.5"),
    m_error_message(),    
    m_input_file_path(),
    m_output_file_path()
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
        ("input-file", po::value<std::string>(&m_input_file_path)->default_value("daq_data.bin"), "Input binary file name")
        ("output-file", po::value<std::string>(&m_output_file_path)->default_value("daq_data.csv"), "Output csv file name");

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

        if (variable_map.count("input-file")) {
            state = is_input_file_valid(m_input_file_path) ? state::success : state::failure;
        } else {
            m_error_message = "ERROR: missing an argument for the option [--input-file].";
            return state::failure;
        }

        if (variable_map.count("output-file")) {
            state = is_output_file_valid(m_output_file_path) ? state::success : state::failure;
        } else {
            m_error_message = "ERROR: missing an argument for the option [--output-file].";
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

bool app::CommandLineOptions::is_input_file_valid(const std::string & file_path)
{
    boost::filesystem::path path(file_path);

    // Проверяем, что путь не является существующей директорией
    if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
    {
        m_error_message = "ERROR - path is a directory, not a file! [" +  file_path  + "]";
        return false;
    }

    // Проверяем, что файл существует
    if (!boost::filesystem::exists(path))
    {
        m_error_message = "ERROR - input file does not exist! [" +  file_path  + "]";
        return false;
    }

    // Проверяем расширение файла (.bin) через строку
    std::string filename = path.filename().string();
    if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".bin")
    {
        m_error_message = "ERROR - input file must have .bin extension! [" +  file_path  + "]";
        return false;
    }

    return true;
}

bool app::CommandLineOptions::is_output_file_valid(const std::string & file_path)
{
    boost::filesystem::path path(file_path);

    // Проверяем, что путь не является существующей директорией
    if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
    {
        m_error_message = "ERROR - path is a directory, not a file! [" +  file_path  + "]";
        return false;
    }

    // Проверяем расширение файла через регулярное выражение (.csv)
    std::string filename = path.filename().string();
    std::regex binPattern(R"(^.*\.csv$)", std::regex_constants::icase);
    if (!std::regex_match(filename, binPattern))
    {
        m_error_message = "ERROR - input file must have .bin extension! [" +  file_path  + "]";
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

std::string app::CommandLineOptions::get_input_file_path() const
{
    return m_input_file_path;
}

std::string app::CommandLineOptions::get_output_file_path() const
{
    return m_output_file_path;
}