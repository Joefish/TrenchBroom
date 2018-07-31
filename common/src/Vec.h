/*
Copyright (C) 2010-2017 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TrenchBroom_Vec_h
#define TrenchBroom_Vec_h

#include "MathUtils.h"
#include "StringUtils.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <map>
#include <ostream>
#include <set>
#include <vector>

template <typename T, size_t S>
class Vec {
private:
    class SelectionHeapCmp {
    private:
        const Vec<T,S>& m_vec;
        bool m_abs;
    public:
        SelectionHeapCmp(const Vec<T,S>& vec, const bool abs) :
        m_vec(vec),
        m_abs(abs) {}
        
        bool operator()(size_t lhs, size_t rhs) const {
            assert(lhs < S);
            assert(rhs < S);
            if (m_abs)
                return std::abs(m_vec.v[lhs]) < std::abs(m_vec.v[rhs]);
            return m_vec.v[lhs] < m_vec.v[rhs];
        }
    };
    
    int weight(T c) const {
        if (std::abs(c - static_cast<T>(1.0)) < static_cast<T>(0.9))
            return 0;
        if (std::abs(c + static_cast<T>(1.0)) < static_cast<T>(0.9))
            return 1;
        return 2;
    }
public:
    typedef Vec<float, S> FloatType;

    typedef T Type;
    static const size_t Size = S;

    static const Vec<T,S> PosX;
    static const Vec<T,S> PosY;
    static const Vec<T,S> PosZ;
    static const Vec<T,S> NegX;
    static const Vec<T,S> NegY;
    static const Vec<T,S> NegZ;
    static const Vec<T,S> Null;
    static const Vec<T,S> One;
    static const Vec<T,S> NaN;
    static const Vec<T,S> Min;
    static const Vec<T,S> Max;
    
    using List = std::vector<Vec<T,S>>;

    static const List AllAxes;
    static const List PosAxes;
    static const List NegAxes;
    
    static const List EmptyList;
public:
    static const Vec<T,S> axis(const size_t index) {
        Vec<T,S> axis;
        axis[index] = static_cast<T>(1.0);
        return axis;
    }
    
    static Vec<T,S> fill(const T value) {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = value;
        return result;
    }
    
    static Vec<T,S> unit(const size_t index) {
        assert(index < S);
        
        Vec<T,S> result;
        result[index] = static_cast<T>(1.0);
        return result;
    }
    
    template <typename U1>
    static Vec<T,S> create(const U1 i_x) {
        Vec<T,S> result;
        if (S > 0) {
            result[0] = static_cast<T>(i_x);
        }
        for (size_t i = 1; i < S; ++i)
            result[i] = static_cast<T>(0.0);
        return result;
    }
    
    template <typename U1, typename U2>
    static Vec<T,S> create(const U1 i_x, const U2 i_y) {
        Vec<T,S> result;
        if (S > 0) {
            result[0] = static_cast<T>(i_x);
            if (S > 1) {
                result[1] = static_cast<T>(i_y);
            }
        }
        for (size_t i = 2; i < S; ++i)
            result[i] = static_cast<T>(0.0);
        return result;
    }

    template <typename U1, typename U2, typename U3>
    static Vec<T,S> create(const U1 i_x, const U2 i_y, const U3 i_z) {
        Vec<T,S> result;
        if (S > 0) {
            result[0] = static_cast<T>(i_x);
            if (S > 1) {
                result[1] = static_cast<T>(i_y);
                if (S > 2) {
                    result[2] = static_cast<T>(i_z);
                }
            }
        }
        for (size_t i = 3; i < S; ++i)
            result[i] = static_cast<T>(0.0);
        return result;
    }

    template <typename U1, typename U2, typename U3, typename U4>
    static Vec<T,S> create(const U1 i_x, const U2 i_y, const U3 i_z, const U4 i_w) {
        Vec<T,S> result;
        if (S > 0) {
            result[0] = static_cast<T>(i_x);
            if (S > 1) {
                result[1] = static_cast<T>(i_y);
                if (S > 2) {
                    result[2] = static_cast<T>(i_z);
                    if (S > 3) {
                        result[3] = static_cast<T>(i_w);
                    }
                }
            }
        }
        for (size_t i = 4; i < S; ++i)
            result[i] = static_cast<T>(0.0);
        return result;
    }
    
    static Vec<T,S> parse(const std::string& str) {
        size_t pos = 0;
        Vec<T,S> result;
        doParse(str, pos, result);
        return result;
    }
    
    static List parseList(const std::string& str) {
        static const std::string blank(" \t\n\r,;");
        
        size_t pos = 0;
        List result;

        while (pos != std::string::npos) {
            Vec<T,S> temp;
            if (doParse(str, pos, temp))
                result.push_back(temp);
            pos = str.find_first_of(blank, pos);
        }
        
        return result;
    }

private:
    static bool doParse(const std::string& str, size_t& pos, Vec<T,S>& result) {
        static const std::string blank(" \t\n\r()");

        const char* cstr = str.c_str();
        for (size_t i = 0; i < S; ++i) {
            if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos)
                return false;
            result[i] = static_cast<T>(std::atof(cstr + pos));
            if ((pos = str.find_first_of(blank, pos)) == std::string::npos)
                return false;;
        }
        return true;
    }
public:
    T v[S];
    
    Vec() {
        setNull();
    }
            
    template <typename U1, typename U2>
    Vec(const U1 i_x, const U2 i_y) {
        if (S > 0) {
            v[0] = static_cast<T>(i_x);
            if (S > 1)
                v[1] = static_cast<T>(i_y);
        }
        for (size_t i = 2; i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }
    
    template <typename U1, typename U2, typename U3>
    Vec(const U1 i_x, const U2 i_y, const U3 i_z) {
        if (S > 0) {
            v[0] = static_cast<T>(i_x);
            if (S > 1) {
                v[1] = static_cast<T>(i_y);
                if (S > 2)
                    v[2] = static_cast<T>(i_z);
            }
        }
        for (size_t i = 3; i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }

    template <typename U1, typename U2, typename U3, typename U4>
    Vec(const U1 i_x, const U2 i_y, const U3 i_z, const U4 i_w) {
        if (S > 0) {
            v[0] = static_cast<T>(i_x);
            if (S > 1) {
                v[1] = static_cast<T>(i_y);
                if (S > 2) {
                    v[2] = static_cast<T>(i_z);
                    if (S > 3)
                        v[3] = static_cast<T>(i_w);
                }
            }
        }
        for (size_t i = 4; i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }

    // We want this constructor to be non-explicit because it allows for quick conversions.
    // cppcheck-suppress noExplicitConstructor
    template <typename U, size_t O>
    Vec(const Vec<U,O>& vec) {
        for (size_t i = 0; i < std::min(S,O); ++i)
            v[i] = static_cast<T>(vec[i]);
        for (size_t i = std::min(S,O); i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }

    template <typename U, size_t O>
    Vec(const Vec<U,O>& vec, const U last) {
        for (size_t i = 0; i < std::min(S-1,O); ++i)
            v[i] = static_cast<T>(vec[i]);
        for (size_t i = std::min(S-1, O); i < S-1; ++i)
            v[i] = static_cast<T>(0.0);
        v[S-1] = static_cast<T>(last);
    }
    
    template <typename U, size_t O>
    Vec(const Vec<U,O>& vec, const U oneButLast, const U last) {
        for (size_t i = 0; i < std::min(S-2,O); ++i)
            v[i] = static_cast<T>(vec[i]);
        for (size_t i = std::min(S-2, O); i < S-2; ++i)
            v[i] = static_cast<T>(0.0);
        v[S-2] = static_cast<T>(oneButLast);
        v[S-1] = static_cast<T>(last);
    }

    const Vec<T,S> operator-() const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = -v[i];
        return result;
    }

    T& operator[](const size_t index) {
        assert(index < S);
        return v[index];
    }
    
    const T& operator[](const size_t index) const {
        assert(index < S);
        return v[index];
    }
    
    T x() const {
        assert(S > 0);
        return v[0];
    }
            
    T y() const {
        assert(S > 1);
        return v[1];
    }
    
    T z() const {
        assert(S > 2);
        return v[2];
    }

    T w() const {
        assert(S > 3);
        return v[3];
    }
            
    Vec<T,2> xy() const {
        return Vec<T,2>(x(), y());
    }

    Vec<T,2> xz() const {
        return Vec<T,2>(x(), z());
    }

    Vec<T,2> yz() const {
        return Vec<T,2>(y(), z());
    }
    
    Vec<T,3> xyz() const {
        return Vec<T,3>(x(), y(), z());
    }
            
    Vec<T,4> xyzw() const {
        return Vec<T,4>(x(), y(), z(), w());
    }

    Vec<T,S-1> overLast() const {
        Vec<T,S-1> result;
        for (size_t i = 0; i < S-1; ++i)
            result[i] = v[i] / v[S-1];
        return result;
    }

    Vec<T,S> roundDownToMultiple(const Vec<T,S>& m) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::roundDownToMultiple(v[i], m[i]);
        return result;
    }
    
    Vec<T,S> roundUpToMultiple(const Vec<T,S>& m) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::roundUpToMultiple(v[i], m[i]);
        return result;
    }
    
    Vec<T,S> roundToMultiple(const Vec<T,S>& m) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::roundToMultiple(v[i], m[i]);
        return result;
    }

    // projects the given distance along this (normalized) vector onto the given vector along the orthogonal of this vector
    // unlike the dot product which projects orthogonally to the other vector
    T inverseDot(const T l, const Vec<T,S>& cd) const {
        const T cos = dot(*this, cd);
        return l / cos;
    }
    
    T length() const {
        return std::sqrt(squaredLength());
    }
    
    T squaredLength() const {
        return dot(*this, *this);
    }
    
    T distanceTo(const Vec<T,S>& other) const {
        return (*this - other).length();
    }
    
    T squaredDistanceTo(const Vec<T,S>& other) const {
        return (*this - other).squaredLength();
    }
    
    Vec<T,S>& normalize() {
        *this /= length();
        return *this;
    }
    
    const Vec<T,S> normalized() const {
        return Vec<T,S>(*this).normalize();
    }
    
    bool isNormalized() const {
        return equals(normalized());
    }
    
    Vec<T,S> normalizeRadians() const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::normalizeRadians(v[i]);
        return result;
    }
    
    Vec<T,S> normalizeDegrees() const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = Math::normalizeDegrees(v[i]);
        return result;
    }
    
    bool equals(const Vec<T,S>& other, const T epsilon = Math::Constants<T>::almostZero()) const {
        for (size_t i = 0; i < S; ++i)
            if (std::abs(v[i] - other[i]) > epsilon)
                return false;
        return true;
    }
    
    bool null() const {
        return equals(Null, Math::Constants<T>::almostZero());
    }

    void setNull() {
        for (size_t i = 0; i < S; ++i)
            v[i] = static_cast<T>(0.0);
    }
            
    void set(const T value) {
        for (size_t i = 0; i < S; ++i)
            v[i] = value;
    }
    
    bool nan() const {
        for (size_t i = 0; i < S; ++i)
            if (!Math::isnan(v[i]))
                return false;
        return true;
    }

    static bool colinear(const typename Vec<T,S>::List& points) {
        assert(points.size() == 3);
        return colinear(points[0], points[1], points[2]);
    }
    
    bool colinear(const Vec<T,S>& p2, const Vec<T,S>& p3, const T epsilon = Math::Constants<T>::colinearEpsilon()) const {
        return colinear(*this, p2, p3, epsilon);
    }

    static bool colinear(const Vec<T,S>& p1, const Vec<T,S>& p2, const Vec<T,S>& p3, const T epsilon = Math::Constants<T>::colinearEpsilon()) {
        const Vec<T,S> p1p2 = p2 - p1;
        const Vec<T,S> p2p3 = p3 - p2;
        const Vec<T,S> p1p3 = p3 - p1;
        
        return p1p3.equals(p1p2 + p2p3, epsilon);
        
        /*
        const Vec<T,S> v1 = p2 - p1;
        const Vec<T,S> v2 = p3 - p2;
        const Vec<T,S> v3 = p1 - p3;
        return v1.parallelTo(v2, epsilon) && v1.parallelTo(v3, epsilon) && v2.parallelTo(v3, epsilon);
        */
    }
    
    bool parallelTo(const Vec<T,S>& other, const T epsilon = Math::Constants<T>::colinearEpsilon()) const {
        const T d = dot(normalized(), other.normalized());
        return Math::eq(Math::abs(d), static_cast<T>(1.0), epsilon);
    }
    
    bool colinearTo(const Vec<T,3>& other, const T epsilon = Math::Constants<T>::colinearEpsilon()) const {
        return 1.0 - dot(*this, other) < epsilon;
    }
    
    int weight() const {
        return weight(v[0]) * 100 + weight(v[1]) * 10 + weight(v[2]);
    }
    
    bool hasMajorComponent(const T epsilon = Math::Constants<T>::almostZero()) const {
        if (S == 0)
            return false;
        if (S == 1)
            return true;
        
        Vec<T,S> copy(*this);
        const Math::Less<T, true> less;
        std::sort(&copy.v[0], &copy.v[S-1]+1, less);
        return less(copy[0], copy[1]);
    }
    
    size_t majorComponent(const size_t k) const {
        assert(k < S);
        
        if (k == 0) {
            size_t index = 0;
            for (size_t i = 1; i < S; ++i) {
                if (std::abs(v[i]) > std::abs(v[index]))
                    index = i;
            }
            return index;
        }
        
        // simple selection algorithm
        // we store the indices of the values in heap
        SelectionHeapCmp cmp(*this, true);
        std::vector<size_t> heap;
        for (size_t i = 0; i < S; ++i) {
            heap.push_back(i);
            std::push_heap(std::begin(heap), std::end(heap), cmp);
        }
        
        std::sort_heap(std::begin(heap), std::end(heap), cmp);
        return heap[S - k - 1];
    }

    const Vec<T,S> majorAxis(const size_t k) const {
        const size_t c = majorComponent(k);
        Vec<T,S> a = axis(c);
        if (v[c] < static_cast<T>(0.0))
            return -a;
        return a;
    }

    const Vec<T,S> absMajorAxis(const size_t k) const {
        const size_t c = majorComponent(k);
        return axis(c);
    }
    
    size_t firstComponent() const {
        return majorComponent(0);
    }
    
    size_t secondComponent() const {
        return majorComponent(1);
    }
    
    size_t thirdComponent() const {
        return majorComponent(2);
    }
    
    const Vec<T,3> firstAxis() const {
        return majorAxis(0);
    }
            
    const Vec<T,3> absFirstAxis() const {
        return absMajorAxis(0);
    }
    
    const Vec<T,3> secondAxis() const {
        return majorAxis(1);
    }
    
    const Vec<T,3> absSecondAxis() const {
        return absMajorAxis(1);
    }
    
    const Vec<T,3> thirdAxis() const {
        return majorAxis(2);
    }
    
    const Vec<T,3> absThirdAxis() const {
        return absMajorAxis(2);
    }
    
    Vec<T,S> makePerpendicular() const {
        // get an axis that this vector has the least weight towards.
        const Vec<T,S> leastAxis = majorAxis(S-1);
        
        return cross(*this, leastAxis).normalized();
    }
    
    void write(std::ostream& str, const size_t components = S) const {
        for (size_t i = 0; i < components; ++i) {
            str << v[i];
            if (i < components - 1)
                str << ' ';
        }
    }
    
    std::string asString(const size_t components = S) const {
        StringStream result;
        write(result, components);
        return result.str();
    }

    Vec<T,S>& makeAbsolute() {
        for (size_t i = 0; i < S; ++i)
            v[i] = std::abs(v[i]);
        return *this;
    }
            
    Vec<T,S> absolute() const {
        return Vec<T,S>(*this).makeAbsolute();
    }
    
    Vec<T,S> max(const Vec<T,S>& o) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; ++i)
            result[i] = std::max(v[i], o[i]);
        return result;
    }
    
    Vec<T,S>& round() {
        for (size_t i = 0; i < S; ++i)
            v[i] = Math::round(v[i]);
        return *this;
    }
    
    Vec<T,S> rounded() const {
        return Vec<T,S>(*this).round();
    }
    
    Vec<T,S>& mix(const Vec<T,S>& vec, const Vec<T,S>& factor) {
        *this = *this * (Vec<T,S>::One - factor) + vec * factor;
        return *this;
    }
    
    Vec<T,S> mixed(const Vec<T,S>& vec, const Vec<T,S>& factor) const {
        return Vec<T,S>(*this).mix(vec, factor);
    }
    
    bool isInteger(const T epsilon = Math::Constants<T>::almostZero()) const {
        for (size_t i = 0; i < S; ++i)
            if (std::abs(v[i] - Math::round(v[i])) > epsilon)
                return false;
        return true;
    }
    
    Vec<T,S>& correct(const size_t decimals = 0, const T epsilon = Math::Constants<T>::correctEpsilon()) {
        for (size_t i = 0; i < S; ++i)
            v[i] = Math::correct(v[i], decimals, epsilon);
        return *this;
    }
    
    Vec<T,S> corrected(const size_t decimals = 0, const T epsilon = Math::Constants<T>::correctEpsilon()) const {
        return Vec<T,S>(*this).correct(decimals, epsilon);
    }
    
    Vec<T,S-1> at(const size_t j, const T a) const {
        assert(v[j] != 0.0f);
        
        const T f = a / v[j];
        Vec<T,S-1> result;
        size_t k = 0;
        for (size_t i = 0; i < S; ++i) {
            if (i != j)
                result[k++] = v[i] * f;
        }
        return result;
    }
    
    struct EdgeDistance {
        const Vec<T,S> point;
        const T distance;
        
        EdgeDistance(const Vec<T,S>& i_point, const T i_distance) :
        point(i_point),
        distance(i_distance) {}
    };
    
    EdgeDistance distanceToSegment(const Vec<T,S>& start, const Vec<T,S>& end) const {
        const Vec<T,S> edgeVec = end - start;
        const Vec<T,S> edgeDir = edgeVec.normalized();
        const T scale = dot(*this - start, edgeDir);
        
        // determine the closest point on the edge
        Vec<T,S> closestPoint;
        if (scale < 0.0) {
            closestPoint = start;
        } else if ((scale * scale) > edgeVec.squaredLength()) {
            closestPoint = end;
        } else {
            closestPoint = start + edgeDir * scale;
        }

        const T distance = (*this - closestPoint).length();
        return EdgeDistance(closestPoint, distance);
    }

    static Vec<T,S> average(const typename Vec<T,S>::List& vecs) {
        assert(!vecs.empty());
        Vec<T,S> sum;
        for (size_t i = 0; i < vecs.size(); ++i)
            sum += vecs[i];
        return sum / static_cast<T>(vecs.size());
    }

    bool containedWithinSegment(const Vec<T,S>& start, const Vec<T,S>& end) const {
        assert(linearlyDependent(*this, start, end));
        const Vec<T,S> toStart = start - *this;
        const Vec<T,S> toEnd   =   end - *this;

        const T d = dot(toEnd, toStart.normalized());
        return !Math::pos(d);
    }

    template <typename I, typename G>
    static Vec<T,S> center(I cur, I end, const G& get) {
        assert(cur != end);
        Vec<T,S> result = get(*cur++);
        T count = 1.0;
        while (cur != end) {
            result += get(*cur++);
            count += 1.0;
        }
        return result / count;
    }
    
    template <typename I, typename G>
    static typename Vec<T,S>::List asList(I cur, I end, const G& get) {
        typename Vec<T,S>::List result;
        toList(cur, end, get, result);
        return result;
    }
    
    template <typename I, typename G>
    static void toList(I cur, I end, const G& get, typename Vec<T,S>::List& result) {
        addAll(cur, end, get, std::back_inserter(result));
    }
    
    template <typename I, typename G, typename O>
    static void addAll(I cur, I end, const G& get, O outp) {
        while (cur != end) {
            outp = get(*cur);
            ++cur;
        }
    }
};

