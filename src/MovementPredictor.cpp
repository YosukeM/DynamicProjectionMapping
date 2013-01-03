//
//  MovementPredictor.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/12/15.
//
//

#include "MovementPredictor.h"


MovementPredictor::MovementPredictor()
: history(2)
{
	
}

bool MovementPredictor::canPredict() const {
	return history.size() == 2;
}

void MovementPredictor::clearHistory() {
	history.clear();
}

void MovementPredictor::pushHistory(const Transform& transform) {
	history.push_back(transform);
}

MovementPredictor::Transform MovementPredictor::predictNext() const {
	const auto& prev = history[0];
	const auto& now = history[1];
	
	Transform next;
	next.position = (now.position - prev.position) + now.position;
	next.orientation = (now.orientation * prev.orientation.inverse()) * now.orientation;
	return next;
}