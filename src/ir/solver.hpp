#ifndef SOLVER_H
#define SOLVER_H

#include "ir.hpp"
#include "path.hpp"

#include <fstream>
#include <map>

namespace ir {

class variable {
    public:
        variable(const std::string& name, var_type type) : name(name), type(type) { }
        ~variable() { }

        const std::string name;
        const var_type type;
};

inline void write_template_file(std::ostream& os, const std::string& name) {
    std::ifstream file;
    char line[256];
    file.open(ABS_TOP_SRCDIR + "/templates/" + name,
            std::ios::in);
    if (!file.is_open()) {
        log::err() << "Could not open template file " << name << '\n';
    }
    while (file.getline(line, 256)) {
        os << line << '\n';
    }
    file.close();
    os << '\n';
}

class solver {
    public:
        solver() { }
        ~solver() { vars.clear(); }

        int add_var(std::shared_ptr<const variable> var) {
            for (auto v: vars) {
                if (v->name == var->name) {
                    return 1;
                }
            }
            vars.push_back(var);
            return 0;
        }

        void add_param(const std::string& name, const std::string& type) {
            params[name] = type;
        }

        void add_eq(std::shared_ptr<const equation> eq) {
            eqs.push_back(eq);
        }

        void info() {
            log::log() << "Solver:\n";
            log::log() << "  - Variables:\n";
            for (auto v: vars) {
                log::log() << "        - " << v->name;
                switch (v->type) {
                    case REAL:
                        log::log() << " (real)";
                        break;
                    case FIELD:
                        log::log() << " (field)";
                        break;
                }
                log::log() << '\n';
            }
            log::log() << "  - Equations:\n";
            for (auto eq: eqs) {
                log::log() << "        - " << eq->name
                    << " (BCs: " << eq->bcs.size() << ")" << '\n';
            }
        }

        void emit_code(std::ostream& os) {
            os << "#include <ester.h>\n\n";
            write_template_file(os, "mapping.cpp");

            for (auto p: params) {
                os << "extern " << p.second << " " << p.first << ";\n";
            }

            emit_solver(os);
        }

        void emit_solver(std::ostream& os) {
            os << "// definition of matrices used to store variables value\n";
            for (auto v: vars) {
                os << "extern matrix " << v->name << ";\n";
            }

            // Register variables & symbolic obj
            os << "solver *create_solver() {\n";
            os << "    mapping map;\n";
            os << "    symbolic S;\n";
            os << "    solver *op = new solver();\n";
            os << "    op->init(1, " << vars.size() << ", \"full\");\n";
            os << "    create_map(map);\n";
            os << "    S.set_map(map);\n";
            os << "    op->set_nr(map.npts);\n";
            for (auto var: vars) {
                os << "    sym sym_" << var->name
                    << " = S.regvar(\"" << var->name << "\");\n";
                os << "    op->regvar(\"" << var->name << "\");\n";
                os << "    S.set_value(\"" << var->name << "\", "
                    << var->name << ");\n";
            }
            for (auto eq: eqs) {
                if (eq->rhs.has_field_value() || eq->lhs.has_field_value()) {
                    emit_eq_in_bc(os, *eq);
                }
                else {
                    os << "\n    sym eq_" << eq->name << " = ";
                    emit_eq(os, eq);
                    os << ";\n";

                    std::vector<const identifier *> vars;
                    get_vars(eq->lhs, vars);
                    get_vars(eq->rhs, vars);
                    for (auto id: vars) {
                        os << "    eq_" << eq->name << ".add(op, \""
                            << eq->name << "\", \""
                            << id->name << "\");\n";
                    }

                    os << "\n    // Boundary conditions\n";
                    for (auto bc: eq->bcs) {
                        if (bc->eq.lhs != ir::value(0))
                            emit_bc(os, eq->name, bc->bc_loc, bc->eq.lhs);
                        if (bc->eq.rhs != ir::value(0))
                            emit_bc(os, eq->name, bc->bc_loc, -bc->eq.rhs);
                    }

                    os << "\n    // RHS\n";
                    emit_rhs(os, *eq);
                }
            }
            os << "    return op;\n";
            os << "}\n";
        }