template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::PosX = Vec<T,S>::unit(0);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::PosY = Vec<T,S>::unit(1);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::PosZ = Vec<T,S>::unit(2);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegX = -Vec<T,S>::unit(0);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegY = -Vec<T,S>::unit(1);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NegZ = -Vec<T,S>::unit(2);
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::Null = Vec<T,S>::fill(static_cast<T>(0.0));
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::One  = Vec<T,S>::fill(static_cast<T>(1.0));
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::NaN  = Vec<T,S>::fill(std::numeric_limits<T>::quiet_NaN());
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::Min  = Vec<T,S>::fill(std::numeric_limits<T>::min());
template <typename T, size_t S>
const Vec<T,S> Vec<T,S>::Max  = Vec<T,S>::fill(std::numeric_limits<T>::max());

template <typename T, size_t S>
const typename Vec<T,S>::List Vec<T,S>::PosAxes = Vec<T,S>::List({ PosX, PosY, PosZ });
template <typename T, size_t S>
const typename Vec<T,S>::List Vec<T,S>::NegAxes = Vec<T,S>::List({ NegX, NegY, NegZ });
template <typename T, size_t S>
const typename Vec<T,S>::List Vec<T,S>::AllAxes = Vec<T,S>::List({ PosX, NegX, PosY, NegY, PosZ, NegZ });

