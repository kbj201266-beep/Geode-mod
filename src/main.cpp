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
#include <Geode/binding/UILayer.hpp>

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

	// ---- hitboxes + player path
	inline std::vector<CCPoint> trail;

	inline void drawBox(CCDrawNode* node, CCRect const& r, ccColor4F fill, ccColor4F border) {
		CCPoint v[4] = {
			CCPoint(r.origin.x, r.origin.y),
			CCPoint(r.origin.x + r.size.width, r.origin.y),
			CCPoint(r.origin.x + r.size.width, r.origin.y + r.size.height),
			CCPoint(r.origin.x, r.origin.y + r.size.height)
		};
		node->drawPolygon(v, 4, fill, 0.5f, border);
	}

	inline void updateHitboxes(GJBaseGameLayer* layer) {
		if (!layer->m_objectLayer) return;
		auto node = typeinfo_cast<CCDrawNode*>(layer->m_objectLayer->getChildByID("bot-hitbox"));
		if (!node) return;
		node->clear();
		if (!Mod::get()->getSettingValue<bool>("show-hitboxes")) return;

		auto p1 = layer->m_player1;
		if (!p1) return;
		float px = p1->getPositionX();

		ccColor4F redFill{1.f, 0.f, 0.f, 0.45f};
		ccColor4F redBorder{1.f, 0.1f, 0.1f, 1.f};
		ccColor4F blueFill{0.f, 0.f, 1.f, 0.4f};
		ccColor4F blueBorder{0.2f, 0.2f, 1.f, 1.f};
		ccColor4F greenFill{0.f, 1.f, 0.f, 0.4f};
		ccColor4F greenBorder{0.2f, 1.f, 0.2f, 1.f};

		if (layer->m_objects) {
			CCObject* o = nullptr;
			CCARRAY_FOREACH(layer->m_objects, o) {
				auto obj = static_cast<GameObject*>(o);
				float dx = obj->getPositionX() - px;
				if (dx < -150.f || dx > 450.f) continue;

				auto type = obj->m_objectType;
				if (type == GameObjectType::Decoration) continue;

				if (type == GameObjectType::Hazard) {
					drawBox(node, obj->getObjectRect(), redFill, redBorder);
				} else if (type == GameObjectType::Solid) {
					drawBox(node, obj->getObjectRect(), blueFill, blueBorder);
				} else {
					drawBox(node, obj->getObjectRect(), greenFill, greenBorder);
				}
			}
		}

		// player box (yellow border)
		drawBox(node, p1->getObjectRect(), ccColor4F{1.f, 1.f, 0.f, 0.2f}, ccColor4F{1.f, 1.f, 0.f, 1.f});

		// player path (recent positions)
		trail.push_back(p1->getPosition());
		if (trail.size() > 80) trail.erase(trail.begin());
		for (size_t i = 1; i < trail.size(); ++i) {
			node->drawSegment(trail[i - 1], trail[i], 0.8f, ccColor4F{1.f, 1.f, 0.f, 1.f});
		}
	}

	// ---- frame perfect counter (gap in frames between consecutive inputs)
	// buckets: 0="1", 1="2", 2="3", 3="4", 4="5-6", 5="7-8", 6="9-12"
	inline int fpCount[7] = {0, 0, 0, 0, 0, 0, 0};
	inline int lastInputFrame = -1;

	inline void updateFp() {
		auto pl = PlayLayer::get();
		if (!pl || !pl->m_uiLayer) return;
		for (int b = 0; b < 7; ++b) {
			auto v = typeinfo_cast<CCLabelBMFont*>(
				pl->m_uiLayer->getChildByID(fmt::format("bot-fp-val-{}", b)));
			if (v) v->setString(std::to_string(fpCount[b]).c_str());
		}
	}

	inline void resetFp() {
		for (int b = 0; b < 7; ++b) fpCount[b] = 0;
		lastInputFrame = -1;
		trail.clear();
		updateFp();
	}

	inline void countInput(int frame) {
		if (lastInputFrame >= 0 && frame > lastInputFrame) {
			int gap = frame - lastInputFrame;
			int b = -1;
			if (gap <= 4) b = gap - 1;
			else if (gap <= 6) b = 4;
			else if (gap <= 8) b = 5;
			else if (gap <= 12) b = 6;
			if (b >= 0) {
				fpCount[b]++;
				updateFp();
			}
		}
		if (frame > lastInputFrame) lastInputFrame = frame;
	}

	// ---- on-screen frame number
	inline void updateCounter(GJBaseGameLayer* layer) {
		auto pl = PlayLayer::get();
		if (!pl || !pl->m_uiLayer) return;
		auto label = typeinfo_cast<CCLabelBMFont*>(pl->m_uiLayer->getChildByID("bot-counter"));
		if (!label) return;

		int frame = frameOf(layer);
		std::string text;
		switch (mode) {
			case BotMode::Record:
				text = fmt::format("REC  Frame: {} | Clicks: {}", frame, actions.size());
				break;
			case BotMode::Replay:
				text = fmt::format("PLAY  Frame: {} | {}/{}", frame, cursor, actions.size());
				break;
			default:
				text = fmt::format("Frame: {}", frame);
				break;
		}
		label->setString(text.c_str());
	}

	// ---- macros are stored in the mod's saved data (no file paths needed)
	inline std::string keyFor(GJGameLevel* level) {
		std::string n = std::string(level->m_levelName);
		for (auto& c : n) {
			if (!std::isalnum(static_cast<unsigned char>(c))) c = '_';
		}
		return fmt::format("macro_{}_{}", level->m_levelID.value(), n);
	}

	inline bool save(GJGameLevel* level) {
		if (actions.empty()) return false;
		std::ostringstream out;
		for (auto const& a : actions) {
			out << a.frame << ' ' << a.button << ' ' << (a.down ? 1 : 0) << ' ' << (a.player1 ? 1 : 0) << '\n';
		}
		Mod::get()->setSavedValue<std::string>(keyFor(level), out.str());
		return true;
	}

	inline bool load(GJGameLevel* level) {
		std::string text = Mod::get()->getSavedValue<std::string>(keyFor(level), "");
		if (text.empty()) return false;
		std::istringstream in(text);
		std::vector<BotAction> loaded;
		std::string line;
		while (std::getline(in, line)) {
			if (line.empty()) continue;
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
			int frame = Bot::frameOf(this);
			if (Bot::mode == BotMode::Record) {
				Bot::actions.push_back({frame, button, down, isPlayer1});
				Bot::countInput(frame);
			} else if (Bot::mode == BotMode::Replay) {
				return; // ignore real input, the macro drives the game
			} else {
				Bot::countInput(frame);
			}
		}
		GJBaseGameLayer::handleButton(down, button, isPlayer1);
	}

	void processCommands(float dt, bool isHalfTick, bool isLastTick) {
		if (Bot::mode == BotMode::Replay && Bot::isPlayLayer(this)) {
			int frame = Bot::frameOf(this);
			while (Bot::cursor < Bot::actions.size() && Bot::actions[Bot::cursor].frame <= frame) {
				auto const& a = Bot::actions[Bot::cursor++];
				Bot::countInput(frame);
				// qualified call = original function, bypasses our hook above
				GJBaseGameLayer::handleButton(a.down, a.button, a.player1);
			}
		}
		GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);

		if (Bot::isPlayLayer(this)) {
			// refresh the frame number every 4 physics steps
			if (Bot::frameOf(this) % 4 == 0) Bot::updateCounter(this);
			// redraw hitboxes once per rendered frame
			if (isLastTick) Bot::updateHitboxes(this);
		}
	}
};

