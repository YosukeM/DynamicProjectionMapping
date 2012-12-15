//
//  ofVectorExtension.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/20.
//
//

#include "ofVectorExtension.h"
#include <boost/unordered_set.hpp>

size_t hash_value( const ofVec3f& d ) {
	size_t h = 0;
	boost::hash_combine(h, d.x);
	boost::hash_combine(h, d.y);
	boost::hash_combine(h, d.z);
	return h;
}


/*bool operator < (const ofVec3f& v1, const ofVec3f& v2) {
	return v1.lengthSquared() < v2.lengthSquared();
}

bool operator <= (const ofVec3f& v1, const ofVec3f& v2) {
	return (v1.lengthSquared() < v2.lengthSquared()) || (v1 == v2);
}

bool operator > (const ofVec3f& v1, const ofVec3f& v2) {
	return v1.lengthSquared() > v2.lengthSquared();
}

bool operator >= (const ofVec3f& v1, const ofVec3f& v2) {
	return (v1.lengthSquared() > v2.lengthSquared()) || (v1 == v2);
}
*/