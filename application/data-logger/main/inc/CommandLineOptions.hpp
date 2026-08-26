#ifndef COMMAND_LINE_OPTIONS_HPP
#define COMMAND_LINE_OPTIONS_HPP

#include <string>
#include <boost/program_options.hpp>

namespace app
{
    namespace po = boost::program_options;

    class command_line_options
    {
    public:
        enum class state : int
        {
            success,
            version,
            help,
            failure
        };

        explicit command_line_options();
        virtual ~command_line_options() = default;
        command_line_options(const command_line_options& rhs) = delete;
        command_line_options& operator=(const command_line_options& rhs) = delete;

        state parse(int argc, char* argv[]);

        virtual std::string get_help() const;
        virtual std::string get_version() const;
        virtual std::string get_error_message() const;
        virtual std::string get_device_description() const;
        virtual int get_start_channel() const;
        virtual int get_end_channel() const;
        virtual int get_channel_count() const;
        virtual double get_sampling_rate() const;
        virtual int get_samples_per_channel() const;
        virtual std::string get_output_file_path() const;
        std::string get_input_mode() const;
        std::string get_input_range() const;

    protected:
        virtual void setup();
        virtual bool is_file_valid(const std::string & file_path);

    private:
        po::options_description m_options;

        std::string m_help_string;
        std::string m_version_string;
        std::string m_error_message;
        std::string m_device_description;
        std::string m_output_file_path;
        std::string m_input_mode;
        std::string m_input_range;
        long m_start_channel;
        long m_end_channel;
        double m_sampling_rate;
        long m_samples_per_channel;
    };
}

#endif // COMMAND_LINE_OPTIONS_HPP