template <typename T, size_t S>
const typename Vec<T,S>::List Vec<T,S>::EmptyList = Vec<T,S>::List();

typedef Vec<float,1> Vec1f;
typedef Vec<double,1> Vec1d;
typedef Vec<int,1> Vec1i;
typedef Vec<long,1> Vec1l;
typedef Vec<size_t,1> Vec1s;
typedef Vec<float,2> Vec2f;
typedef Vec<double,2> Vec2d;
typedef Vec<int,2> Vec2i;
typedef Vec<long,2> Vec2l;
typedef Vec<size_t,2> Vec2s;
typedef Vec<bool,2> Vec2b;
typedef Vec<float,3> Vec3f;
typedef Vec<double,3> Vec3d;
typedef Vec<int,3> Vec3i;
typedef Vec<long,3> Vec3l;
typedef Vec<size_t,3> Vec3s;
typedef Vec<bool,3> Vec3b;
typedef Vec<float,4> Vec4f;
typedef Vec<double,4> Vec4d;
typedef Vec<int,4> Vec4i;
typedef Vec<long,4> Vec4l;
typedef Vec<size_t,4> Vec4s;
typedef Vec<bool,4> Vec4b;

/* ========== comparison operators ========== */

/**
 * Lexicographically compares the given components of the vectors using the given epsilon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @param epsilon the epsilon for component wise comparison
 * @return -1 if the left hand size is less than the right hand size, +1 if the left hand size is greater than the right hand size, and 0 if both sides are equal
 */
