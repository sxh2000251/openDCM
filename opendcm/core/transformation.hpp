/*
    openDCM, dimensional constraint manager
    Copyright (C) 2013  Stefan Troeger <stefantroeger@gmx.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along
    with this library; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef DCM_TRANSFORMATION_H
#define DCM_TRANSFORMATION_H

#include <cmath>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include <boost/mpl/if.hpp>

namespace dcm {
namespace details {

template<typename Scalar, int Dim>
class Transform {

public:
    typedef Eigen::Matrix<Scalar, Dim, 1> Vector;
    typedef typename boost::mpl::if_c< Dim == 3,
            Eigen::Quaternion<Scalar>,
            Eigen::Rotation2D<Scalar> >::type     Rotation;
    typedef Eigen::Translation<Scalar, Dim>	  Translation;
    typedef Eigen::UniformScaling<Scalar> 	  Scaling;
    typedef typename Rotation::RotationMatrixType RotationMatrix;

protected:
    Rotation   	m_rotation;
    Translation	m_translation;
    Scaling  	m_scale;

public:
    Transform();

    Transform(const Rotation& r);
    Transform(const Translation& t);
    Transform(const Scaling& s);
    Transform(const Rotation& r, const Translation& t);
    Transform(const Rotation& r, const Translation& t, const Scaling& s);

    //access the single parts and manipulate them
    //***********************
    const Rotation& rotation() const;
    template<typename Derived>
    Transform& setRotation(const Eigen::RotationBase<Derived,Dim>& rotation);
    template<typename Derived>
    Transform& rotate(const Eigen::RotationBase<Derived,Dim>& rotation);

    const Translation& translation() const;
    Transform& setTranslation(const Translation& translation);
    Transform& translate(const Translation& translation);

    const Scaling& scaling() const;
    Transform& setScale(const Scaling& scaling);
    Transform& scale(const Scalar& scaling);
    Transform& scale(const Scaling& scaling);
    
    Eigen::Transform<Scalar, Dim, Eigen::AffineCompact> transformation();

    Transform& invert();
    Transform inverse();

    //operators for value manipulation
    //********************************

    Transform& operator=(const Translation& t);
    Transform operator*(const Translation& s) const;
    Transform& operator*=(const Translation& t);

    Transform& operator=(const Scaling& s);
    Transform operator*(const Scaling& s) const;
    Transform& operator*=(const Scaling& s);

    template<typename Derived>
    Transform& operator=(const Eigen::RotationBase<Derived,Dim>& r);
    template<typename Derived>
    Transform operator*(const Eigen::RotationBase<Derived,Dim>& r) const;
    template<typename Derived>
    Transform& operator*=(const Eigen::RotationBase<Derived,Dim>& r);

    Transform operator* (const Transform& other) const;
    Transform& operator*= (const Transform& other);

    //transform Vectors
    //*****************
    template<typename Derived>
    Derived& rotate(Eigen::MatrixBase<Derived>& vec) const;
    template<typename Derived>
    Derived& translate(Eigen::MatrixBase<Derived>& vec) const;
    template<typename Derived>
    Derived& scale(Eigen::MatrixBase<Derived>& vec) const;
    template<typename Derived>
    Derived& transform(Eigen::MatrixBase<Derived>& vec) const;
    template<typename Derived>
    Derived operator*(const Eigen::MatrixBase< Derived >& vec) const;
    template<typename Derived>
    void operator()(Eigen::MatrixBase<Derived>& vec) const;

    //Stuff
    //*****
    bool isApprox(const Transform& other, Scalar prec) const;
    void setIdentity();
    static const Transform Identity();

    Transform& normalize();

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

/* When you overload a binary operator as a member function of a class the overload is used
 * when the first operand is of the class type. For stream operators, the first operand
 * is the stream and not (usually) the custom class.
*/
template<typename charT, typename traits, typename Kernel, int Dim>
std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& os, const dcm::details::Transform<Kernel, Dim>& t);

/**********************************************************************************************************************************
 *
 *                      IMPELEMNTATION
 * 
 * ********************************************************************************************************************************/


template<typename Scalar, int Dim>
Transform<Scalar, Dim>::Transform() : m_rotation(Rotation::Identity()),
    m_translation(Translation::Identity()),
    m_scale(Scaling(1.)) { };

template<typename Scalar, int Dim>
Transform<Scalar, Dim>::Transform(const Rotation& r) : m_rotation(r),
    m_translation(Translation::Identity()),
    m_scale(Scaling(1.)) {
    m_rotation.normalize();
};

