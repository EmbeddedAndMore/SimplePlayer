#include "DecoderTest.h"
#include "../SimplePlayer.Core/video_decoder.h"


DecoderTest::DecoderTest()
{
}


DecoderTest::~DecoderTest()
{
}



int main(int argv, char** argc)
{
	video_decoder decoder;
	decoder.init(300, 500, "D:\\H.264 sample video.mp4");
	decoder.start();

	return 0;
}