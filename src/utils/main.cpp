#include "log.hpp"

void foo() {
    error("This is a test error");
}

int main() {

    log::err() << "This is an error\n";
    log::warn() << "This is a warning\n";
    log::log() << "This is an info\n";

    foo();

    return 0;
}