template<typename Scalar, int Dim>
Transform<Scalar, Dim>::Transform(const Translation& t) : m_rotation(Rotation::Identity()),
    m_translation(t),
    m_scale(Scaling(1.)) {};

template<typename Scalar, int Dim>
Transform<Scalar, Dim>::Transform(const Scaling& s) : m_rotation(Rotation::Identity()),
    m_translation(Translation::Identity()),
    m_scale(s) {};

template<typename Scalar, int Dim>
Transform<Scalar, Dim>::Transform(const Rotation& r, const Translation& t) : m_rotation(r),
    m_translation(t),
    m_scale(Scaling(1.)) {
    m_rotation.normalize();
};

template<typename Scalar, int Dim>
Transform<Scalar, Dim>::Transform(const Rotation& r, const Translation& t, const Scaling& s) : m_rotation(r),
    m_translation(t),
    m_scale(s) {
    m_rotation.normalize();
};

template<typename Scalar, int Dim>
const typename Transform<Scalar, Dim>::Rotation& Transform<Scalar, Dim>::rotation() const {
    return m_rotation;
}

template<typename Scalar, int Dim>
template<typename Derived>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::setRotation(const Eigen::RotationBase<Derived,Dim>& rotation) {
    m_rotation = rotation.derived().normalized();
    return *this;
}

template<typename Scalar, int Dim>
template<typename Derived>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::rotate(const Eigen::RotationBase<Derived,Dim>& rotation) {
    m_rotation = rotation.derived().normalized()*m_rotation;
    return *this;
}

template<typename Scalar, int Dim>
const typename Transform<Scalar, Dim>::Translation& Transform<Scalar, Dim>::translation() const {
    return m_translation;
}

template<typename Scalar, int Dim>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::setTranslation(const Translation& translation) {
    m_translation = translation;
    return *this;
}

template<typename Scalar, int Dim>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::translate(const Translation& translation) {
    m_translation = m_translation*translation;
    return *this;
}

template<typename Scalar, int Dim>
const typename Transform<Scalar, Dim>::Scaling& Transform<Scalar, Dim>::scaling() const {
    return m_scale;
}
template<typename Scalar, int Dim>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::scale(const Scalar& scaling) {
    m_scale *= Scaling(scaling);
    return *this;
}
template<typename Scalar, int Dim>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::setScale(const Scaling& scaling) {
    m_scale.factor() = scaling.factor();
    return *this;
}
template<typename Scalar, int Dim>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::scale(const Scaling& scaling) {
    m_scale.factor() *= scaling.factor();
    return *this;
}

template<typename Scalar, int Dim>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::invert() {
    m_rotation = m_rotation.inverse();
    m_translation.vector() = (m_rotation*m_translation.vector()) * (-m_scale.factor());
    m_scale = Scaling(1./m_scale.factor());
    return *this;
};
template<typename Scalar, int Dim>
Transform<Scalar, Dim> Transform<Scalar, Dim>::inverse() {
    Transform res(*this);
    res.invert();
    return res;
};

template<typename Scalar, int Dim>
inline Transform<Scalar, Dim>& Transform<Scalar, Dim>::operator=(const Translation& t) {
    m_translation = t;
    m_rotation = Rotation::Identity();
    m_scale = Scaling(1.);
    return *this;
}
template<typename Scalar, int Dim>
inline Transform<Scalar, Dim> Transform<Scalar, Dim>::operator*(const Translation& t) const {
    Transform res = *this;
    res.translate(t);
    return res;
}
template<typename Scalar, int Dim>
inline Transform<Scalar, Dim>& Transform<Scalar, Dim>::operator*=(const Translation& t) {
    return translate(t);
}

template<typename Scalar, int Dim>
inline Transform<Scalar, Dim>& Transform<Scalar, Dim>::operator=(const Scaling& s) {
    m_scale = s;
    m_translation = Translation::Identity();
    m_rotation = Rotation::Identity();
    return *this;
}
template<typename Scalar, int Dim>
inline Transform<Scalar, Dim> Transform<Scalar, Dim>::operator*(const Scaling& s) const {
    Transform res = *this;
    res.scale(s);
    return res;
}
template<typename Scalar, int Dim>
inline Transform<Scalar, Dim>& Transform<Scalar, Dim>::operator*=(const Scaling& s) {
    return scale(s);
}

