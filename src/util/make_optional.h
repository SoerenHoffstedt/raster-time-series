

#ifndef RASTER_TIME_SERIES_MAKE_OPTIONAL_H
#define RASTER_TIME_SERIES_MAKE_OPTIONAL_H

#include <boost/optional.hpp>

namespace rts {

    /**
     * Boost::optional does provide a make_optional method but it is used for moving an already constructed object
     * into the optional object. std::make_optional, that was previously used, allows constructing the object in place.
     * This function provides this functionality and allows creating a boost::optional in one line.
     * It takes a generic list of parameters used for constructing the object, creates a boost::optional
     * and emplaces the parameters into the boost::optional.
     *
     * @tparam T Type that is wrapped into an boost::optional
     * @tparam Args Types of arguments for emplacing in the optional object.
     * @param args Arguments used to constructing T.
     * @return An object of type T wrapped into a boost::optional.
     */
    template<class T, class... Args>
    inline boost::optional<T> make_optional(Args&&... args){
        boost::optional<T> opt;
        opt.emplace(std::forward<Args>(args)...);
        return opt;
    }


}

#endif //RASTER_TIME_SERIES_MAKE_OPTIONAL_H
