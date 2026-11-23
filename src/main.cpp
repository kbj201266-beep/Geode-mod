// AutoBot - macro bot + mod menu (Geode)

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/GJEffectManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/GJGroundLayer.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/UILayer.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

using namespace geode::prelude;

enum class BotMode { Off = 0, Record = 1, Replay = 2 };

struct BotAction {
	int frame;
	int button;
	bool down;
	bool player1;
};

// ================================================================ cheats / menu state
namespace Cheats {
	inline constexpr int TPS_MIN = 170;
	inline constexpr int TPS_MAX = 1000000;

	inline bool icons = false;
	inline bool speed = false;
	inline bool practiceSync = false;
	inline bool layout = false;
	inline bool hitboarder = false;
	inline float speedVal = 1.f;
	inline int tps = 240; // physics ticks per second chosen in the menu (240 = normal)

	// ---- frame perfect counter settings
	inline constexpr int PALETTE_N = 12;
	inline const ccColor3B PALETTE[PALETTE_N] = {
		{255, 90, 60},   // 0 red-orange
		{255, 165, 60},  // 1 orange
		{255, 230, 80},  // 2 yellow
		{255, 255, 255}, // 3 white
		{120, 255, 120}, // 4 green
		{100, 170, 255}, // 5 light blue
		{130, 130, 255}, // 6 purple-blue
		{255, 120, 200}, // 7 pink
		{80, 255, 255},  // 8 cyan
		{170, 170, 170}, // 9 gray
		{190, 255, 60},  // 10 lime
		{255, 0, 0}      // 11 red
	};
	inline int fpMax = 12;                              // widest window, in frames at 240 TPS
	inline int fpColor[7] = {0, 1, 2, 3, 4, 5, 6};      // palette index per row (0 = bottom row)

	inline bool speedActive() { return speed && std::fabs(speedVal - 1.f) > 0.0001f; }
	inline bool tpsActive() { return tps != 240; }

	inline void applySpeed() {
		float s = (speed && PlayLayer::get()) ? speedVal : 1.f;
		CCDirector::get()->getScheduler()->setTimeScale(s);
		auto eng = FMODAudioEngine::sharedEngine();
		if (eng && eng->m_system) {
			FMOD::ChannelGroup* master = nullptr;
			eng->m_system->getMasterChannelGroup(&master);
			if (master) master->setPitch(s);
		}
	}

	inline void resetSpeed() {
		speedVal = 1.f;
		applySpeed();
	}

	// layout mode: hide decoration, blue background/ground, default colour for the rest
	inline void appl