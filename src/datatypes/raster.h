
#ifndef RASTER_TIME_SERIES_RASTER_H
#define RASTER_TIME_SERIES_RASTER_H

#include <memory>
#include "datatypes/spatial_temporal_reference.h"
#include <gdal.h>

namespace rts {

    /**
     * Base type for raster/tile data. Contains common functionality, resolution, data type, and more.
     * The actual data is stored in the generic sub type TypedRaster.
     */
    class Raster {
    public:
        /**
         * Creates a TypedRaster based on the passed dataType. The TypedRater<T> will be cast to a generic UniqueRaster.
         * @param dataType Data type for the allocated raster.
         * @param v Other parameters for TypedRaster constructors.
         * @return unique ptr of the base class.
         */
        template<typename... V>
        static std::unique_ptr<Raster> createRaster(GDALDataType dataType, V... v);


        Raster(GDALDataType dataType, int res_x, int res_y);
        Raster(GDALDataType dataType, Resolution res);
        virtual ~Raster() = default;
        //TODO: delete them for now, and check later what is needed. Would prob. need clone/copy functions instead.
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

        /**
         * Prints the first MAX_PRINT_SIZE cells of each dimension of the tile to std::cout. Used for debugging.
         */
        virtual void print() const = 0;

        /**
         * @return The total number of cells of the tile, is res_x * res_y.
         */
        int getDataLength() const;

        /**
         * @return Pointer to the raster/tile data, casted to a void pointer for general usage.
         */
        virtual void *getVoidDataPointer() = 0;

        /**
         * Returns the data pointer, but offsets it by offsetX pixel and offsetY lines.
         * This is important for reading and writing to the data, when the wanted data does not start at (0,0)
         * but has an offset. The actual data pointer is not affected.
         * @param offsetX how many pixel to offset.
         * @param offsetY how many lines of the tile to offset.
         * @return Pointer to the raster data but offset.
         */
        virtual void *getVoidDataPointerOffset(int x, int y) = 0;

        /**
         * @return The data type of the tile.
         */
        GDALDataType getDataType() const;

        /**
         * @return The resolution of the tile.
         */
        Resolution getResolution() const;

        /**
         * @return The size of the data type of the raster in bytes.
         */
        virtual int32_t sizeOfDataType() const = 0;

        /**
         * @return The minimum value possible in the data type of the tile.
         */
        virtual double getValueRangeMin() const = 0;

        /**
         * @return The maximum value possible in the data type of the tile.
         */
        virtual double getValueRangeMax() const = 0;

    protected:
        GDALDataType dataType;
        int res_x;
        int res_y;
        int data_length;
        static constexpr int MAX_PRINT_SIZE = 32;
    };
    using UniqueRaster = std::unique_ptr<Raster>;

    /**
     * A raster with actual data of a specific type. Many functions from Raster class are implemented here.
     * @tparam T The data type of the raster, expressed as C++ type.
     */
    template<class T>
    class TypedRaster : public Raster {
    public:
        TypedRaster(GDALDataType dataType, int res_x, int res_y);
        TypedRaster(GDALDataType dataType, Resolution res);
        TypedRaster(GDALDataType dataType, int res_x, int res_y, void *data_pre);
        TypedRaster(GDALDataType dataType, Resolution res, void *data_pre);
        ~TypedRaster() override;
        void *getVoidDataPointer() override;
        void *getVoidDataPointerOffset(int offsetX, int offsetY) override;
        T *getDataPointer();
        T getCell(int x, int y) const;
        void setCell(int x, int y, T value);
        double getCellDouble(int x, int y) override;
        void setCellDouble(int x, int y, double value) override;
        void print() const override;
        int32_t sizeOfDataType() const override;
        double getValueRangeMin() const override;
        double getValueRangeMax() const override;
    private:
        T* data;
        const bool owns_data;
    };

    template<typename... V>
    std::unique_ptr<Raster> Raster::createRaster(GDALDataType dataType, V... v) {
        Raster *ptr = nullptr;

        switch(dataType){
            case GDT_Byte:
                ptr = static_cast<Raster*>(new TypedRaster<uint8_t>(dataType, v...));
                break;
            case GDT_UInt16:
                ptr = static_cast<Raster*>(new TypedRaster<uint16_t>(dataType, v...));
                break;
            case GDT_Int16:
                ptr = static_cast<Raster*>(new TypedRaster<int16_t>(dataType, v...));
                break;
            case GDT_UInt32:
                ptr = static_cast<Raster*>(new TypedRaster<uint32_t>(dataType, v...));
                break;
            case GDT_Int32:
                ptr = static_cast<Raster*>(new TypedRaster<int32_t>(dataType, v...));
                break;
            case GDT_Float32:
                ptr = static_cast<Raster*>(new TypedRaster<float>(dataType, v...));
                break;
            case GDT_Float64:
                ptr = static_cast<Raster*>(new TypedRaster<double>(dataType, v...));
                break;

            default:
                throw std::runtime_error("Unsupported data type for raster creation.");
        }

        return std::unique_ptr<Raster>(ptr);
    }

    template<class T>
    TypedRaster<T>::TypedRaster(GDALDataType dataType, Resolution res) : Raster(dataType, res), owns_data(true) {
        data = new T[data_length];
    }

    template<class T>
    TypedRaster<T>::TypedRaster(GDALDataType dataType, int res_x, int res_y) : Raster(dataType, res_x, res_y), owns_data(true) {
        data = new T[data_length];
    }

    template<class T>
    TypedRaster<T>::TypedRaster(GDALDataType dataType, int res_x, int res_y, void *data_pre)
            : Raster(dataType, res_x, res_y), owns_data(false) {
        data = (T*)data_pre;
    }

    template<class T>
    TypedRaster<T>::TypedRaster(GDALDataType dataType, Resolution res, void *data_pre)
            : Raster(dataType, res), owns_data(false) {
        data = (T*)data_pre;
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
            if(owns_data)
                delete[] data;
            data = nullptr;
        }
    }

    template<>
    void TypedRaster<unsigned char>::print() const;

    template<class T>
    void TypedRaster<T>::print() const {
        for(int y = 0; y < res_y && y < MAX_PRINT_SIZE; y++){
            for(int x = 0; x < res_x && x < MAX_PRINT_SIZE; x++){
                std::cout << data[x + y * res_x] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    template<class T>
    void *TypedRaster<T>::getVoidDataPointer() {
        return static_cast<void *>(data);
    }

    template<class T>
    int32_t TypedRaster<T>::sizeOfDataType() const {
        return sizeof(T);
    }

    template<class T>
    void *TypedRaster<T>::getVoidDataPointerOffset(int offsetX, int offsetY) {
        T* offset_ptr = data;
        offset_ptr += offsetY * res_x * sizeof(T);
        offset_ptr += offsetX * sizeof(T);
        return static_cast<void*>(offset_ptr);
    }

    template<class T>
    double TypedRaster<T>::getValueRangeMin() const {
        return static_cast<double>(std::numeric_limits<T>::min());
    }

    template<class T>
    double TypedRaster<T>::getValueRangeMax() const {
        return static_cast<double>(std::numeric_limits<T>::max());
    }

}

#endif //RASTER_TIME_SERIES_RASTER_H
