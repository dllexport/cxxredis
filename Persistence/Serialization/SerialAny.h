//
// Created by Mario on 2020/5/27.
//

#pragma once
#include <boost/any.hpp>
#include "../../Primitives/Encoding.h"

class SerialAny {
public:
    template<class Archive>
    static void Save(Archive & ar, const boost::any& any, ENCODING_TYPE encoding) {
        switch (encoding) {
            case ENCODING_TYPE::STRING: {
                auto str = boost::any_cast<std::string>(any);
                ar & boost::serialization::make_nvp("string", str);
            }
        }
    }

    template<class Archive>
    static void Load(Archive & ar, boost::any& any, ENCODING_TYPE encoding) {
        switch (encoding) {
            case ENCODING_TYPE::STRING: {
                std::string str_load;
                ar & boost::serialization::make_nvp("string", str_load);
                any = str_load;
            }
        }
    }
};
