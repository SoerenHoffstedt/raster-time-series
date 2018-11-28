
#ifndef RASTER_TIME_SERIES_RASTER_H
#define RASTER_TIME_SERIES_RASTER_H

#include <memory>
#include "datatypes/spatial_temporal_reference.h"
#include <gdal.h>

namespace rts {

    /**
     * Base type for raster/tiles. Contains common functionality, resolution, data type, and more.
     * The actual data is stored in the generic sub type TypedRaster.
     */
    class Raster {
    public:
        /**
         * Creates a TypedRaster based on the passed dataType. The TypedRater<T> will be cast to a generic UniqueRaster.
         * @param dataType Data type for the allocated raster.
         * @param res Resolution of the allocated raster.
         * @return
         */
        static std::unique_ptr<Raster> createRaster(GDALDataType dataType, Resolution res);
        static std::unique_ptr<Raster> createRaster(GDALDataType dataType, int res_x, int res_y);

        Raster(GDALDataType dataType, int res_x, int res_y);
        Raster(GDALDataType dataType, Resolution res);
        virtual ~Raster() = default;
        //TODO: delete them for now, and check later what is needed. Would prob. need clone/copy functions.
        Raster(const Raster &other) = delete;
        Raster& operator=(const Raster &other) = delete;
        Raster(Raster &&other) = delete;
        Raster& operator=(Raster &&other) = delete;

        /**
         * Method for commonly accessing data of the raster independent from the actual data type.
         * A double can contain all available data ranges.
         * @param x width index of the accessed cell.
         * @param y height index of the accessed cell.
         * @return The value at cell (x,y) after casting it to double.
         */
        virtual double getCellDouble(int x, int y) = 0;
        /**
         * Method for commonly setting a cell of the raster independent from the actual data type.
         * A double can contain all available data ranges.
         * @param x width index of the accessed cell.
         * @param y height index of the accessed cell.
         * @param value The value to store at (x,y). It will be cast from double to the actual data type of
         * the raster. So data could be lost.
         */
        virtual void setCellDouble(int x, int y, double value) = 0;
        virtual void print() const = 0;
        int getDataLength() const;
        GDALDataType getDataType() const;
        Resolution getResolution() const;

    protected:
        GDALDataType dataType;
        int res_x;
        int res_y;
        int data_length;
    };
    using UniqueRaster = std::unique_ptr<Raster>;

    /**
     * A raster with actual data of a specific type.
     * @tparam T The data type of the raster, expressed as C++ type.
     */
    template<class T>
    class TypedRaster : public Raster {
    public:
        TypedRaster(GDALDataType dataType, int res_x, int res_y);
        TypedRaster(GDALDataType dataType, Resolution res);
        ~TypedRaster() override;
        T* getDataPointer();
        T getCell(int x, int y) const;
        void setCell(int x, int y, T value);
        double getCellDouble(int x, int y) override;
        void setCellDouble(int x, int y, double value) override;
        void print() const override;
    private:
        T* data;
    };

    template<class T>
    TypedRaster<T>::TypedRaster(GDALDataType dataType, Resolution res) : Raster(dataType, res) {
        data = new T[data_length];
    }

    template<class T>
    TypedRaster<T>::TypedRaster(GDALDataType dataType, int res_x, int res_y) : Raster(dataType, res_x, res_y) {
        data = new T[data_length];
    }

    template<class T>
    T *TypedRaster<T>::getDataPointer() {
        return data;
    }

    template<class T>
    double TypedRaster<T>::getCellDouble(int x, int y) {
        if(data == nullptr)
            throw std::runtime_error("Data of raster is not allocated");
        return (double)data[x + y * res_x];
    }

    template<class T>
    void TypedRaster<T>::setCellDouble(int x, int y, double value) {
        if(data == nullptr)
            throw std::runtime_error("Data of raster is not allocated");
        data[x + y * res_x] = (T)value;
    }

    template<class T>
    T TypedRaster<T>::getCell(int x, int y) const {
        if(data == nullptr)
            throw std::runtime_error("Data of raster is not allocated");
        return data[x + y * res_x];
    }

    template<class T>
    void TypedRaster<T>::setCell(int x, int y, T value) {
        if(data == nullptr)
            throw std::runtime_error("Data of raster is not allocated");
        data[x + y * res_x] = value;
    }

    template<class T>
    TypedRaster<T>::~TypedRaster() {
        if(data != nullptr){
            delete[] data;
            data = nullptr;
        }
    }

    template<class T>
    void TypedRaster<T>::print() const {
        for(int y = 0; y < res_y; y++){
            for(int x = 0; x < res_x; x++){
                std::cout << data[x + y * res_x] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }


}

#endif //RASTER_TIME_SERIES_RASTER_H