        void emit_rhs(std::ostream& os, const equation& eq) {
            os << "    matrix rhs = -eq_" << eq.name << ".eval();\n";

            // Fix rhs at BC
            int n_top_bc = 0;
            int n_bot_bc = 0;
            for (auto bc: eq.bcs) {
                switch (bc->bc_loc) {
                    case CENTER:
                    case BOTTOM:
                        n_bot_bc++;
                        os << "    rhs(0) = -(";
                        if (bc->eq.rhs == value(0))
                            emit_eval_expr(os, bc->eq.lhs);
                        else
                            emit_eval_expr(os, bc->eq.lhs-bc->eq.rhs);
                        os << ")(0);\n";
                        break;
                    case SURFACE:
                    case TOP:
                        os << "    rhs(-1) = -(";
                        if (bc->eq.rhs == value(0))
                            emit_eval_expr(os, bc->eq.lhs);
                        else
                            emit_eval_expr(os, bc->eq.lhs-bc->eq.rhs);
                        os << ")(-1);\n";
                        n_top_bc++;
                        break;
                    default:
                        error("Unknown BC type" + std::to_string(bc->bc_loc));
                }
            }
            if (n_top_bc > 1 || n_bot_bc > 1) {
                error("Too many BC imposed on equation " + eq.name);
            }

            os << "    op->set_rhs(\"" << eq.name << "\", rhs);\n";
        }

        void emit_eval_expr(std::ostream& os, const expr& expr) {
            if (auto id = dynamic_cast<const identifier *>(&expr)) {
                os << id->name;
            }
            else if (auto be = dynamic_cast<const bin_expr *>(&expr)) {
                os << "(";
                emit_eval_expr(os, be->lhs);
                os << ")";
                os << be->op;
                os << "(";
                emit_eval_expr(os, be->rhs);
                os << ")";
            }
            else if (auto ue = dynamic_cast<const unary_expr *>(&expr)) {
                if (ue->op == '-') {
                    os << "-(";
                    emit_eval_expr(os, ue->expr);
                    os << ")";
                }
                else {
                    error("Unknown unary operator `"
                            + std::to_string(ue->op)
                            + "\'");
                }
            }
            else if (auto de = dynamic_cast<const diff_expr *>(&expr)) {
                if (de->id.name == "r") {
                    os << "(map.D, ";
                    emit_eval_expr(os, de->expr);
                    os << ")";
                }
                else {
                    TODO;
                }
            }
            else {
                expr.display("Term skipped");
                error("Term skipped...");
            }
        }

        int need_value_at(const expr& expr) {
            if (auto fv = dynamic_cast<const field_value *>(&expr)) {
                if (auto index = dynamic_cast<const value *>(&fv->index)) {
                    if (index->val == 0) {
                        return BOTTOM;
                    }
                    if (index->val == 1) {
                        return TOP;
                    }
                }
                TODO;
            }
            else if (auto be = dynamic_cast<const bin_expr *>(&expr)) {
                int l = need_value_at(be->lhs);
                int r = need_value_at(be->rhs);
                if (l == -1) return r;
                if (r == -1) return l;
                if (r == l) return r;
                error("Value of field need at 2 different locations");
            }
            else if (dynamic_cast<const identifier *>(&expr)) {
                return -1;
            }
            else if (dynamic_cast<const value *>(&expr)) {
                return -1;
            }
            else {
                expr.display();
                TODO;
            }
            return -1;
        }

        void emit_eq_in_bc(std::ostream& os, const equation& eq) {
            const expr& e = eq.lhs - eq.rhs;
            auto dexpr = func_der(e);
            int loc = need_value_at(e);
            switch (loc) {
                case CENTER:
                case BOTTOM:
                    os << "\n    // have to set equation "
                        << eq.name << " in bottom BC\n";
                    break;
                case SURFACE:
                case TOP:
                    os << "\n    // have to set equation "
                        << eq.name << " in top BC\n";
                    break;
                case -1:
                    error("No reason to set equation " + eq.name + " at a BC");
                default:
                    error("Unknown BC " + std::to_string(loc));
            }
            emit_bc_expr(os, eq.name, loc, *dexpr);

            os << "\n    // RHS\n";
            os << "    op->set_rhs(\""
                << eq.name << "\", -(";
            emit_expr(os, eq.lhs - eq.rhs);
            os << ")"; 
            switch (loc) {
                case CENTER:
                case BOTTOM:
                    os << "(0)*ones(1, 1)";
                    break;
                case SURFACE:
                case TOP:
                    os << "(-1)*ones(1, 1)";
                    break;
            }
            os << ");\n";
        }

