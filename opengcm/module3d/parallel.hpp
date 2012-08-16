/*
    openGCM, geometric constraint manager
    Copyright (C) 2012  Stefan Troeger <stefantroeger@gmx.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more detemplate tails.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef GCM_PARALLEL_H
#define GCM_PARALLEL_H

#include "geometry.hpp"

namespace gcm {

//the possible directions
enum Direction { Same, Opposite, Both };

//the calculations( same as we always calculate directions we can outsource the work to this functions)
namespace parallel {

template<typename Kernel, typename T>
inline typename Kernel::number_type calc(T d1,
        T d2,
        Direction dir)  {

    switch(dir) {
        case Same:
            return (d1-d2).norm();
        case Opposite:
            return (d1+d2).norm();
        case Both:
	    if(d1.dot(d2) >= 0) {
	      return (d1-d2).norm();
	    }
	    return (d1+d2).norm();
    }
};


template<typename Kernel, typename T>
inline typename Kernel::number_type calcGradFirst(T d1,
        T d2,
        T dd1,
        Direction dir)  {

    switch(dir) {
        case Same:
            return (d1-d2).dot(dd1) / (d1-d2).norm();
        case Opposite:
            return (d1+d2).dot(dd1) / (d1+d2).norm();
        case Both:
	    if(d1.dot(d2) >= 0) {
	      return ((d1-d2).dot(dd1) / (d1-d2).norm());
	    }
	    return ((d1+d2).dot(dd1) / (d1+d2).norm());
    }
};

template<typename Kernel, typename T>
inline typename Kernel::number_type calcGradSecond(T d1,
        T d2,
        T dd2,
        Direction dir)  {

    switch(dir) {
        case Same:
            return (d1-d2).dot(-dd2) / (d1-d2).norm();
        case Opposite:
            return (d1+d2).dot(dd2) / (d1+d2).norm();
        case Both:
            if(d1.dot(d2) >= 0) {
	      return ((d1-d2).dot(-dd2) / (d1-d2).norm());
	    }
	    return ((d1+d2).dot(dd2) / (d1+d2).norm());
    }
};

template<typename Kernel, typename T>
inline void calcGradFirstComp(T d1,
                              T d2,
                              T grad,
                              Direction dir)  {

    switch(dir) {
        case Same:
            grad = (d1-d2) / (d1-d2).norm();
            return;
        case Opposite:
            grad = (d1+d2) / (d1+d2).norm();
            return;
        case Both:
            assert(false);
    }
};

template<typename Kernel, typename T>
inline void calcGradSecondComp(T d1,
                               T d2,
                               T grad,
                               Direction dir)  {

    switch(dir) {
        case Same:
            grad = (d2-d1) / (d1-d2).norm();
            return;
        case Opposite:
            grad = (d2+d1) / (d1+d2).norm();
            return;
        case Both:
            assert(false);
    }
};

}

template< typename Kernel, typename Tag1, typename Tag2 >
struct Parallel3D {

    typedef typename Kernel::number_type Scalar;
    typedef typename Kernel::VectorMap   Vector;
    Direction m_dir;

    Parallel3D(Direction d = Same) : m_dir(d) {
        //  Base::Console().Message("choosen direction (0=same, 1=opposite): %d\n",m_dir);
    };

    //template definition
    Scalar calculate(Vector& param1,  Vector& param2) {
        assert(false);
    };
    Scalar calculateGradientFirst(Vector& param1, Vector& param2, Vector& dparam1) {
        assert(false);
    };
    Scalar calculateGradientSecond(Vector& param1, Vector& param2, Vector& dparam2) {
        assert(false);
    };
    void calculateGradientFirstComplete(Vector& param1, Vector& param2, Vector& gradient) {
        assert(false);
    };
    void calculateGradientSecondComplete(Vector& param1, Vector& param2, Vector& gradient) {
        assert(false);
    };
};

template< typename Kernel >
struct Parallel3D< Kernel, tag::line3D, tag::line3D > {

    typedef typename Kernel::number_type Scalar;
    typedef typename Kernel::VectorMap   Vector;

    Direction m_dir;

    Parallel3D(Direction d = Same) : m_dir(d) {};

    //template definition
    Scalar calculate(Vector& param1,  Vector& param2) {
        return parallel::calc<Kernel>(param1.template tail<3>(), param2.template tail<3>(), m_dir);
    };
    Scalar calculateGradientFirst(Vector& param1, Vector& param2, Vector& dparam1) {
        return parallel::calcGradFirst<Kernel>(param1.template tail<3>(), param2.template tail<3>(), dparam1.template tail<3>(), m_dir);
    };
    Scalar calculateGradientSecond(Vector& param1, Vector& param2, Vector& dparam2) {
        return parallel::calcGradSecond<Kernel>(param1.template tail<3>(), param2.template tail<3>(), dparam2.template tail<3>(), m_dir);
    };
    void calculateGradientFirstComplete(Vector& param1, Vector& param2, Vector& gradient) {
        gradient.template head<3>().setZero();
        parallel::calcGradFirstComp<Kernel>(param1.template tail<3>(), param2.template tail<3>(), gradient.template tail<3>(), m_dir);
    };
    void calculateGradientSecondComplete(Vector& param1, Vector& param2, Vector& gradient) {
        gradient.template head<3>().setZero();
        parallel::calcGradSecondComp<Kernel>(param1.template tail<3>(), param2.template tail<3>(), gradient.template tail<3>(), m_dir);
    };
};

//planes like lines have the direction as segment 3-5, so we can use the same implementations
template< typename Kernel >
struct Parallel3D< Kernel, tag::plane3D, tag::plane3D > : public Parallel3D<Kernel, tag::line3D, tag::line3D> {
    Parallel3D(Direction d = Same) : Parallel3D<Kernel, tag::line3D, tag::line3D>(d) {};
};
template< typename Kernel >
struct Parallel3D< Kernel, tag::line3D, tag::plane3D > : public Parallel3D<Kernel, tag::line3D, tag::line3D> {
    Parallel3D(Direction d = Same) : Parallel3D<Kernel, tag::line3D, tag::line3D>(d) {};
};

template< typename Kernel >
struct Parallel3D< Kernel, tag::cylinder3D, tag::cylinder3D > {

    typedef typename Kernel::number_type Scalar;
    typedef typename Kernel::VectorMap   Vector;

    Direction m_dir;

    Parallel3D(Direction d = Same) : m_dir(d) { };

    //template definition
    Scalar calculate(Vector& param1,  Vector& param2) {
        return parallel::calc<Kernel>(param1.template segment<3>(3), param2.template segment<3>(3), m_dir);
    };
    Scalar calculateGradientFirst(Vector& param1, Vector& param2, Vector& dparam1) {
        return parallel::calcGradFirst<Kernel>(param1.template segment<3>(3), param2.template segment<3>(3), dparam1.template segment<3>(3), m_dir);
    };
    Scalar calculateGradientSecond(Vector& param1, Vector& param2, Vector& dparam2) {
        return parallel::calcGradSecond<Kernel>(param1.template segment<3>(3), param2.template segment<3>(3), dparam2.template segment<3>(3), m_dir);
    };
    void calculateGradientFirstComplete(Vector& param1, Vector& param2, Vector& gradient) {
        gradient.template head<3>().setZero();
        parallel::calcGradFirstComp<Kernel>(param1.template segment<3>(3), param2.template segment<3>(3), gradient.template segment<3>(3), m_dir);
    };
    void calculateGradientSecondComplete(Vector& param1, Vector& param2, Vector& gradient) {
        gradient.template head<3>().setZero();
        parallel::calcGradSecondComp<Kernel>(param1.template segment<3>(3), param2.template segment<3>(3), gradient.template segment<3>(3), m_dir);
    };
};
}

#endif //GCM_ANGLE
