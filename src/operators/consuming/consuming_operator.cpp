
#include "consuming_operator.h"

rts::OptionalDescriptor rts::ConsumingOperator::nextDescriptor() {
    return std::nullopt;
}

rts::OptionalDescriptor rts::ConsumingOperator::getDescriptor(int tileSize) {
    return std::nullopt;
}
