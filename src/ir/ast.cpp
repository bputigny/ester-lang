#include "config.h"
#include "log.hpp"
#include "ir.hpp"

#include <fstream>

namespace ir {

    int ast::nodes = 0;
    const int& ast::n_nodes = ast::nodes;
    const int& n_nodes = ast::n_nodes;

    void display_file(const std::string& file) {
#ifdef HAVE_DOT
        std::string cmd = "dot -Txlib " + file + " &";
        system(cmd.c_str());
#else
        err << "cannot disply graph in `" << file <<
            "': graphviz (dot) not found\n";
#endif
    }

    void ast::display(std::string title) const {
        char *n = std::tmpnam(NULL);
        std::string tmp(n);
        std::ofstream f;
        f.open(tmp);
        this->write_dot(f, title, true);
        f.close();
        display_file(tmp);
    }

    void ast::write_dot(std::ostream& os,
            const std::string& title, bool root) const {
        if (root) {
            os << "digraph ir {\n";
            if (title != "") {
                os << "graph [label=\"" << title <<
                    "\", labelloc=t, fontsize=20];\n";
            }
            os << "node [shape = Mrecord]\n";
        }
        os << (long) this << " [label=\"";
        os << std::string(*this);
        os << "\"]\n";

        for (auto c: this->children) {
            os << (long) this << " -> " << (long) c.get() << "\n";
        }

        for (auto c: this->children) {
            c->write_dot(os, title, false);
        }
        if (root) {
            os << "}\n";
        }
    }

    void equation::add_bc(std::shared_ptr<const bc> bc) {
        bcs.push_back(bc);
        add_child(bc);
    }

}
