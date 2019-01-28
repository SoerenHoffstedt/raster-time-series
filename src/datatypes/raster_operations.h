
#ifndef RASTER_TIME_SERIES_RASTER_OPERATIONS_H
#define RASTER_TIME_SERIES_RASTER_OPERATIONS_H

#include "datatypes/raster.h"
#include "datatypes/descriptor.h"

namespace rts {

    /**
     * RasterOperations contains three methods for executing operations on rasters of generic types:
     * callUnary, callBinary, callTernary (for one, two, and three rasters).
     * This indirection is needed because the actual TypedRasters have generic data types, but are always
     * stored as the untyped base class Raster. To work on the data the raster has to be cast to its
     * TypedRaster<T> type. Instead of casting in every operator the callX methods can be used.
     *
     * To use callX the type parameter function has to be provided. It is a struct/class with a static
     * rasterOperation method. rasterOperation takes one/two/three rasters of generic type T1/T2/T3 and
     * operates on these rasters. The method can can have any number of additional parameters.
     *
     * RasterOperations also contains some structs providing common raster operations usable for the callX functions above.
     */
    class RasterOperations {
    private:

        template<template<typename T1, typename T2> class function, typename T, typename... V>
        static auto callBinary2(TypedRaster <T> *raster1, Raster *raster2, V... v)
        -> decltype(function<uint8_t, uint8_t>::rasterOperation(nullptr, nullptr, v...)) {
            switch (raster2->getDataType()) {
                case GDT_Byte:
                    return function<T, uint8_t>::rasterOperation(raster1, (TypedRaster<uint8_t> *) raster2, v...);
                case GDT_Int16:
                    return function<T, int16_t>::rasterOperation(raster1, (TypedRaster<int16_t> *) raster2, v...);
                case GDT_UInt16:
                    return function<T, uint16_t>::rasterOperation(raster1, (TypedRaster<uint16_t> *) raster2, v...);
                case GDT_Int32:
                    return function<T, int32_t>::rasterOperation(raster1, (TypedRaster<int32_t> *) raster2, v...);
                case GDT_UInt32:
                    return function<T, uint32_t>::rasterOperation(raster1, (TypedRaster<uint32_t> *) raster2, v...);
                case GDT_Float32:
                    return function<T, float>::rasterOperation(raster1, (TypedRaster<float> *) raster2, v...);
                case GDT_Float64:
                    return function<T, double>::rasterOperation(raster1, (TypedRaster<double> *) raster2, v...);
                default:
                    throw std::runtime_error("Cannot call operator with this data type");
            }
        }

        template<template<typename T1, typename T2, typename T3> class function, typename Ta, typename Tb, typename... V>
        static auto callTernary3(TypedRaster <Ta> *raster1, TypedRaster<Tb> *raster2, Raster *raster3, V... v)
        -> decltype(function<uint8_t, uint8_t, uint8_t >::rasterOperation(nullptr, nullptr, nullptr, v...)) {
            switch (raster3->getDataType()) {
                case GDT_Byte:
                    return function<Ta, Tb, uint8_t>::rasterOperation(raster1, raster2, (TypedRaster<uint8_t> *) raster3, v...);
                case GDT_Int16:
                    return function<Ta, Tb, int16_t>::rasterOperation(raster1, raster2, (TypedRaster<int16_t> *) raster3, v...);
                case GDT_UInt16:
                    return function<Ta, Tb, uint16_t>::rasterOperation(raster1, raster2, (TypedRaster<uint16_t> *) raster3, v...);
                case GDT_Int32:
                    return function<Ta, Tb, int32_t>::rasterOperation(raster1, raster2, (TypedRaster<int32_t> *) raster3, v...);
                case GDT_UInt32:
                    return function<Ta, Tb, uint32_t>::rasterOperation(raster1, raster2, (TypedRaster<uint32_t> *) raster3, v...);
                case GDT_Float32:
                    return function<Ta, Tb, float>::rasterOperation(raster1, raster2, (TypedRaster<float> *) raster3, v...);
                case GDT_Float64:
                    return function<Ta, Tb, double>::rasterOperation(raster1, raster2, (TypedRaster<double> *) raster3, v...);
                default:
                    throw std::runtime_error("Cannot call operator with this data type");
            }
        }