template <typename T, size_t S>
int compare(const Vec<T,S>& lhs, const Vec<T,S>& rhs, const T epsilon = static_cast<T>(0.0)) {
    for (size_t i = 0; i < S; ++i) {
        if (Math::lt(lhs[i], rhs[i], epsilon))
            return -1;
        if (Math::gt(lhs[i], rhs[i], epsilon))
            return 1;
    }
    return 0;
}

/**
 * Performs a pairwise lexicographical comparison of the pairs of vectors given by the two ranges. This function iterates over
 * both ranges in a parallel fashion, and compares the two current elements lexicagraphically until one range ends.
 *
 * @tparam I the range iterator type
 * @param lhsCur the beginning of the left hand range
 * @param lhsEnd the end of the left hand range
 * @param rhsCur the beginning of the right hand range
 * @param rhsEnd the end of the right hand range
 * @param epsilon the epsilon value for component wise comparison
 * @return -1 if the left hand range is less than the right hand range, +1 if the left hand range is greater than the right hand range, and 0 if both ranges are equal
 */
template <typename I>
int compare(I lhsCur, I lhsEnd, I rhsCur, I rhsEnd, const typename I::value_type::Type epsilon = static_cast<typename I::value_type::Type>(0.0)) {
    while (lhsCur != lhsEnd && rhsCur != rhsEnd) {
        const auto cmp = compare(*lhsCur, *rhsCur, epsilon);
        if (cmp < 0) {
            return -1;
        } else if (cmp > 0) {
            return 1;
        }
        ++lhsCur;
        ++rhsCur;
    }

    assert(lhsCur == lhsEnd && rhsCur == rhsEnd);
    return 0;
}

