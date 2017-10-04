#ifndef ARGS_H
#define ARGS_H

#include <unistd.h>
#include <iomanip>
#include <map>
#include <vector>
#include <string>

namespace cmdline {

enum {
    no_argument         = 0,
    required_argument   = 1,
    optional_argument   = 2
};

class arg {
    public:
        arg(const std::string& name);
        arg(const std::string& name, std::string default_value);
        int has_arg; // 0: no arg, 1: required_argument, 2: optional_argument
        const std::string name;
        const std::string default_value;
};


class parser {
    public:
        parser(int argc, char *argv[]);

        int getopt(std::string &opt, std::string &arg);

    private:
        int n;
        int argc;
        std::vector<std::string> argv;
};


class args {
    public:
        /// \brief Adds an option to parse
        void add_opt(const std::string& opt, int has_arg);
        void add_opt(const std::string& opt,
                const std::string& default_value,
                int has_arg);

        void add_pos_arg(const std::string& name);

        int parse(int argc, char *argv[]);

        void print();

        std::string get(const std::string& key);

    private:
        std::vector<arg> opts;
        std::map<std::string, std::string> values;

        std::vector<std::string> pos_args_names;
        std::vector<std::string> pos_args_values;
};

} // end namespace cmdline

#endif