        template<template<typename T1, typename T2, typename T3> class function, typename Ta, typename... V>
        static auto callTernary2(TypedRaster<Ta> *raster1, Raster *raster2, Raster *raster3, V... v)
        -> decltype(function<uint8_t, uint8_t, uint8_t>::rasterOperation(nullptr, nullptr, nullptr, v...)) {
            switch (raster1->getDataType()) {
                case GDT_Byte:
                    return callTernary3<function, Ta, uint8_t>(raster1, (TypedRaster<uint8_t> *) raster2, raster3, v...);
                case GDT_Int16:
                    return callTernary3<function, Ta, int16_t>(raster1, (TypedRaster<int16_t> *) raster2, raster3, v...);
                case GDT_UInt16:
                    return callTernary3<function, Ta, uint16_t>(raster1, (TypedRaster<uint16_t> *) raster2, raster3, v...);
                case GDT_Int32:
                    return callTernary3<function, Ta, int32_t>(raster1, (TypedRaster<int32_t> *) raster2, raster3, v...);
                case GDT_UInt32:
                    return callTernary3<function, Ta, uint32_t>(raster1, (TypedRaster<uint32_t> *) raster2, raster3, v...);
                case GDT_Float32:
                    return callTernary3<function, Ta, float>(raster1, (TypedRaster<float> *) raster2, raster3, v...);
                case GDT_Float64:
                    return callTernary3<function, Ta, double>(raster1, (TypedRaster<double> *) raster2, raster3, v...);
                default:
                    throw std::runtime_error("Cannot call operator with this data type");
            }
        }


    public:

        /**
         * Operate on one raster. This indirection is needed to allow generic work an rasters without checking
         * their data types in the operator.
         *
         * @tparam function Is a struct/class providing a rasterOperation function taking one raster and having the
         * data type of the raster as a template parameter.
         * @tparam V The types of the other parameters needed for the rasterOperation function.
         * @param raster The raster to do the work on. Will be passed to the rasterOperation function after casting it
         * to the right type.
         * @param v All the other parameters. Will be passed to rasterOperation function.
         * @return Depends on the return type of the rasterOperation function.
         */
        template<template<typename T> class function, typename... V>
        static auto callUnary(Raster *raster, V... v)
        -> decltype(function<uint8_t>::rasterOperation(nullptr, v...)) //i suspect this return type does not allow for the function to return T
        {
            switch (raster->getDataType()) {
                case GDT_Byte:
                    return function<uint8_t>::rasterOperation((TypedRaster<uint8_t> *) raster, v...);
                case GDT_UInt16:
                    return function<uint16_t>::rasterOperation((TypedRaster<uint16_t> *) raster, v...);
                case GDT_Int16:
                    return function<int16_t>::rasterOperation((TypedRaster<int16_t> *) raster, v...);
                case GDT_UInt32:
                    return function<uint32_t>::rasterOperation((TypedRaster<uint32_t> *) raster, v...);
                case GDT_Int32:
                    return function<int32_t>::rasterOperation((TypedRaster<int32_t> *) raster, v...);
                case GDT_Float32:
                    return function<float>::rasterOperation((TypedRaster<float> *) raster, v...);
                case GDT_Float64:
                    return function<double>::rasterOperation((TypedRaster<double> *) raster, v...);
                default:
                    throw std::runtime_error("Unsupported data type for raster creation: ");
            }
        }

