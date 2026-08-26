#ifndef COMMAND_LINE_OPTIONS_HPP
#define COMMAND_LINE_OPTIONS_HPP

#include <string>
#include <boost/program_options.hpp>

namespace app
{
    namespace po = boost::program_options;

    class CommandLineOptions
    {
    public:
        enum class state : int
        {
            success,
            version,
            help,
            failure
        };

        explicit CommandLineOptions();
        virtual ~CommandLineOptions() = default;
        CommandLineOptions(const CommandLineOptions& rhs) = delete;
        CommandLineOptions& operator=(const CommandLineOptions& rhs) = delete;

        state parse(int argc, char* argv[]);

        virtual std::string get_help() const;
        virtual std::string get_version() const;
        virtual std::string get_error_message() const;
        virtual std::string get_input_file_path() const;
        virtual std::string get_output_file_path() const;

    protected:
        virtual void setup();
        virtual bool is_output_file_valid(const std::string & file_path);
        virtual bool is_input_file_valid(const std::string & file_path);

    private:
        po::options_description m_options;

        std::string m_help_string;
        std::string m_version_string;
        std::string m_error_message;
        std::string m_input_file_path;
        std::string m_output_file_path;
    };
}

#endif // COMMAND_LINE_OPTIONS_HPP