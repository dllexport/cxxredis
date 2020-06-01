//
// Created by Mario on 2020/5/19.
//

#pragma once
#include <boost/intrusive_ptr.hpp>
#include <stdint.h>
#include <iostream>
#include <boost/checked_delete.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_member.hpp>

template<class T>
class SmartPtr {
public:
    /**
    * 缺省构造函数
    */
    SmartPtr(): ref_count(0) {
        // std::cout << "  Default constructor " << std::endl;
    }

    /**
    * 不允许拷贝构造，只能使用intrusive_ptr来构造另一个intrusive_ptr
    */
    SmartPtr(SmartPtr<T> const&): ref_count(0) {
        // std::cout << "  Copy constructor..." << std::endl;
    }

    /**
    * 不允许进行赋值操作
    */
    SmartPtr& operator=(SmartPtr const& rhs) {
        // std::cout << "  Assignment operator..." << std::endl;
        return *this;
    }

    /**
    * 递增引用计数（放到基类中以便compiler能找到，否则需要放到boost名字空间中）
    */
    friend void intrusive_ptr_add_ref(SmartPtr<T> const* s) {
        // std::cout << "  intrusive_ptr_add_ref..." << std::endl;
        assert(s->ref_count >= 0);
        assert(s != 0);
        ++s->ref_count;
    }

    /**
    * 递减引用计数
    */
    friend void intrusive_ptr_release(SmartPtr<T> const* s) {
//        std::cout << "  intrusive_ptr_release... " << s->ref_count << std::endl;
        assert(s->ref_count > 0);
        assert(s != 0);
        if (--s->ref_count == 0)
            boost::checked_delete(static_cast<T const*>(s));  //s的实际类型就是T，intrusive_ptr_base<T>为基类
    }

    /**
    * 类似于shared_from_this()函数
    */
    boost::intrusive_ptr<T> self() {
        return boost::intrusive_ptr<T>((T*)this);
    }

    boost::intrusive_ptr<const T> self() const {
        return boost::intrusive_ptr<const T>((T const*)this);
    }

    int refcount() const {
        return ref_count;
    }

private:
    ///should be modifiable even from const intrusive_ptr objects
    mutable uint64_t ref_count;

    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        --ref_count;
        ar & ref_count;
        ++ref_count;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & ref_count;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};