/**
 * Compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) == 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given vectors have equal values for each component, and false otherwise
 */
template <typename T, size_t S>
bool operator==(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    return compare(lhs, rhs) == 0;
}

/**
 * Compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) != 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given vectors do not have equal values for each component, and false otherwise
 */
template <typename T, size_t S>
bool operator!=(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    return compare(lhs, rhs) != 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) < 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is less than the given right hand vector
 */
template <typename T, size_t S>
bool operator<(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    return compare(lhs, rhs) < 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) <= 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is less than or equal to the given right hand vector
 */
template <typename T, size_t S>
bool operator<=(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    return compare(lhs, rhs) <= 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) > 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is greater than than the given right hand vector
 */
template <typename T, size_t S>
bool operator>(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    return compare(lhs, rhs) > 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) >= 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is greater than or equal to than the given right hand vector
 */
template <typename T, size_t S>
bool operator>=(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    return compare(lhs, rhs) >= 0;
}

/* ========== arithmetic operators ========== */

/**
 * Returns the sum of the given vectors, which is computed by adding all of their components.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the sum of the given two vectors
 */
template <typename T, size_t S>
Vec<T,S> operator+(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result(lhs);
    return result += rhs;
}

/**
 * Adds the given right hand side to the given left hand side and returns a reference to the left hand side.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return a reference to the left hand vector after adding the right hand vector to it
 */
