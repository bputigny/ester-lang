#include "frontend.hpp"
#include "ir.hpp"

namespace yacc {
    std::string filename = "";
}

frontend::~frontend() {
    if (yyin) {
        fclose(yyin);
        yyin = NULL;
        yacc::filename = "";
    }
}

int frontend::parse(const std::string& filename) {
    yyin = fopen(filename.c_str(), "r");
    yacc::filename = filename;
    if (yyin == NULL) {
        std::cerr << "Opening file `" << filename << "' failed\n";
        return 1;
    }
    if (yyparse(&solver)) {
        return 1;
    }
    return 0;
}

