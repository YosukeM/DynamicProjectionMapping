//
//  Content.cpp
//  emptyExample
//
//  Created by 森本 陽介 on 2012/11/30.
//
//

#include "Content.h"
#include "testApp.h"

void Content::setTestApp(class testApp* app) {
	parent = app;
}

ofVec3f Content::getModelPosition() const {
	return parent->modelBase.getPosition();
}

ofQuaternion Content::getModelOrientation() const {
	return parent->modelBase.getOrientationQuat();
}

void Content::generateOutput(ofImage& image0, ofImage& image1, ofImage& image2, ofImage& image3, ofImage& image4, ofImage& image5, const ofColor& color) {
	int part_w = image0.width;
	
	if (part_w != image0.height
		|| part_w != image1.width || part_w != image1.height
		 || part_w != image2.width || part_w != image2.height
		 || part_w != image3.width || part_w != image3.height
		 || part_w != image4.width || part_w != image4.height
		 || part_w != image5.width || part_w != image5.height
	) {
		std::cout << "generateOutput: parts image widthes and heights are not all the same." << std::endl;
		return;
	}
	
	int w = part_w * 4;
	output.allocate(w, w, OF_IMAGE_COLOR);
	
	for (int i = 0; i < 3*w*w; i+=3) {
		output.getPixels()[i] = color[0];
		output.getPixels()[i+1] = color[1];
		output.getPixels()[i+2] = color[2];
		//output.getPixels()[i+3] = color[3];
	}
	
	image0.getPixelsRef().pasteInto(output, 1.5 * part_w, 0);
	image1.getPixelsRef().pasteInto(output, 1.5 * part_w, part_w);
	image2.getPixelsRef().pasteInto(output, 1.5 * part_w, part_w*2);
	image3.getPixelsRef().pasteInto(output, 1.5 * part_w, part_w*3);
	image4.getPixelsRef().pasteInto(output, 0.5 * part_w, part_w*3);
	image5.getPixelsRef().pasteInto(output, 2.5 * part_w, part_w*3);
	
	output.reloadTexture();
}