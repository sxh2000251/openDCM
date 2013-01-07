/*
    openDCM, dimensional constraint manager
    Copyright (C) 2012  Stefan Troeger <stefantroeger@gmx.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <boost/test/unit_test.hpp>

#include<opendcm/Core>
#include<opendcm/Module3D>

namespace dcm {

template<>
struct geometry_traits<Eigen::Vector3d> {
    typedef tag::point3D tag;
    typedef modell::XYZ modell;
    typedef orderd_roundbracket_accessor accessor;
};

}

typedef dcm::Kernel<double> Kernel;
typedef dcm::Module3D< mpl::vector< Eigen::Vector3d> > Module3D;
typedef dcm::System<Kernel, Module3D::type> System;

typedef typename Module3D::type<System>::Geometry3D Geometry3D;
typedef boost::shared_ptr<Geometry3D> Geom;

//namespace dcm {
typedef dcm::details::ClusterMath<System> cmath;
//};

BOOST_AUTO_TEST_SUITE(clustermath_suit);

BOOST_AUTO_TEST_CASE(clustermath_scaling) {

    System sys;
    cmath math;

    typename Kernel::Vector3 vec(0,0,0);
    math.initFixMaps();
    new(&math.m_normQ) typename Kernel::Vector3Map(&vec(0));

    for(int i=1; i<100; i++) {

        //add the amount of points
        for(int j=0; j<i; j++) {

            Eigen::Vector3d v = Eigen::Vector3d::Random()*100;
            Geom g(new Geometry3D(v, sys));
            //to set the local value which is used by scaling
            g->setClusterMode(true, false);
            math.addGeometry(g);
        };

        //calculate the scale value for these points
        double scale = math.calculateClusterScale();

        //see if the scale value is correct
        if(i!=1) {
            for(int j=0; j<i; j++) {
                double val = (math.getGeometry()[j]->getPoint() - math.midpoint).norm();
                BOOST_CHECK_GE(val / scale , 0.7999);
                BOOST_CHECK_LE(val / scale , 1.2111);
            };
        } else BOOST_REQUIRE(scale==0);

        //see if we can set arbitrary bigger scale values. no hart checking as currently the alogrithm
        //is not perfect
        math.applyClusterScale(2.*scale, false);
        if(i!=1) {
            for(int j=0; j<i; j++) {
                double val = math.getGeometry()[j]->getPoint().norm();
                BOOST_CHECK_GE(val, 0.7999);
                BOOST_CHECK_LE(val, 1.2111);
            };
        } else BOOST_REQUIRE(scale==0);

        math.clearGeometry();
        math.initFixMaps();
    }
}

BOOST_AUTO_TEST_CASE(clustermath_identityhandling) {

    System sys;
    cmath math;
    
    typename Kernel::Quaternion Q(1,2,3,4);
    typename Kernel::DiffTransform3D trans = Q;
    trans *= typename Kernel::Transform3D::Translation(1,2,3);
    
    //need to init a few things
    typename Kernel::Vector3 vec(0,0,0);
    math.initFixMaps();
    new(&math.m_normQ) typename Kernel::Vector3Map(&vec(0));
    math.m_transform =  trans.rotation();
    math.m_transform *= trans.translation();
    
    typename Kernel::DiffTransform3D transI(trans);
    transI.invert();

    //add two points to the clustermath
    Eigen::Vector3d p1 = Eigen::Vector3d::Random()*100;
    Eigen::Vector3d p2 = Eigen::Vector3d::Random()*100;
    Geom g1(new Geometry3D(p1, sys));
    Geom g2(new Geometry3D(p2, sys));
    //the stuff that is normaly done by map downstream geometry
    g1->m_offset = math.getParameterOffset();
    g1->setClusterMode(true, false);
    g1->transform(transI);
    g2->m_offset = math.getParameterOffset();
    g2->setClusterMode(true, false);
    g2->transform(transI);
    math.addGeometry(g1);
    math.addGeometry(g2);
 
    //check if we have the local values right
    BOOST_CHECK(g1->m_toplocal.isApprox(trans.inverse()*p1, 1e-10));
    BOOST_CHECK(g2->m_toplocal.isApprox(trans.inverse()*p2, 1e-10));
   
    math.resetClusterRotation(trans);    

    //check if the new toplocal is changed to the new transformation and that trans is adjusted
    BOOST_CHECK(g1->m_toplocal.isApprox(trans.inverse()*p1, 1e-10));
    BOOST_CHECK(g2->m_toplocal.isApprox(trans.inverse()*p2, 1e-10));

    
    //see if the downstream processing works
    g1->recalculate(trans);
    g2->recalculate(trans);
    BOOST_CHECK(g1->m_rotated.isApprox(p1,1e-10));
    BOOST_CHECK(g2->m_rotated.isApprox(p2,1e-10));
   
    //this should redo the rotation to original
    math.resetClusterRotation(trans);
    g1->recalculate(trans);
    g2->recalculate(trans);
    
    //let's check if rotations still hold
    BOOST_CHECK(g1->m_toplocal.isApprox(trans.inverse()*p1, 1e-10));
    BOOST_CHECK(g2->m_toplocal.isApprox(trans.inverse()*p2, 1e-10));
    
    //see if we have the same Quaternion as at the begining
    typename Kernel::Quaternion Qinit(1,2,3,4);
    Qinit.normalize();
    BOOST_CHECK(Qinit.isApprox(trans.rotation(), 1e-10));
   
    //see if it works with shifting and scaling    	
    //math.resetClusterRotation(trans);
    math.m_transform = trans;
    math.initMaps();
    typename Kernel::number_type s = math.calculateClusterScale();
    math.applyClusterScale(s, false);
    
    math.recalculate();
    BOOST_CHECK(Kernel::isSame((g1->m_rotated*s-p1).norm(),0.));
    BOOST_CHECK(Kernel::isSame((g2->m_rotated*s-p2).norm(),0.));
    
    math.finishCalculation();
    BOOST_CHECK(Kernel::isSame((g1->m_rotated-p1).norm(),0.));
    BOOST_CHECK(Kernel::isSame((g2->m_rotated-p2).norm(),0.));
    BOOST_CHECK(trans.rotation().isApprox(Qinit, 1e-10));
    
   /* 
    //see if it works with initial translation
    Q.setIdentity();
    math.m_quaternion.setIdentity();
    math.m_normQ.setZero();
    math.initFixMaps();
    math.m_translation<<1,2,3;
    g1->transformInverse(Eigen::Matrix3d::Identity(), -math.m_translation);
    g2->transformInverse(Eigen::Matrix3d::Identity(), -math.m_translation);
    
    s = math.calculateClusterScale();
    math.applyClusterScale(s, false);
    math.recalculate();
    g1->recalculate(1./s);
    g2->recalculate(1./s);
    BOOST_CHECK(Kernel::isSame((g1->m_rotated*s-p1).norm(),0.));
    BOOST_CHECK(Kernel::isSame((g2->m_rotated*s-p2).norm(),0.));
    
    math.finishCalculation();
    g1->recalculate(1.);
    g2->recalculate(1.);
    BOOST_CHECK(Kernel::isSame((g1->m_rotated-p1).norm(),0.));
    BOOST_CHECK(Kernel::isSame((g2->m_rotated-p2).norm(),0.));*/
}

BOOST_AUTO_TEST_SUITE_END();
