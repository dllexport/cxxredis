//
// Created by Mario on 2020/5/27.
//

#pragma once

#include <boost/serialization/serialization.hpp>
#include <boost/intrusive_ptr.hpp>
#include "../../Primitives/Object.h"

BOOST_SERIALIZATION_SPLIT_FREE(boost::intrusive_ptr<Object>)

namespace boost {
    namespace serialization {

        template<class Archive>
        void save(Archive & ar, const boost::intrusive_ptr<Object> & g, const unsigned int version)
        {
            ar & *g;
        }

        template<class Archive>
        void load(Archive & ar,  boost::intrusive_ptr<Object> & g, const unsigned int version)
        {
            auto obj = new Object();
            ar & *obj;
            g = boost::intrusive_ptr<Object>(obj);
        }
    }
}
