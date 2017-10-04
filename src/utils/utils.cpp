#include "log.hpp"
#include "args.hpp"

namespace log {

    std::ostream& log() {
        return std::cout;
    }

    std::ostream& warn() {
        std::cout << termcolor::bold << termcolor::yellow
            << "[WARNING]: " << termcolor::reset;
        return std::cerr;
    }

    std::ostream& err() {
        std::cout << termcolor::bold << termcolor::red
            << "[ERROR]: " << termcolor::reset;
        return std::cerr;
    }

} // end namespace log

namespace cmdline {

    arg::arg(const std::string& name) : name(name) { }
    arg::arg(const std::string& name, std::string default_value)
        : name(name), default_value(default_value)  { }


    parser::parser(int argc, char *argv[]) : argc(argc) {
        this->n = 1;
        for (int i=0; i<argc; i++) {
            this->argv.push_back(argv[i]);
        }
    }

    int parser::getopt(std::string &opt, std::string &arg) {
        if (n < argc) {
            opt = argv[n++];

            if (opt[0] == '-') {
                if (n < argc) {
                    if (argv[n][0] == '-') arg = "";
                    else arg = argv[n++];
                }
                else arg = "";
            }

            return 1;
        }
        return 0;
    }

    void args::add_opt(const std::string& opt, int has_arg) {
        add_opt(opt, "", has_arg);
    }

    void args::add_opt(const std::string& opt, const std::string& default_value,
            int has_arg) {

        class arg arg(opt, default_value);
        arg.has_arg = has_arg;
        opts.push_back(arg);

    }

    void args::add_pos_arg(const std::string& name) {
        pos_args_names.push_back(name);
    }

    int args::parse(int argc, char *argv[]) {
        parser parser(argc, argv);
        std::string opt, arg;

        for (auto o: opts) {
            values[o.name] = o.default_value;
        }

        while (parser.getopt(opt, arg)) {
            if (opt[0] == '-') {
                bool found_opt = false;
                for (auto o: opts) {
                    if (std::string("-") + o.name == opt) {
                        found_opt = true;
                        if (arg == "") {
                            if (o.has_arg == required_argument) {
                                log::err()
                                    << "Missing required argument to "
                                    << " option `" << o.name << "'\n";
                                return 1;
                            }
                            else {
                                values[o.name] = "";
                            }
                        }
                        else {
                            if (o.has_arg == no_argument) {
                                pos_args_values.push_back(std::string(arg));
                            }
                            else {
                                values[o.name] = std::string(arg);
                            }
                        }
                    }
                }
                if (!found_opt) {
                    log::warn() << "Ingoring unknown option " << opt << '\n';
                    if (arg != "") {
                        pos_args_values.push_back(arg);
                    }
                }
            }
            else {
                pos_args_values.push_back(opt);
            }
        }

        if (pos_args_values.size() < pos_args_names.size()) {
            log::err() << "Missing command line argument "
                << pos_args_names[pos_args_values.size()] << '\n';
            return 1;
        }
        if (pos_args_values.size() > pos_args_names.size()) {
            std::string ignored_arg = "";
            for (size_t i=pos_args_names.size(); i<pos_args_values.size(); i++) {
                ignored_arg = ignored_arg + " " + pos_args_values[i];
            }
            log::warn() << "Ignoring command line arguments: " << ignored_arg << '\n';
        }
        return 0;
    }

    void args::print() {
        log::log() << "Options:\n";
        for (auto val: values) {
            log::log() << std::setw(16) << "-"
                << val.first;
            if (val.second != "")
                log::log() << ": " << val.second << '\n';
            else
                log::log() << '\n';
        }
        log::log() << "Arguments:\n";
        int i = 0;
        for (auto arg: pos_args_names) {
            log::log() << std::setw(16) << "-"
                << arg << ": ";
            log::log() << pos_args_values[i++] << '\n';
        }
    }

    std::string args::get(const std::string& key) {
        int i = 0;
        for (auto name: pos_args_names) {
            if (name == key)
                return pos_args_values[i];
            i++;
        }
        return values[key];
    }

} // end namespace cmdline

