#include "frontend.hpp"
#include "log.hpp"
#include "args.hpp"

#include <cstring>

int main(int argc, char *argv[]) {
    cmdline::args args;
    args.add_pos_arg("filename");
    args.add_opt("o", cmdline::required_argument);
    args.add_opt("v", "0", cmdline::optional_argument);
    int verbosity = 0;
    if (args.parse(argc, argv)) {
        std::exit(EXIT_FAILURE);
    }
    std::string output = args.get("o");
    try {
        verbosity = std::stoi(args.get("v"));
    }
    catch (std::invalid_argument) {
        verbosity = 1;
    }

    std::ofstream ofile;
    if (output != "") {
        ofile.open(output, std::ios::out);
    }
    std::ostream& os = (output == "")?std::cout:ofile;

    int r;
    {
        frontend f;
        std::string input = args.get("filename");
        if (input == "-") {
            log::log() << "Reading from standard input\n";
            r = f.parse("/dev/stdin");
        }
        else {
            r = f.parse(input);
            if (r == 0) {
                if (verbosity > 0) f.info();
                f.emit_code(os);
                if (ofile.is_open())
                    ofile.close();
            }
        }
    }
    if (ir::n_nodes > 0) {
        log::warn() << ir::n_nodes << " nodes still allocated\n";
    }

    return r;
}