// ---------------------------------------------------------------- level lifecycle
class $modify(BotPlayLayer, PlayLayer) {
	void setupHasCompleted() {
		PlayLayer::setupHasCompleted();

		// hitbox drawing layer (inside the object layer so it scrolls with the level)
		if (m_objectLayer) {
			auto draw = CCDrawNode::create();
			draw->setID("bot-hitbox");
			m_objectLayer->addChild(draw, 1000);
		}

		if (!m_uiLayer) return;
		auto win = CCDirector::get()->getWinSize();

		// frame number (bottom-left)
		auto label = CCLabelBMFont::create("Frame: 0", "bigFont.fnt");
		label->setScale(0.4f);
		label->setAnchorPoint({0.f, 0.f});
		label->setPosition({8.f, 8.f});
		label->setOpacity(180);
		label->setID("bot-counter");
		m_uiLayer->addChild(label, 1000);

		// frame perfect counter (top-left), top row = "9-12", bottom row = "1"
		static const char* names[7] = {"1:", "2:", "3:", "4:", "5-6:", "7-8:", "9-12:"};
		static const ccColor3B colors[7] = {
			{255, 90, 60},   // 1
			{255, 165, 60},  // 2
			{255, 230, 80},  // 3
			{255, 255, 255}, // 4
			{120, 255, 120}, // 5-6
			{100, 170, 255}, // 7-8
			{130, 130, 255}  // 9-12
		};
		for (int b = 0; b < 7; ++b) {
			int row = 6 - b;
			float y = win.height - 20.f - row * 14.f;

			auto n = CCLabelBMFont::create(names[b], "bigFont.fnt");
			n->setScale(0.35f);
			n->setAnchorPoint({0.f, 0.5f});
			n->setPosition({10.f, y});
			n->setColor(colors[b]);
			n->setID(fmt::format("bot-fp-name-{}", b));
			m_uiLayer->addChild(n, 1000);

			auto v = CCLabelBMFont::create("0", "bigFont.fnt");
			v->setScale(0.35f);
			v->setAnchorPoint({1.f, 0.5f});
			v->setPosition({75.f, y});
			v->setColor(colors[b]);
			v->setID(fmt::format("bot-fp-val-{}", b));
			m_uiLayer->addChild(v, 1000);
		}
	}

	void resetLevel() {
		PlayLayer::resetLevel();
		Bot::cursor = 0;
		if (Bot::mode == BotMode::Record) {
			Bot::actions.clear(); // always record from the start of the level
		} else if (Bot::mode == BotMode::Replay && Bot::actions.empty()) {
			Bot::load(m_level);
		}
		Bot::resetFp();
		Bot::updateCounter(this);
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