        /// \brief calculates the functional derivative of the expression
        /// provided as argument
        std::shared_ptr<const expr> func_der(const expr& e) {
            if (auto be = dynamic_cast<const bin_expr *>(&e)) {
                auto dlhs = func_der(be->lhs);
                auto drhs = func_der(be->rhs);
                switch (be->op) {
                    case '+':
                        if (*drhs == value(0)) return dlhs;
                        if (*dlhs == value(0)) return drhs;
                        return std::make_shared<const bin_expr>(dlhs, '+', drhs);
                        break;
                    case '-':
                        if (*drhs == value(0)) return dlhs;
                        if (*dlhs == value(0))
                            return std::make_shared<const unary_expr>('-', drhs);
                        return std::make_shared<const bin_expr>(dlhs, '-', drhs);
                        break;
                    case '*':
                        return std::make_shared<const bin_expr>(
                                std::make_shared<const bin_expr>(
                                    dlhs, '*', be->rhs.copy())
                                , '+', 
                                std::make_shared<const bin_expr>(
                                    be->lhs.copy(), '*', drhs)
                                );
                        break;
                    default:
                        TODO;
                }
            }
            else if (auto id = dynamic_cast<const identifier *>(&e)) {
                return std::make_shared<const delta>(id->name);
            }
            else if (dynamic_cast<const value *>(&e)) {
                return std::make_shared<const value>(0);
            }
            else {
                TODO;
            }
        }

        bool contains_delta(const expr& expr) {
            if (dynamic_cast<const delta *>(&expr)) return true;
            else if (auto be = dynamic_cast<const bin_expr *>(&expr)) {
                return contains_delta(be->lhs)
                    && contains_delta(be->rhs);
            }
            else {
                TODO;
            }
        }

        void emit_bc_expr(std::ostream& os,
                const std::string& eq_name, int bc_loc,
                const expr& expr, bool neg = false) {
            std::string bc_func_name;
            std::string loc_index = "0";
            switch (bc_loc) {
                case TOP:
                case SURFACE:
                    bc_func_name = "bc_top1_add_d";
                    bc_func_name = "bc_top1_add_d";
                    loc_index = "-1";
                    break;
                case BOTTOM:
                case CENTER:
                    bc_func_name = "bc_bot2_add_d";
                    break;
                default:
                    error("Unknown BC " + std::to_string(bc_loc));
            }
            if (auto d = dynamic_cast<const delta *>(&expr)) {
                std::string factor;
                if (neg) factor = "-ones(1, 1)";
                else factor = "ones(1, 1)";
                os << "    op->" << bc_func_name << "(0, \"" << eq_name
                    << "\", \"" << d->name << "\", " << factor << ");\n";
            }
            else if (auto be = dynamic_cast<const bin_expr *>(&expr)) {
                switch (be->op) {
                    case '+':
                        emit_bc_expr(os, eq_name, bc_loc, be->lhs, neg);
                        emit_bc_expr(os, eq_name, bc_loc, be->rhs, neg);
                        break;
                    case '-':
                        emit_bc_expr(os, eq_name, bc_loc, be->lhs, neg);
                        emit_bc_expr(os, eq_name, bc_loc, be->rhs, !neg);
                        break;
                    case '*':
                        if (auto d = dynamic_cast<const delta *>(&be->lhs)) {
                            os << "    op->" << bc_func_name << "(0, \""
                                << eq_name << "\", \""
                                << d->name << "\", (";
                            if (neg) emit_expr(os, -be->rhs);
                            else emit_expr(os, be->rhs);
                            os << ")(" << loc_index << ")*ones(1, 1));\n";
                        }
                        else if (auto d = dynamic_cast<const delta *>(&be->rhs)) {
                            os << "    op->" << bc_func_name << "(0, \""
                                << eq_name << "\", \""
                                << d->name << "\", (";
                            if (neg) emit_expr(os, -be->lhs);
                            else emit_expr(os, be->lhs);
                            os << ")(" << loc_index << ")*ones(1, 1));\n";
                        }
                        else {
                            if (auto rbe = dynamic_cast<const bin_expr *>(&be->rhs)) {
                                if (rbe->op == '+') {
                                    emit_bc_expr(os, eq_name, bc_loc,
                                            be->lhs*rbe->lhs, neg);
                                    emit_bc_expr(os, eq_name, bc_loc,
                                            be->lhs*rbe->rhs, neg);
                                }
                                else if (rbe->op == '-') {
                                    emit_bc_expr(os, eq_name, bc_loc,
                                            be->lhs*rbe->lhs, neg);
                                    emit_bc_expr(os, eq_name, bc_loc,
                                            be->lhs*rbe->rhs, !neg);
                                }
                                else {
                                    TODO;
                                }
                            }
                            else {
                                TODO;
                            }
                        }
                        break;
                    default:
                        TODO;
                }
            }
            else {
                TODO;
            }
        }