template <typename T, size_t S>
Vec<T,S>& operator+=(Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] += rhs[i];
    }
    return lhs;
}

/**
 * Returns the difference of the given vectors, which is computed by subtracting the corresponding components
 * of the right hand vector from the components of the left hand vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the difference of the given two vectors
 */
template <typename T, size_t S>
Vec<T,S> operator-(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result(lhs);
    return result -= rhs;
}

/**
 * Subtracts the given right hand side from the given left hand side and returns a reference to the left hand side.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return a reference to the left hand vector after subtracting the right hand vector from it
 */
template <typename T, size_t S>
Vec<T,S>& operator-=(Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] -= rhs[i];
    }
    return lhs;
}

/**
 * Returns the product of the given vectors, which is computed by multiplying the corresponding components
 * of the right hand vector with the components of the left hand vector. Note that this does not compute
 * either the inner (or dot) product or the outer (or cross) product.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the product of the given two vectors
 */
template <typename T, size_t S>
Vec<T,S> operator*(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result(lhs);
    return result *= rhs;
}

/**
 * Returns the product of the given vector and scalar factor, which is computed by multiplying each component of the
 * vector with the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar
 * @return the scalar product of the given vector with the given factor
 */
template <typename T, size_t S>
Vec<T,S> operator*(const Vec<T,S>& lhs, const T rhs) {
    Vec<T,S> result(lhs);
    return result *= rhs;
}


/**
 * Returns the product of the given vector and scalar factor, which is computed by multiplying each component of the
 * vector with the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the scalar
 * @param rhs the vector
 * @return the scalar product of the given vector with the given factor
 */
template <typename T, size_t S>
Vec<T,S> operator*(const T lhs, const Vec<T,S>& rhs) {
    return Vec<T,S>(rhs) * lhs;
}

/**
 * Multiplies each component of the given left hand vector with the corresponding component of the given right hand
 * vector, stores the result in the left hand vector, and returns a reference to the left hand vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return a reference to the left hand vector after multiplying it with the right hand vector in a component wise fashion
 */
template <typename T, size_t S>
Vec<T,S>& operator*=(Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] *= rhs[i];
    }
    return lhs;
}

/**
 * Computes the scalar product of the vector with the scalar factor, stores the result in the vector, and returns a
 * reference to the vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the hand vector
 * @param rhs the scalar factor
 * @return a reference to the left hand vector after multiplying each of its components with the given scalar
 */
