// AutoBot - macro recorder / auto-play bot for Geometry Dash (Geode)

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJEffectManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

using namespace geode::prelude;

enum class BotMode { Off = 0, Record = 1, Replay = 2 };

struct BotAction {
	int frame;
	int button;
	bool down;
	bool player1;
};

namespace Bot {
	inline BotMode mode = BotMode::Off;
	inline std::vector<BotAction> actions;
	inline size_t cursor = 0;

	inline const char* name(BotMode m) {
		switch (m) {
			case BotMode::Record: return "Record";
			case BotMode::Replay: return "Replay";
			default: return "Off";
		}
	}

	inline int frameOf(GJBaseGameLayer* layer) {
		return layer->m_gameState.m_currentProgress;
	}

	inline bool isPlayLayer(GJBaseGameLayer* layer) {
		return PlayLayer::get() != nullptr && static_cast<GJBaseGameLayer*>(PlayLayer::get()) == layer;
	}

	inline std::filesystem::path pathFor(GJGameLevel* level) {
		auto dir = Mod::get()->getSaveDir() / "macros";
		std::error_code ec;
		std::filesystem::create_directories(dir, ec);
		std::string n = level->m_levelName;
		for (auto& c : n) {
			if (!std::isalnum(static_cast<unsigned char>(c))) c = '_';
		}
		return dir / fmt::format("{}_{}.txt", level->m_levelID.value(), n);
	}

	inline bool save(GJGameLevel* level) {
		if (actions.empty()) return false;
		std::ofstream out(pathFor(level));
		if (!out) return false;
		out << "# frame button down player1\n";
		for (auto const& a : actions) {
			out << a.frame << ' ' << a.button << ' ' << (a.down ? 1 : 0) << ' ' << (a.player1 ? 1 : 0) << '\n';
		}
		return true;
	}

	inline bool load(GJGameLevel* level) {
		std::ifstream in(pathFor(level));
		if (!in) return false;
		std::vector<BotAction> loaded;
		std::string line;
		while (std::getline(in, line)) {
			if (line.empty() || line[0] == '#') continue;
			std::istringstream ss(line);
			int f, b, d, p;
			if (ss >> f >> b >> d >> p) loaded.push_back({f, b, d != 0, p != 0});
		}
		if (loaded.empty()) return false;
		actions = std::move(loaded);
		cursor = 0;
		return true;
	}
}

// ---------------------------------------------------------------- input / replay
class $modify(BotBaseLayer, GJBaseGameLayer) {
	void handleButton(bool down, int button, bool isPlayer1) {
		if (Bot::isPlayLayer(this)) {
			if (Bot::mode == BotMode::Record) {
				Bot::actions.push_back({Bot::frameOf(this), button, down, isPlayer1});
			} else if (Bot::mode == BotMode::Replay) {
				return; // ignore real input, the macro drives the game
			}
		}
		GJBaseGameLayer::handleButton(down, button, isPlayer1);
	}

	void processCommands(float dt, bool isHalfTick, bool isLastTick) {
		if (Bot::mode == BotMode::Replay && Bot::isPlayLayer(this)) {
			int frame = Bot::frameOf(this);
			while (Bot::cursor < Bot::actions.size() && Bot::actions[Bot::cursor].frame <= frame) {
				auto const& a = Bot::actions[Bot::cursor++];
				// qualified call = original function, bypasses our hook above
				GJBaseGameLayer::handleButton(a.down, a.button, a.player1);
			}
		}
		GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
	}
};

// ---------------------------------------------------------------- level lifecycle
class $modify(BotPlayLayer, PlayLayer) {
	void resetLevel() {
		PlayLayer::resetLevel();
		Bot::cursor = 0;
		if (Bot::mode == BotMode::Record) {
			Bot::actions.clear(); // always record from the start of the level
		} else if (Bot::mode == BotMode::Replay && Bot::actions.empty()) {
			Bot::load(m_level);
		}
	}

	void levelComplete() {
		if (Bot::mode == BotMode::Record) {
			bool ok = Bot::save(m_level);
			Notification::create(ok ? "Macro saved" : "Macro save failed",
				ok ? NotificationIcon::Success : NotificationIcon::Error)->show();
		}
		PlayLayer::levelComplete();
	}

	void destroyPlayer(PlayerObject* player, GameObject* obj) {
		if (Bot::mode == BotMode::Replay
			&& Mod::get()->getSettingValue<bool>("safety-net")
			&& obj != m_anticheatSpike) {
			return; // safety net: ignore the death
		}
		PlayLayer::destroyPlayer(player, obj);
	}
};

// ---------------------------------------------------------------- never submit progress while botting
class $modify(BotLevel, GJGameLevel) {
	void savePercentage(int percent, bool practice, int clicks, int attempts, bool valid) {
		if (Bot::mode != BotMode::Off) return;
		GJGameLevel::savePercentage(percent, practice, clicks, attempts, valid);
	}
};

// ---------------------------------------------------------------- pause menu UI
class $modify(BotPauseLayer, PauseLayer) {
	void customSetup() {
		PauseLayer::customSetup();

		auto win = CCDirector::get()->getWinSize();

		auto label = CCLabelBMFont::create(fmt::format("Bot: {}", Bot::name(Bot::mode)).c_str(), "bigFont.fnt");
		label->setScale(0.5f);
		label->setPosition({win.width - 70.f, win.height / 2 + 70.f});
		label->setID("bot-status");
		this->addChild(label);

		auto menu = CCMenu::create();
		menu->setPosition({win.width - 70.f, win.height / 2});
		menu->setID("bot-menu");

		auto addBtn = [&](char const* text, BotMode m, float y) {
			auto spr = ButtonSprite::create(text);
			spr->setScale(0.7f);
			auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(BotPauseLayer::onBotMode));
			btn->setTag(static_cast<int>(m));
			btn->setPosition({0.f, y});
			menu->addChild(btn);
		};
		addBtn("Record", BotMode::Record, 35.f);
		addBtn("Replay", BotMode::Replay, 0.f);
		addBtn("Bot Off", BotMode::Off, -35.f);

		this->addChild(menu);
	}

	void onBotMode(CCObject* sender) {
		auto mode = static_cast<BotMode>(static_cast<CCNode*>(sender)->getTag());
		auto pl = PlayLayer::get();

		if (mode == BotMode::Replay) {
			if (!pl || !Bot::load(pl->m_level)) {
				Notification::create("No macro found for this level", NotificationIcon::Error)->show();
				return;
			}
		}
		if (mode == BotMode::Record) Bot::actions.clear();

		Bot::mode = mode;
		Bot::cursor = 0;

		if (auto l = static_cast<CCLabelBMFont*>(this->getChildByID("bot-status"))) {
			l->setString(fmt::format("Bot: {}", Bot::name(mode)).c_str());
		}
		Notification::create(fmt::format("Bot: {} - restart the level to start", Bot::name(mode)),
			NotificationIcon::Success)->show();
	}
};