template<typename Scalar, int Dim>
template<typename Derived>
inline Transform<Scalar, Dim>& Transform<Scalar, Dim>::operator=(const Eigen::RotationBase<Derived,Dim>& r) {
    m_rotation = r.derived();
    m_rotation.normalize();
    m_translation = Translation::Identity();
    m_scale = Scaling(1);
    return *this;
}
template<typename Scalar, int Dim>
template<typename Derived>
inline Transform<Scalar, Dim> Transform<Scalar, Dim>::operator*(const Eigen::RotationBase<Derived,Dim>& r) const {
    Transform res = *this;
    res.rotate(r.derived());
    return res;
}
template<typename Scalar, int Dim>
template<typename Derived>
inline Transform<Scalar, Dim>& Transform<Scalar, Dim>::operator*=(const Eigen::RotationBase<Derived,Dim>& r) {
    return rotate(r.derived());
}

template<typename Scalar, int Dim>
inline Transform<Scalar, Dim> Transform<Scalar, Dim>::operator* (const Transform& other) const  {
    Transform res(*this);
    res*= other;
    return res;
}
template<typename Scalar, int Dim>
inline Transform<Scalar, Dim>& Transform<Scalar, Dim>::operator*= (const Transform& other) {
    rotate(other.rotation());
    other.rotate(m_translation.vector());
    m_translation.vector() += other.translation().vector()/m_scale.factor();
    m_scale.factor() *= other.scaling().factor();
    return *this;
}

template<typename Scalar, int Dim>
template<typename Derived>
inline Derived& Transform<Scalar, Dim>::rotate(Eigen::MatrixBase<Derived>& vec) const {
    vec = m_rotation*vec;
    return vec.derived();
}

template<typename Scalar, int Dim>
template<typename Derived>
inline Derived& Transform<Scalar, Dim>::translate(Eigen::MatrixBase<Derived>& vec) const {
    vec = m_translation*vec;
    return vec.derived();
}

template<typename Scalar, int Dim>
template<typename Derived>
inline Derived& Transform<Scalar, Dim>::scale(Eigen::MatrixBase<Derived>& vec) const {
    vec*=m_scale.factor();
    return vec.derived();
}

template<typename Scalar, int Dim>
template<typename Derived>
inline Derived& Transform<Scalar, Dim>::transform(Eigen::MatrixBase<Derived>& vec) const {
    vec = (m_rotation*vec + m_translation.vector())*m_scale.factor();
    return vec.derived();
}

template<typename Scalar, int Dim>
template<typename Derived>
inline Derived Transform<Scalar, Dim>::operator*(const Eigen::MatrixBase<Derived>& vec) const {
    return (m_rotation*vec + m_translation.vector())*m_scale.factor();
}

template<typename Scalar, int Dim>
template<typename Derived>
inline void Transform<Scalar, Dim>::operator()(Eigen::MatrixBase<Derived>& vec) const {
    transform(vec);
}

template<typename Scalar, int Dim>
bool Transform<Scalar, Dim>::isApprox(const Transform& other, Scalar prec) const {
    return m_rotation.isApprox(other.rotation(), prec)
           && ((m_translation.vector()- other.translation().vector()).norm() < prec)
           && (std::abs(m_scale.factor()-other.scaling().factor()) < prec);
};

template<typename Scalar, int Dim>
void Transform<Scalar, Dim>::setIdentity() {
    m_rotation.setIdentity();
    m_translation = Translation::Identity();
    m_scale = Scaling(1.);
}

template<typename Scalar, int Dim>
const Transform<Scalar, Dim> Transform<Scalar, Dim>::Identity() {
    return Transform(Rotation::Identity(), Translation::Identity(), Scaling(1));
}

template<typename Scalar, int Dim>
Transform<Scalar, Dim>& Transform<Scalar, Dim>::normalize() {
    m_rotation.normalize();
    return *this;
}

template<typename Scalar, int Dim>
Eigen::Transform<Scalar, Dim, Eigen::AffineCompact> Transform<Scalar, Dim>::transformation() {
    return Eigen::Transform<Scalar, Dim, Eigen::AffineCompact>(m_rotation.toRotationMatrix()) *
            Eigen::Transform<Scalar, Dim, Eigen::AffineCompact>(m_translation);
};

/*When you overload a binary operator as a member function of a class the overload is used
 * when the first operand is of the class type.For stream operators, the first operand
 * is the stream and not (usually) the custom class.
*/
template<typename charT, typename traits, typename Kernel, int Dim>
std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& os, const dcm::details::Transform<Kernel, Dim>& t) {
    os << "Rotation:    " << t.rotation().coeffs().transpose() << std::endl
       << "Translation: " << t.translation().vector().transpose() <<std::endl
       << "Scale:       " << t.scaling().factor();
    return os;
}

}//details
}//DCM



#endif //DCM_TRANSFORMATION