template <typename T, size_t S>
Vec<T,S>& operator*=(Vec<T,S>& lhs, const T rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] *= rhs;
    }
    return lhs;
}

/**
 * Returns the division of the given vectors, which is computed by dividing the corresponding components
 * of the left hand vector by the components of the right hand vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the division of the given two vectors
 */
template <typename T, size_t S>
Vec<T,S> operator/(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result(lhs);
    return result /= rhs;
}

/**
 * Returns the division of the given vector and scalar factor, which is computed by dividing each component of the
 * vector by the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar
 * @return the scalar division of the given vector with the given factor
 */
template <typename T, size_t S>
Vec<T,S> operator/(const Vec<T,S>& lhs, const T rhs) {
    Vec<T,S> result(lhs);
    return result /= rhs;
}

/**
 * Computes the component wise division of the left hand vector by the right hand vector,
 * stores the result in the vector, and returns a reference to the vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar factor
 * @return a reference to the left hand vector after dividing each of its components by corresponding component of the right hand vector
 */
template <typename T, size_t S>
Vec<T,S>& operator/=(Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] /= rhs[i];
    }
    return lhs;
}

/**
 * Computes the scalar division of the vector by the scalar factor, stores the result in the vector, and returns a
 * reference to the vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar factor
 * @return a reference to the left hand vector after dividing each of its components by the given scalar
 */
template <typename T, size_t S>
Vec<T,S>& operator/=(Vec<T,S>& lhs, const T rhs) {
    for (size_t i = 0; i < S; ++i) {
        lhs[i] /= rhs;
    }
    return lhs;
}

/**
 * Adds the given vector to each of the vectors in the given range.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the range of vectors
 * @param rhs the right hand vector
 * @return a range containing the sum of each of the vectors in the given range with the right hand vector
 */
template <typename T, size_t S>
typename Vec<T,S>::List operator+(const typename Vec<T,S>::List& lhs, const Vec<T,S>& rhs) {
    typename Vec<T,S>::List result;
    result.reserve(lhs.size());
    for (const auto& vec : lhs) {
        result.push_back(vec + rhs);
    }
    return result;
}

/**
 * Adds the given vector to each of the vectors in the given range.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the range of vectors
 * @return a range containing the sum of each of the vectors in the given range with the left hand vector
 */
template <typename T, size_t S>
typename Vec<T,S>::List operator+(const Vec<T,S>& lhs, const typename Vec<T,S>::List& rhs) {
    return rhs + lhs;
}

/**
 * Multiplies each vector in the given range by the given scalar.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the range of vectors
 * @param rhs the scalar factor
 * @return a range containing the scalar product of each vector in the given range with the given scalar
 */
template <typename T, size_t S>
typename Vec<T,S>::List operator*(const typename Vec<T,S>::List& lhs, const T rhs) {
    typename Vec<T,S>::List result;
    result.reserve(lhs.size());
    for (const auto& vec : lhs) {
        result.push_back(vec + rhs);
    }
    return result;
}

/**
 * Multiplies each vector in the given range by the given scalar.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the scalar factor
 * @param rhs the range of vectors
 * @return a range containing the scalar product of each vector in the given range with the given scalar
 */
template <typename T, size_t S>
typename Vec<T,S>::List operator*(const T lhs, const typename Vec<T,S>::List& rhs) {
    return rhs * lhs;
}

/* ========== stream operators ========== */

template <typename T, size_t S>
std::ostream& operator<< (std::ostream& stream, const Vec<T,S>& vec) {
    stream << "(";
    if (S > 0) {
        stream << vec[0];
        for (size_t i = 1; i < S; ++i) {
            stream << ", " << vec[i];
        }
    }
    stream << ")";
    return stream;
}

/* ========== arithmetic functions ========== */

/**
 * Returns the dot product (also called inner product) of the two given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the dot product of the given vectors
 */
template <typename T, size_t S>
T dot(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    T result = static_cast<T>(0.0);
    for (size_t i = 0; i < S; ++i) {
        result += (lhs[i] * rhs[i]);
    }
    return result;
}

/**
 * Returns the cross product (also called outer product) of the two given 3d vectors.
 *
 * @tparam T the component type
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the cross product of the given vectors
 */
template <typename T>
const Vec<T,3> cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs) {
    return Vec<T,3>(lhs[1] * rhs[2] - lhs[2] * rhs[1],
                    lhs[2] * rhs[0] - lhs[0] * rhs[2],
                    lhs[0] * rhs[1] - lhs[1] * rhs[0]);
}

/*
 * The normal will be pointing towards the reader when the points are oriented like this:
 *
 * 1
 * |
 * v2
 * |
 * |
 * 0------v1----2
 */
