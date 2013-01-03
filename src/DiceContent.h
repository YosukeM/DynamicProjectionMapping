//
//  DiceContent.h
//  emptyExample
//
//  Created by 森本 陽介 on 2012/12/04.
//
//

#ifndef __emptyExample__DiceContent__
#define __emptyExample__DiceContent__

#include "Content.h"

class DiceContent : public Content {
	ofImage srcImage[6];
public:
	DiceContent();
	
	void update();
};

#endif /* defined(__emptyExample__DiceContent__) */
