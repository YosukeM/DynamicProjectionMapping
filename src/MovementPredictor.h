//
//  MovementPredictor.h
//  emptyExample
//
//  Created by 森本 陽介 on 2012/12/15.
//
//

#ifndef __emptyExample__MovementPredictor__
#define __emptyExample__MovementPredictor__

#include <ofMath.h>
#include <ofQuaternion.h>
#include <boost/circular_buffer.hpp>

class MovementPredictor {
public:
	typedef struct {
		ofVec3f position;
		ofQuaternion orientation;
	} Transform;
	
protected:
	boost::circular_buffer<Transform> history;
	
public:
	MovementPredictor();
	
	bool canPredict() const;
	void clearHistory();
	void pushHistory(const Transform& transform);
	Transform predictNext() const;
};

#endif /* defined(__emptyExample__MovementPredictor__) */