template <typename T>
bool planeNormal(Vec<T,3>& normal, const Vec<T,3>& point0, const Vec<T,3>& point1, const Vec<T,3>& point2, const T epsilon = Math::Constants<T>::angleEpsilon()) {
    const Vec<T,3> v1 = point2 - point0;
    const Vec<T,3> v2 = point1 - point0;
    normal = cross(v1, v2);
    
    // Fail if v1 and v2 are parallel, opposite, or either is zero-length.
    // Rearranging "A cross B = ||A|| * ||B|| * sin(theta) * n" (n is a unit vector perpendicular to A and B) gives sin_theta below
    const T sin_theta = Math::abs(normal.length() / (v1.length() * v2.length()));
    if (Math::isnan(sin_theta) ||
        Math::isinf(sin_theta) ||
        sin_theta < epsilon)
        return false;
    
    normal.normalize();
    return true;
}

/**
 Computes the CCW angle between axis and vector in relation to the given up vector.
 All vectors are expected to be normalized.
 */
template <typename T>
T angleBetween(const Vec<T,3>& vec, const Vec<T,3>& axis, const Vec<T,3>& up) {
    const T cos = dot(vec, axis);
    if (Math::one(+cos))
        return static_cast<T>(0.0);
    if (Math::one(-cos))
        return Math::Constants<T>::pi();
    const Vec<T,3> perp = cross(axis, vec);
    if (!Math::neg(dot(perp, up)))
        return std::acos(cos);
    return Math::Constants<T>::twoPi() - std::acos(cos);
}

template <typename T>
bool commonPlane(const Vec<T,3>& p1, const Vec<T,3>& p2, const Vec<T,3>& p3, const Vec<T,3>& p4, const T epsilon = Math::Constants<T>::almostZero()) {
    assert(!p1.colinear(p2, p3, epsilon));
    const Vec<T,3> normal = cross(p3 - p1, p2 - p1).normalized();
    const T offset = dot(p1, normal);
    const T dist = dot(p4, normal) - offset;
    return Math::abs(dist) < epsilon;
}

template <typename T, size_t S>
Vec<T,S> min(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i)
        result[i] = Math::min(lhs[i], rhs[i]);
    return result;
}

template <typename T, size_t S>
Vec<T,S> max(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i)
        result[i] = Math::max(lhs[i], rhs[i]);
    return result;
}

template <typename T, size_t S>
Vec<T,S> absMin(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i)
        result[i] = Math::absMin(lhs[i], rhs[i]);
    return result;
}

template <typename T, size_t S>
Vec<T,S> absMax(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i)
        result[i] = Math::absMax(lhs[i], rhs[i]);
    return result;
}

template <typename T>
Vec<T,3> crossed(const Vec<T,3>& point0, const Vec<T,3>& point1, const Vec<T,3>& point2) {
    const Vec<T,3> v1 = point2 - point0;
    const Vec<T,3> v2 = point1 - point0;
    return cross(v1, v2);
}

template <typename T, size_t S>
bool linearlyDependent1(const Vec<T,S>& a, const Vec<T,S>& b, const Vec<T,S>& c) {
    // see http://math.stackexchange.com/a/1778739
    // advantage over linearlyDependent2 is that no square root is required here
    
    T j = 0.0;
    T k = 0.0;
    T l = 0.0;
    for (size_t i = 0; i < S; ++i) {
        const T ac = a[i] - c[i];
        const T ba = b[i] - a[i];
        j += ac * ba;
        k += ac * ac;
        l += ba * ba;
    }
    
    return Math::zero(j * j - k * l, Math::Constants<T>::colinearEpsilon());
}


template <typename T, size_t S>
bool linearlyDependent2(const Vec<T,S>& a, const Vec<T,S>& b, const Vec<T,S>& c) {
    // A,B,C are colinear if and only if the largest of the lenghts of AB,AC,BC is equal to the sum of the other two.
    
    const T ac = (c - a).length();
    const T bc = (c - b).length();
    const T ab = (b - a).length();
    
    if (ac > bc) {
        if (ac > ab) // ac > ab, bc
            return ac == bc + ab;
        else if (ab > ac) // ab > ac > bc
            return ab == ac + bc;
        else // ac == ab > bc
            return ac == ab + bc; // bc could be 0
    } else if (bc > ac) {
        if (bc > ab) // bc > ac, ab
            return bc == ac + ab;
        else if (ab > bc) // ab > bc > ac
            return ab == bc + ac;
        else // ab == bc > ac
            return ab == bc + ac; // ac could be 0
    } else { // ac == bc
        if (ab > ac) // ab > ac == bc
            return ab == ac + bc;
        else // bc == ac >= ab // ab could be 0
            return bc == ac + ab;
    }
}

template <typename T, size_t S>
bool linearlyDependent(const Vec<T,S>& a, const Vec<T,S>& b, const Vec<T,S>& c) {
    return linearlyDependent1(a, b, c);
}

#endif
