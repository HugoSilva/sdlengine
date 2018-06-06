#include <iostream>
#include <gtest/gtest.h>
#include <Audio/SoundManager.h>

using namespace std;
using namespace audio;

TEST(SoundManager, AddSound)
{
	SoundManager::add(new Sound("bgm", "Resources/background.ogg"));
	EXPECT_EQ(1, 1);
}

TEST(SoundManager, AddMusic)
{
	SoundManager::add(new Music("bgm", "Resources/background.ogg"));
	EXPECT_EQ(1, 1);
}

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}