//
//  Projector.h
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/11.
//
//

#ifndef __emptyExample__Projector__
#define __emptyExample__Projector__

#include "ofMain.h"

class ProjectorNode : public ofCamera {
public:
	ProjectorNode();
private:
	virtual void customDraw();
};

#endif /* defined(__emptyExample__Projector__) */
