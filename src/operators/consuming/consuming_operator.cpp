
#include "consuming_operator.h"

rts::OptionalDescriptor rts::ConsumingOperator::nextDescriptor() {
    return boost::none;
}

rts::OptionalDescriptor rts::ConsumingOperator::getDescriptor(int tileSize) {
    return boost::none;
}
