#include "ir.hpp"

#include <iostream>

#define ptr std::shared_ptr

void test_func() {
    ir::identifier a("a");
    ir::identifier b("b");
    ir::identifier c("c");

    ir::grad_expr e1 = ir::grad(a*b + c);
    e1.display("grad");

}

void build_pb() {
    ir::identifier phi("phi");
    ir::identifier rho("rho");
    ir::lap_expr lap_phi = ir::lap(phi);

    std::shared_ptr<ir::equation> poisson =
        std::make_shared<ir::equation>("poisson", lap_phi.copy(), rho.copy());
    poisson->display("poisson");
    // poisson->add_bc();
}

int main() {

    test_func();
    build_pb();
    log::log() << "Nodes: " << ir::n_nodes << "\n";
    if (ir::n_nodes > 0)
        error("Memory leak!");

    return 0;
}
