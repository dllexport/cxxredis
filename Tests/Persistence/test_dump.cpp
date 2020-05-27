//
// Created by Mario on 2020/5/26.
//
#include <stdio.h>
#include "../../Object.h"
#include <boost/intrusive_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/serialization/split_free.hpp>
//namespace boost {
//namespace serialization {
//
//    template<class Archive, class T>
//    void save(
//        Archive & ar,
//        const boost::intrusive_ptr< T > & t,
//        const unsigned int /* version */
//    ){
//        T* r = t.get();
//        ar << boost::serialization::make_nvp("intrusive_ptr", r);
//    }
//
//    template<class Archive, class T>
//    void load(
//        Archive & ar,
//        boost::intrusive_ptr< T > & t,
//        const unsigned int /* version */
//    ){
//        T* r;
//        ar >> boost::serialization::make_nvp("intrusive_ptr", r);
//        t.reset(r);
//    }
//
//    template<class Archive, class T>
//    void serialize(
//        Archive& ar,
//        boost::intrusive_ptr< T >& t,
//        const unsigned int version
//    ){
//        boost::serialization::split_free(ar, t, version);
//    }
//
//} // namespace serialization
//} // namespace boost

#include <fstream>
#include <boost/serialization/base_object.hpp>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include "../../Database.h"


int main() {


    // create and open a character archive for output
    std::ofstream ofs("dump.rdb");

    // save data to archive
    {
        boost::archive::binary_oarchive oa(ofs);
        // write class instance to archive
        std::vector<std::unordered_map<std::string, boost::intrusive_ptr<Object>>> dbs(16);
        {
            boost::intrusive_ptr<Object> object = new Object(ENCODING_TYPE::STRING, std::string("123"));
            object->SetExpire(std::chrono::system_clock::now());

            boost::intrusive_ptr<Object> object2 = new Object(ENCODING_TYPE::STRING, std::string("123"));
            object->SetExpire(std::chrono::system_clock::now());

            dbs[0].insert({"object", object});
            dbs[0].insert({"object2", object2});
        }
        oa << dbs;
        // archive and stream closed when destructors are called
    }

    std::vector<std::unordered_map<std::string, boost::intrusive_ptr<Object>>> dbs;
    // ... some time later restore the class instance to its orginal state
    std::unordered_map<std::string, boost::intrusive_ptr<Object>> umap2;
    {
        // create and open an archive for input
        std::ifstream ifs("filename");
        boost::archive::binary_iarchive ia(ifs);
        // read class state from archive
        ia >> dbs;
        // archive and stream closed when destructors are called
    }
//    auto str = boost::any_cast<std::string>(newg.any);
    return 0;
}