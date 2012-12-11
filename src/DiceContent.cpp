//
//  DiceContent.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/12/04.
//
//

#include "DiceContent.h"

DiceContent::DiceContent() {
	output.allocate(1024, 1024, OF_IMAGE_COLOR);
	
	srcImage[0].loadImage("0.png");
	srcImage[1].loadImage("1.png");
	srcImage[2].loadImage("2.png");
	srcImage[3].loadImage("3.png");
	srcImage[4].loadImage("4.png");
	srcImage[5].loadImage("5.png");
}

void DiceContent::update() {
	generateOutput(srcImage[0], srcImage[1], srcImage[2], srcImage[3], srcImage[4], srcImage[5], ofColor::black);
}