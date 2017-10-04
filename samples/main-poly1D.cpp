#include <ester.h>
#include <iomanip>

extern void create_map(mapping& map);
extern solver *create_solver();

matrix Phi;
matrix Phi0;
matrix Lambda;

int main(int argc, char *arg[]) {
    int it = 0;
    double error = 1.;
    double tol = 1e-12;

    mapping map;
    create_map(map);

    Phi = map.r*map.r;
    Phi0 = zeros(1, 1);
    Lambda = ones(1, 1);

    std::cout << std::scientific;
    std::vector<double> errors;
    while (error > tol) {
        solver *s = create_solver();
        s->solve();

        matrix dPhi = s->get_var("Phi");
        error = max(abs(dPhi));
        errors.push_back(error);
        std::cout << "Iter" << std::setw(3) << ++it << " error: " << error << '\n';

        double relax = 1.;
        if(error>0.01) relax = 0.2; // Relax if error too large

        Phi += relax*dPhi;
        Phi0 += relax*s->get_var("Phi0")(0);
        Lambda += relax*s->get_var("Lambda")(0);

        delete s;

        if (it > 1000) break;
    }

    if (error > tol) {
        printf("No converge\n");
        return 1;
    }

    figure fig("/XSERVE");
    fig.subplot(2, 1);

    fig.plot(map.r, Phi);
    fig.label("r", "Phi", "");

    matrix error_matrix(errors.size(), 1);
    for (size_t i=0; i<errors.size(); i++)
        error_matrix(i) = errors[i];
    fig.plot(error_matrix);
    fig.label("iteration", "Error", "");

    printf("\n");
    printf("Lambda = %f\n", Lambda(0));
    printf("Phi(0) = %f\n", Phi(0));
    printf("Phi(1) = %f\n", Phi(-1));
    printf("Boundary conditions:\n");
    printf("  dPhi/dr(0) = %e\n", (map.D, Phi)(0));
    printf("  dPhi/dr(1) + Phi(1) = %e\n", (map.D, Phi)(-1)+Phi(-1));

    return 0;
}