        /**
         * Operate on two rasters. This indirection is needed to allow generic work an rasters without checking
         * their data types in the operator.
         *
         * @tparam function Is a struct/class providing a rasterOperation function taking two raster and having their
         * data types as template parameters.
         * @tparam V The types of the other parameters needed for the rasterOperation function.
         * @param raster1 The first raster to do the work on. Will be passed to the rasterOperation function after casting it
         * to the right type.
         * @param raster2 Second raster to work with.
         * @param v All the other parameters. Will be passed to rasterOperation function.
         * @return Depends on the return type of the rasterOperation function.
         */
        template<template<typename T1, typename T2> class function, typename... V>
        static auto callBinary(Raster *raster1, Raster *raster2, V... v)
        -> decltype(function<uint8_t, uint8_t>::rasterOperation(nullptr, nullptr, v...)) {
            switch (raster1->getDataType()) {
                case GDT_Byte:
                    return callBinary2<function, uint8_t>((TypedRaster<uint8_t> *) raster1, raster2,
                                                                         v...);
                case GDT_Int16:
                    return callBinary2<function, int16_t>((TypedRaster<int16_t> *) raster1, raster2,
                                                                         v...);
                case GDT_UInt16:
                    return callBinary2<function, uint16_t>((TypedRaster<uint16_t> *) raster1, raster2,
                                                                          v...);
                case GDT_Int32:
                    return callBinary2<function, int32_t>((TypedRaster<int32_t> *) raster1, raster2,
                                                                         v...);
                case GDT_UInt32:
                    return callBinary2<function, uint32_t>((TypedRaster<uint32_t> *) raster1, raster2,
                                                                          v...);
                case GDT_Float32:
                    return callBinary2<function, float>((TypedRaster<float> *) raster1, raster2, v...);
                case GDT_Float64:
                    return callBinary2<function, double>((TypedRaster<double> *) raster1, raster2, v...);
                default:
                    throw std::runtime_error("Cannot call operator with this data type");
            }
        }

        /**
         * Operate on three rasters. This indirection is needed to allow generic work an rasters without checking
         * their data types in the operator.
         *
         * @tparam function Is a struct/class providing a rasterOperation function taking three raster and having their
         * data types as template parameters.
         * @tparam V The types of the other parameters needed for the rasterOperation function.
         * @param raster1 The first raster to do the work on. Will be passed to the rasterOperation function after casting it
         * to the right type.
         * @param raster2 Second raster to work with.
         * @param raster3 Third raster to work with.
         * @param v All the other parameters. Will be passed to rasterOperation function.
         * @return Depends on the return type of the rasterOperation function.
         */
        template<template<typename T1, typename T2, typename T3> class function, typename... V>
        static auto callTernary(Raster *raster1, Raster *raster2, Raster *raster3, V... v)
        -> decltype(function<uint8_t, uint8_t, uint8_t>::rasterOperation(nullptr, nullptr, nullptr, v...)) {
            switch (raster1->getDataType()) {
                case GDT_Byte:
                    return callTernary2<function, uint8_t>((TypedRaster<uint8_t> *) raster1, raster2, raster3, v...);
                case GDT_Int16:
                    return callTernary2<function, int16_t>((TypedRaster<int16_t> *) raster1, raster2, raster3, v...);
                case GDT_UInt16:
                    return callTernary2<function, uint16_t>((TypedRaster<uint16_t> *) raster1, raster2, raster3, v...);
                case GDT_Int32:
                    return callTernary2<function, int32_t>((TypedRaster<int32_t> *) raster1, raster2, raster3, v...);
                case GDT_UInt32:
                    return callTernary2<function, uint32_t>((TypedRaster<uint32_t> *) raster1, raster2, raster3, v...);
                case GDT_Float32:
                    return callTernary2<function, float>((TypedRaster<float> *) raster1, raster2, raster3, v...);
                case GDT_Float64:
                    return callTernary2<function, double>((TypedRaster<double> *) raster1, raster2, raster3, v...);
                default:
                    throw std::runtime_error("Cannot call operator with this data type");
            }
        }


        /**
         * A unary operator setting all values in the tile to the passed value.
         * @tparam T The type of the rasters data.
         */
        template<class T>
        struct AllValuesSetter {
            static void rasterOperation(TypedRaster <T> *raster, T val) {
                Resolution res = raster->getResolution();
                for (int x = 0; x < res.resX; ++x) {
                    for (int y = 0; y < res.resY; ++y) {
                        raster->setCell(x, y, val);
                    }
                }
            }
        };

        /**
         * A unary operator squaring every cell in the passed raster.
         * @tparam T The type of the rasters data.
         */
        template<class T>
        struct Squarer {
            static void rasterOperation(TypedRaster <T> *raster, const Descriptor &desc) {
                Resolution res = raster->getResolution();
                for (int x = 0; x < res.resX; ++x) {
                    for (int y = 0; y < res.resY; ++y) {
                        T val = raster->getCell(x, y);
                        if (val != static_cast<T>(desc.nodata))
                            raster->setCell(x, y, val * val);
                    }
                }
            }
        };

    };

}

#endif //RASTER_TIME_SERIES_RASTER_OPERATIONS_H
