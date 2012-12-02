//
//  LevelMeterContent.h
//  emptyExample
//
//  Created by 森本 陽介 on 2012/12/02.
//
//

#ifndef __emptyExample__LevelMeterContent__
#define __emptyExample__LevelMeterContent__

#include "Content.h"
#include <ofVideoPlayer.h>

class LevelMeterContent : public Content {
	ofVideoPlayer video;
public:
	LevelMeterContent();
	virtual void update();
	virtual void draw();
};

#endif /* defined(__emptyExample__LevelMeterContent__) */
