
#ifndef RASTER_TIME_SERIES_OPERATOR_TREE_H
#define RASTER_TIME_SERIES_OPERATOR_TREE_H

#include <vector>
#include <memory>
#include <string>
#include <json/json.h>
#include "operators/generic_operator.h"

namespace rts {

    class GenericOperator;
    class ConsumingOperator;

    /**
     * An OperatorTree is the logical view on the operators of an query. It is used to instantiate an actual
     * operator. Operators can therefore instantiate new versions of a child operator to alter its qrect or params.
     * The lifetime of the OperatorTree object must outlive its instantiated operators.
     */
    class OperatorTree {
    public:

        /**
         * Constructor for creating the top of an operator tree, so it is a consuming operator.
         * @param query The full query json, including the query rectangle definition.
         */
        explicit OperatorTree(const Json::Value &query);

        /**
         * Constructor for creating a sub operator tree, by passing the qrect already parsed by the consuming operator.
         * @param query Json query of this sub operator.
         * @param qrect Query Rectangle of the full query.
         */
        OperatorTree(const Json::Value &query, QueryRectangle &qrect);

        /**
         * Destructor, freeing all child operator trees.
         */
        ~OperatorTree();

        /**
         * Instantiates a new instance of the operator.
         * Will not initialize the operator.
         * @return A unique pointer to the newly instantiated operator of this operator tree.
         */
        std::unique_ptr<GenericOperator> instantiate() const;

        /**
         * Instantiate the consuming operator as starting point of a operator tree.
         * Therefore it already initializes the operators after instantiating.
         * @return A unique pointer to the newly instantiated consuming operator of this operator tree.
         */
        std::unique_ptr<ConsumingOperator> instantiateConsuming() const;

    private:
        std::string operator_name;
        bool isConsuming;
        QueryRectangle qrect;
        Json::Value params;
        std::vector<OperatorTree*> children;

        /**
         * Instantiates all the children/input operators of this operator and inserts them into the children vector.
         * @param sourcesJson Json array defining all child/input operators of this operator.
         */
        void createChildren(const Json::Value &sourcesJson);
    };

}

#endif //RASTER_TIME_SERIES_OPERATOR_TREE_H
