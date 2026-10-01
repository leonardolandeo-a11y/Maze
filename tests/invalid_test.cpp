#include "circuit_escape/controllers.h"

struct InvalidPolicy {
    int selectAction(
        const Observation&,
        std::span<const Action>
    ) {
        return 0;
    }
};

int main() {
    PolicyController<InvalidPolicy> controller{
        InvalidPolicy{}
    };

    return 0;
}