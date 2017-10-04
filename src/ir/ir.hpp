#ifndef IR_H
#define IR_H

#include "log.hpp"

#include <string>
#include <vector>
#include <memory>


#define TODO    error("not yet implemented");

namespace ir {

///
/// \brief Base class used for internal representation
///
/// All nodes of an AST should derive from this class.
///
class ast {

    public:
        /// \brief Constructor
        ast() { nodes++; }
        ast(const ast& a) = delete;

        /// \brief Destructor
        virtual ~ast() { nodes--; }

        /// \brief Holds the number of currently allocated nodes
        static const int& n_nodes;

        /// \brief Display the AST using GraphViz's dot program
        void display(std::string = "") const;

        /// \brief Casting to string operator
        virtual operator std::string() const = 0;

    protected:
        void add_child(std::shared_ptr<const ast> node) {
            children.push_back(node);
        }

    private:
        static int nodes;
        std::vector<std::shared_ptr<const ast>> children;
        void write_dot(
                std::ostream& os,
                const std::string& title,
                bool root = true) const;

        friend class solver;
};

extern const int& n_nodes;

/// \brief Pure virtual class representing mathematical expressions
class expr : public ast {
    public:
        virtual std::shared_ptr<const expr> copy() const = 0;
        virtual bool operator==(const expr&) const = 0;
        virtual bool operator!=(const expr& e) const;
        virtual ~expr();
        virtual bool has_field_value() const ;
};

/// \brief Class used to represent numerical values
class value : public expr {
    public:
        value(const double& val);
        virtual ~value();
        const double val;

        virtual std::shared_ptr<const expr> copy() const;
        virtual bool operator==(const expr& e) const;
        virtual operator std::string() const;
        virtual bool has_field_value() const ;
};

typedef enum var_type {
    FIELD,
    REAL,
} var_type;

/// \brief Class used to represent identifiers (e.g., variables)
class identifier : public expr {
    public:
        identifier(const std::string& name);
        virtual ~identifier();
        virtual std::shared_ptr<const expr> copy() const;
        virtual bool operator==(const expr& e) const;
        virtual operator std::string() const;

        const std::string name;
};


/// \brief Used to represent functional derivatives
class delta : public identifier {
    public:
        delta(const std::string& name);
        delta(const delta& d);
        virtual ~delta();
        virtual std::shared_ptr<const expr> copy() const;

        virtual operator std::string() const;
};

/// \brief Used to represent value of a field at a particular point
class field_value : public identifier {
    public:
        field_value(const std::string& name, std::shared_ptr<const expr> index);
        field_value(const field_value& fv);
        virtual std::shared_ptr<const expr> copy() const;
        virtual ~field_value();

        virtual bool operator==(const expr& e) const;
        virtual operator std::string() const;

        const expr& index;

        bool has_field_value() const;
};

inline int op_prec(const char c) {
    switch (c) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        default:
            log::err() << "unknown operator precedence for operator " << c;
            std::exit(EXIT_FAILURE);
    }
}

/// \brief Class used to represent binary operation between two expressions
/// (e.g, additions, multiplications)
class bin_expr : public expr {
    public:
        bin_expr(std::shared_ptr<const expr> l,
                char op,
                std::shared_ptr<const expr> r);
        bin_expr(const bin_expr& be);
        virtual ~bin_expr() ;
        virtual std::shared_ptr<const expr> copy() const ;
        virtual bool operator==(const expr& e) const ;
        virtual operator std::string() const ;

        virtual bool has_field_value() const ;

    public:
        const expr& lhs;
        const expr& rhs;
        const char op;

        const int precedence;
};

class unary_expr : public expr {
    public:
        unary_expr(char op, std::shared_ptr<const expr> e);
        unary_expr(const unary_expr& ue);
        virtual ~unary_expr();
        virtual std::shared_ptr<const expr> copy() const;
        virtual bool operator==(const expr& e) const;
        virtual operator std::string() const;

        virtual bool has_field_value() const;

    public:
        const expr& expr;
        const char op;
        const int precedence;
};

class func : public expr {
    public:
        func(const std::string& name);

        func(const std::string& name, std::shared_ptr<const expr> e);

        func(const std::string& name,
                std::vector<std::shared_ptr<const expr>> args);

        func(const func& f);

        virtual std::shared_ptr<const expr> copy() const;
        virtual bool operator==(const expr& e) const;
        virtual ~func();
        virtual operator std::string() const;

        const std::string name;
        std::vector<std::shared_ptr<const expr>> args;

};

class div_expr : public expr {
    public:
        div_expr(std::shared_ptr<const expr> e);
        div_expr(const div_expr& de);
        virtual ~div_expr();
        virtual operator std::string() const;
        virtual bool operator==(const expr& e) const;
        virtual std::shared_ptr<const expr> copy() const;

        const expr& expr;
};

class grad_expr : public expr {
    public:
        grad_expr(std::shared_ptr<const expr> e);
        grad_expr(const grad_expr& ge);
        virtual ~grad_expr();
        virtual operator std::string() const;
        virtual bool operator==(const expr& e) const;
        virtual std::shared_ptr<const expr> copy() const;

        const expr& expr;
};

class lap_expr : public expr {
    public:
        lap_expr(std::shared_ptr<const expr> e);
        lap_expr(const lap_expr& le);
        virtual ~lap_expr();
        virtual operator std::string() const;
        virtual bool operator==(const expr& e) const;
        virtual std::shared_ptr<const expr> copy() const;

        const expr& expr;
};

class diff_expr : public expr {
    public:
        diff_expr(std::shared_ptr<const expr> e,
                std::shared_ptr<const identifier> id);
        diff_expr(const diff_expr& de);
        virtual ~diff_expr();
        virtual operator std::string() const;
        virtual bool operator==(const expr& e) const;
        virtual std::shared_ptr<const expr> copy() const;

        const expr& expr;
        const identifier& id;
};

class bc;
class equation : public ast {
    public:
        equation(const std::string name,
                std::shared_ptr<const expr> lhs,
                std::shared_ptr<const expr> rhs);
        equation(const bc& cond) = delete;
        // equation(const equation& e) : equation(e.name, e.lhs.copy(), e.rhs.copy()) {
        //     for (auto bc: e.bcs) {
        //         add_bc(bc);
        //     }
        // }
        const std::string name;
        const expr& lhs;
        const expr& rhs;
        std::vector<std::shared_ptr<const bc>> bcs;

        void add_bc(std::shared_ptr<const bc> bc);

        virtual operator std::string() const;
};

typedef enum bc_loc {
    CENTER,
    SURFACE,
    TOP,
    BOTTOM,
} bc_loc;

class bc : public ast {
    public:
        bc(std::shared_ptr<const equation> cond, const int& loc);
        bc(const bc& cond) = delete;
        const equation& eq;
        const int bc_loc;
        virtual operator std::string() const;
};

func sin(const expr& e);
func cos(const expr& e);
div_expr div(const expr& e);
grad_expr grad(const expr& e);
lap_expr lap(const expr& e);
func pow(const expr& e, int p);

} // end namespace ir

ir::bin_expr operator+(const ir::expr& l, const ir::expr& r);
ir::bin_expr operator*(const ir::expr& l, const ir::expr& r);
ir::bin_expr operator-(const ir::expr& l, const ir::expr& r);
ir::bin_expr operator/(const ir::expr& l, const ir::expr& r);
ir::unary_expr operator-(const ir::expr& e);

#endif