        void emit_eq(std::ostream& os, std::shared_ptr<const ir::equation> eq) {
            if (auto rhs = dynamic_cast<const unary_expr *>(&eq->rhs)) {
                if (rhs->op == '-') {
                    emit_symbolic_expr(os, eq->lhs + rhs->expr);
                    return;
                }
            }
            emit_symbolic_expr(os, eq->lhs - eq->rhs);
        }

        void emit_symbolic_expr(std::ostream& os, const expr& expr) {
            emit_expr(os, expr, true);
        }

        /// \brief write expression `expr' to the outpur stream. If `symbolic'
        /// is true writes the symbolic version of the expression. Otherwise
        // writes the matrix (value) expression
#define PRETTY_EXPR
        void emit_expr(std::ostream& os, const expr& expr, bool symbolic = false) {
            const ir::expr *e = &expr;
            if (auto be = dynamic_cast<const bin_expr *>(e)) {
#ifdef PRETTY_EXPR
                if (auto lhs = dynamic_cast<const bin_expr *>(&be->lhs))
                    if (lhs->precedence < be->precedence)
                        os << "(";
#else
                os << "(";
#endif
                emit_expr(os, be->lhs, symbolic);
#ifdef PRETTY_EXPR
                if (auto lhs = dynamic_cast<const bin_expr *>(&be->lhs))
                    if (lhs->precedence < be->precedence)
                        os << ")";
#else
                os << ")";
#endif
                os << be->op;
#ifdef PRETTY_EXPR
                if (auto rhs = dynamic_cast<const bin_expr *>(&be->rhs))
                    if (rhs->precedence < be->precedence)
                        os << "(";
                if (dynamic_cast<const unary_expr *>(&be->rhs))
                    os << "(";
#else
                os << "(";
#endif
                emit_expr(os, be->rhs, symbolic);
#ifdef PRETTY_EXPR
                if (auto rhs = dynamic_cast<const bin_expr *>(&be->rhs))
                    if (rhs->precedence < be->precedence)
                        os << ")";
                if (dynamic_cast<const unary_expr *>(&be->rhs))
                    os << ")";
#else
                os << ")";
#endif
            }
            else if (auto ue = dynamic_cast<const unary_expr *>(e)) {
                os << ue->op;
#ifdef PRETTY_EXPR
                if (dynamic_cast<const unary_expr *>(&ue->expr)
                        || dynamic_cast<const bin_expr *>(&ue->expr))
                    os << '(';
#else
                os << "(";
#endif
                emit_expr(os, ue->expr, symbolic);
#ifdef PRETTY_EXPR
                if (dynamic_cast<const unary_expr *>(&ue->expr)
                        || dynamic_cast<const bin_expr *>(&ue->expr))
                    os << ')';
#else
                os << ")";
#endif
            }
            else if (auto val = dynamic_cast<const value *>(e)) {
                os << val->val;
            }
            else if (auto id = dynamic_cast<const identifier *>(e)) {
                if (symbolic && is_var(id->name)) {
                    os << "sym_" << id->name;
                }
                else {
                    if (is_param(id->name) || is_var(id->name))
                        os << id->name;
                    else {
                        error("Undefined identifier " + id->name);
                    }
                }
            }
            else if (auto lap = dynamic_cast<const lap_expr *>(e)) {
                os << "lap(";
                emit_expr(os, lap->expr, symbolic);
                os << ")";
            }
            else if (auto f = dynamic_cast<const func *>(e)) {
                if (f->name == "pow"
                        || f->name == "sin"
                        || f->name == "cos") {
                    os << f->name << "(";
                    int i = 0;
                    for (auto arg: f->args) {
                        if (i > 0)
                            os << ", ";
                        emit_expr(os, *arg, symbolic);
                        i++;
                    }
                    os << ")";
                }
                else {
                    error(std::string("function ") + f->name + " not yet handled\n");
                }
            }
            else {
                expr.display("Term skipped");
                error("Term skipped");
            }
        }

