#include "ir.hpp" 

namespace ir {


// class expr
bool expr::operator!=(const expr& e) const {
    return !((*this) == e);
}

expr::~expr() { }

bool expr::has_field_value() const {
    return false;
}


// class value
value::value(const double& val) : val(val) { }
value::~value() { }

std::shared_ptr<const expr> value::copy() const {
    return std::make_shared<const value>(val);
}

bool value::operator==(const expr& e) const {
    try {
        const ir::value& val = dynamic_cast<const value&>(e);
        return this->val == val.val;
    }
    catch (const std::bad_cast& e) {
        return false;
    }
}

value::operator std::string() const {
    return std::string("VAL: ") + std::to_string(val);
}

bool value::has_field_value() const {
    return false;
}

// class identifier
identifier::identifier(const std::string& name) : name(name) { }
identifier::~identifier() { }

std::shared_ptr<const expr> identifier::copy() const {
    return std::make_shared<const identifier>(name);
}

bool identifier::operator==(const expr& e) const {
    try {
        const ir::identifier& id = dynamic_cast<const identifier&>(e);
        return name == id.name;
    }
    catch (const std::bad_cast& e) {
        return false;
    }
}
identifier::operator std::string() const {
    return std::string("ID: ") + name;
}

// class delta
delta::delta(const std::string& name) : identifier(name) { }
delta::delta(const delta& d) : delta(d.name) { }
delta::~delta() { }
std::shared_ptr<const expr> delta::copy() const {
    return std::make_shared<const delta>(*this);
}

delta::operator std::string() const {
    return std::string("delta: ") + name;
}

// class field_value
field_value::field_value(const std::string& name,
        std::shared_ptr<const expr> index) : identifier(name), index(*index) {
    add_child(index);
}
field_value::field_value(const field_value& fv)
    : field_value(fv.name, fv.index.copy()) { }

std::shared_ptr<const expr> field_value::copy() const {
    return std::make_shared<const field_value>(*this);
}
field_value::~field_value() {}

bool field_value::operator==(const expr& e) const {
    try {
        const ir::field_value& fv = dynamic_cast<const field_value&>(e);
        return name == fv.name && index == fv.index;
    }
    catch (const std::bad_cast& e) {
        return false;
    }
}

field_value::operator std::string() const {
    return std::string("FV: ") + name + "[]";
}

bool field_value::has_field_value() const {
    return true;
}

// class bin_expr{
bin_expr::bin_expr(std::shared_ptr<const expr> l,
        char op, std::shared_ptr<const expr> r)
    : lhs(*l), rhs(*r), op(op), precedence(op_prec(op)) {

    add_child(l);
    add_child(r);
}

bin_expr::bin_expr(const bin_expr& be)
    : bin_expr(be.lhs.copy(), be.op, be.rhs.copy()) { }

bin_expr::~bin_expr() { }

std::shared_ptr<const expr> bin_expr::copy() const {
    return std::make_shared<const bin_expr>(*this);
}

bool bin_expr::operator==(const expr& e) const {
    try {
        const ir::bin_expr& be = dynamic_cast<const bin_expr&>(e);
        return op == be.op
            && lhs == be.lhs
            && rhs == be.rhs;
    }
    catch (const std::bad_cast& e) {
        return false;
    }
}
bin_expr::operator std::string() const {
    return std::string("BE: ") + op;
}

bool bin_expr::has_field_value() const {
    return lhs.has_field_value() || rhs.has_field_value();
}

// class unary_expr
unary_expr::unary_expr(char op, std::shared_ptr<const class expr> e)
    : expr(*e), op(op), precedence(op_prec(op)) {

    add_child(e);
}

unary_expr::unary_expr(const unary_expr& ue) : unary_expr(ue.op, ue.expr.copy()) { }

unary_expr::~unary_expr() { }

std::shared_ptr<const expr> unary_expr::copy() const {
    return std::make_shared<const unary_expr>(*this);
}

bool unary_expr::operator==(const class expr& e) const {
    try {
        const ir::unary_expr& ue = dynamic_cast<const unary_expr&>(e);
        return op == ue.op
            && expr == ue.expr;
    }
    catch (const std::bad_cast& e) {
        return false;
    }
}

unary_expr::operator std::string() const {
    return std::string("UE: ") + op;
}

bool unary_expr::has_field_value() const {
    return expr.has_field_value();
}

// class func
func::func(const std::string& name) : name(name) { }

func::func(const std::string& name, std::shared_ptr<const expr> e) : name(name) {
    add_child(e);
    args.push_back(e);
}

func::func(const std::string& name,
        std::vector<std::shared_ptr<const expr>> args) : name(name) {
    for (auto arg: args) {
        add_child(arg);
        this->args.push_back(arg);
    }
}

func::func(const func& f) : func(f.name) {
    for (auto arg: f.args) {
        std::shared_ptr<const expr> arg_cpy = arg->copy();
        add_child(arg_cpy);
        args.push_back(arg_cpy);
    }
}

std::shared_ptr<const expr> func::copy() const {
    return std::make_shared<const func>(*this);
}

bool func::operator==(const class expr& e) const {
    bool same_args = true;

    try {
        const ir::func& f = dynamic_cast<const func&>(e);
        if (f.args.size() != args.size()) return false;

        for (size_t i=0; i<args.size(); i++) {
            same_args  = same_args || (f.args[i] == args[i]);
        }
        return same_args && f.name == name;
    }
    catch (const std::bad_cast& e) {
        return false;
    }
}

func::~func() { }

func::operator std::string() const {
    return std::string("FUNC: ") + name;
}


// class div_expr
div_expr::div_expr(std::shared_ptr<const class expr> e) : expr(*e) {
    add_child(e);
}

div_expr::div_expr(const div_expr& de) : div_expr(de.expr.copy()) {
}

div_expr:: ~div_expr() { }

div_expr::operator std::string() const {
    return std::string("DIV: ");
}

bool div_expr::operator==(const class expr& e) const {
    TODO;
}

std::shared_ptr<const expr> div_expr::copy() const {
    return std::make_shared<const div_expr>(*this);
}

// class grad_expr
grad_expr::grad_expr(std::shared_ptr<const class expr> e) : expr(*e) {
    add_child(e);
}

grad_expr::grad_expr(const grad_expr& ge) : grad_expr(ge.expr.copy()) { }

grad_expr::~grad_expr() { }

grad_expr::operator std::string() const {
    return std::string("GRAD: ");
}

bool grad_expr::operator==(const class expr& e) const {
    TODO;
}

std::shared_ptr<const expr> grad_expr::copy() const {
    return std::make_shared<const grad_expr>(*this);
}


// class lap_expr
lap_expr::lap_expr(std::shared_ptr<const class expr> e) : expr(*e) {
    add_child(e);
}

lap_expr::lap_expr(const lap_expr& le) : lap_expr(le.expr.copy()) { }

lap_expr::~lap_expr() { }

lap_expr::operator std::string() const {
    return std::string("LAP: ");
}

bool lap_expr::operator==(const class expr& e) const {
    TODO;
}

std::shared_ptr<const expr> lap_expr::copy() const {
    return std::make_shared<const lap_expr>(*this);
}


// class diff_expr
diff_expr::diff_expr(std::shared_ptr<const class expr> e,
        std::shared_ptr<const identifier> id) : expr(*e), id(*id) {
    add_child(e);
    add_child(id);
}

diff_expr::diff_expr(const diff_expr& de) : diff_expr(de.expr.copy(),
        std::make_shared<const identifier>(de.id.name)) { }

diff_expr::~diff_expr() { }

diff_expr::operator std::string() const {
    return std::string("DIFF: ");
}

bool diff_expr::operator==(const class expr& e) const {
    try {
        const ir::diff_expr& de = dynamic_cast<const diff_expr&>(e);
        return this->expr == de.expr
            && this->id == de.id;
    }
    catch (const std::bad_cast& e) {
        return false;
    }
}

std::shared_ptr<const expr> diff_expr::copy() const {
    return std::make_shared<const diff_expr>(*this);
}

// class equation
equation::equation(const std::string name,
        std::shared_ptr<const expr> lhs,
        std::shared_ptr<const expr> rhs)
    : name(name), lhs(*lhs), rhs(*rhs) {

    add_child(lhs);
    add_child(rhs);
}

equation::operator std::string() const {
    return std::string("EQ: ") + name;
}

// class bc
bc::bc(std::shared_ptr<const equation> cond, const int& loc)
    : eq(*cond), bc_loc(loc) {

    add_child(cond);
}

bc::operator std::string() const {
    switch (bc_loc) {
        case CENTER:
            return std::string("BC at CENTER");
        case SURFACE:
            return std::string("BC at SURFACE");
        case TOP:
            return std::string("TOP IC");
        case BOTTOM:
            return std::string("BOTTOM IC");
        default:
            error("Unknown boundary location");
    }
}


func sin(const expr& e) {
    return func("sin", e.copy());
}

func cos(const expr& e) {
    return func("cos", e.copy());
}

div_expr div(const expr& e) {
    return div_expr(e.copy());
}

grad_expr grad(const expr& e) {
    return grad_expr(e.copy());
}

lap_expr lap(const expr& e) {
    return lap_expr(e.copy());
}

func pow(const expr& e, int p) {
    std::vector<std::shared_ptr<const expr>> args;
    args.push_back(e.copy());
    args.push_back(std::make_shared<const value>((double) p));
    return ir::func("pow", args);
}


} // end name ir

ir::bin_expr operator+(const ir::expr& l, const ir::expr& r) {
    return ir::bin_expr(l.copy(), '+', r.copy());
}

ir::bin_expr operator*(const ir::expr& l, const ir::expr& r) {
    return ir::bin_expr(l.copy(), '*', r.copy());
}

ir::bin_expr operator-(const ir::expr& l, const ir::expr& r) {
    return ir::bin_expr(l.copy(), '-', r.copy());
}

ir::bin_expr operator/(const ir::expr& l, const ir::expr& r) {
    return ir::bin_expr(l.copy(), '/', r.copy());
}

ir::unary_expr operator-(const ir::expr& e) {
    return ir::unary_expr('-', e.copy());
}

