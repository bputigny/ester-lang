#ifndef FRONTEND_H
#define FRONTEND_H

#include "solver.hpp"

#include <list>
#include "parser.hpp"

#include <string>

namespace yacc {
    extern std::string filename;
};

extern FILE *yyin;
extern int yyparse();

class frontend {
    public:
        int parse(const std::string&);
        ~frontend();
        void info() {
            solver.info();
        }

        void emit_code(std::ostream& os) { solver.emit_code(os); }

    private:
        std::shared_ptr<const ir::ast> ast;
        ir::solver solver;
};

#endif