        bool is_param(const std::string& name) {
            for (auto p: params) {
                if (p.first == name) return true;
            }
            return false;
        }

        bool is_var(const std::string& name) {
            for (auto var: vars) {
                if (var->name == name)
                    return true;
            }
            return false;
        }

        void get_vars(const expr& expr,
                std::vector<const identifier *>& vars) {

            const class expr *e = &expr;
            if (auto id = dynamic_cast<const identifier *>(e)) {
                if (is_var(id->name)) {
                    bool add = true;
                    for (auto var: vars) {
                        if (id->name == var->name)
                            add = false;
                    }
                    if (add)
                        vars.push_back(id);
                }
            }

            for (auto c: expr.children) {
                if (auto ex = dynamic_cast<const class expr *>(c.get())) {
                    get_vars(*ex, vars);
                }
            }
        }

        void emit_bc(std::ostream& os,
                const std::string& eq_name,
                int location,
                const expr& bc) {

            if (auto de = dynamic_cast<const diff_expr *>(&bc)) {
                if (auto id = dynamic_cast<const identifier *>(&de->expr)) {
                    if (de->id.name == "r") {
                        switch (location) {
                            case CENTER:
                                os << "    op->bc_bot2_add_l(0, \""
                                    << eq_name << "\", \"" << id->name << "\", "
                                    << "ones(1, 1), map.D.block(0).row(0));\n";
                                break;
                            case SURFACE:
                                os << "    op->bc_top1_add_l(0, \""
                                    << eq_name << "\", \"" << id->name << "\", "
                                    << "ones(1, 1), map.D.block(-1).row(-1));\n";
                                break;
                            case TOP:
                            case BOTTOM:
                                TODO;
                                break;
                            default:
                                error("Unknown BC location "
                                        + std::to_string(location));
                        }
                    }
                    else {
                        error("Cannot differentiate wrt " + de->id.name
                                + " in boundary conditions");
                    }
                }
                else {
                    TODO;
                }
            }
            else if (auto be = dynamic_cast<const bin_expr *>(&bc)) {
                switch (be->op) {
                    case '+':
                        emit_bc(os, eq_name, location, be->lhs);
                        emit_bc(os, eq_name, location, be->rhs);
                        break;
                    default:
                        TODO;
                }
            }
            else if (auto id = dynamic_cast<const identifier *>(&bc)) {
                if (is_var(id->name)) {
                    switch (location) {
                        case CENTER:
                            os << "    op->bc_bot2_add_d(0, \""
                                << eq_name << "\", \"" << id->name << "\", "
                                << "ones(1, 1));\n";
                            break;
                        case SURFACE:
                            os << "    op->bc_top1_add_d(0, \""
                                << eq_name << "\", \"" << id->name << "\", "
                                << "ones(1, 1));\n";
                            break;
                        case TOP:
                        case BOTTOM:
                            TODO;
                            break;
                        default:
                            error("Unknown BC location "
                                    + std::to_string(location));
                    }
                }
                else {
                    error("Only variables are allowed in BC");
                }
            }
            else {
                TODO;
            }
        }

    private:
        std::vector<std::shared_ptr<const ir::variable>> vars; 
        std::vector<std::shared_ptr<const ir::equation>> eqs; 

        std::map<std::string, std::string> params; 
};

}

#endif
