#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace AutoBotUI {
    constexpr int Z_ORDER = 1000;
    constexpr float PANEL_WIDTH = 700.f;
    constexpr float PANEL_HEIGHT = 420.f;

    enum class Category {
        Level,
        Counter,
        Speed,
        Cosmetic,
        Config
    };

    struct State {
        bool noclip = false;
        bool showHitboxes = false;
        bool showHitboxesOnDeath = false;
        bool hitboxTrail = true;
        bool showTrajectory = false;
        bool pathfinder = false;
        bool framePerfect = false;
        bool accuratePercentage = true;
        bool macroRecord = false;
        bool macroReplay = false;
        bool speedHack = false;
        bool practiceMusicSync = false;
        bool iconHack = false;
        bool layoutMod = false;
        bool safetyNet = true;
    };

    inline State state;

    static CCLabelBMFont* label(
        char const* text,
        float scale = 0.46f
    ) {
        auto result = CCLabelBMFont::create(
            text,
            "bigFont.fnt"
        );

        result->setScale(scale);
        result->setAnchorPoint({0.f, 0.5f});

        return result;
    }

    static ButtonSprite* button(
        char const* text,
        float scale = 0.42f
    ) {
        return ButtonSprite::create(
            text,
            "goldFont.fnt",
            "GJ_button_01.png",
            scale
        );
    }

    class Panel final : public CCLayerColor {
    private:
        Category m_category = Category::Level;
        CCNode* m_content = nullptr;

        void addCategory(
            CCMenu* menu,
            char const* text,
            Category category,
            float y
        ) {
            auto item = CCMenuItemSpriteExtra::create(
                button(text, 0.38f),
                this,
                menu_selector(Panel::onCategory)
            );

            item->setTag(static_cast<int>(category));
            item->setPosition({105.f, y});

            menu->addChild(item);
        }

        void addToggle(
            CCMenu* menu,
            char const* text,
            bool value,
            int tag,
            float y
        ) {
            auto title = label(text, 0.42f);
            title->setPosition({250.f, y});
            m_content->addChild(title);

            auto item = CCMenuItemSpriteExtra::create(
                button(value ? "ON" : "OFF", 0.36f),
                this,
                menu_selector(Panel::onToggle)
            );

            item->setTag(tag);
            item->setPosition({570.f, y});

            menu->addChild(item);
        }

        void addNote(
            char const* text,
            float y
        ) {
            auto note = label(text, 0.34f);
            note->setPosition({250.f, y});
            m_content->addChild(note);
        }

        void rebuild() {
            if (m_content) {
                m_content->removeFromParentAndCleanup(true);
            }

            m_content = CCNode::create();
            this->addChild(m_content);

            auto categories = CCMenu::create();
            categories->setPosition({0.f, 0.f});
            m_content->addChild(categories);

            addCategory(
                categories,
                "LEVEL",
                Category::Level,
                330.f
            );

            addCategory(
                categories,
                "COUNTER",
                Category::Counter,
                275.f
            );

            addCategory(
                categories,
                "SPEED",
                Category::Speed,
                220.f
            );

            addCategory(
                categories,
                "COSMETIC",
                Category::Cosmetic,
                165.f
            );

            addCategory(
                categories,
                "CONFIG",
                Category::Config,
                110.f
            );

            auto options = CCMenu::create();
            options->setPosition({0.f, 0.f});
            m_content->addChild(options);

            if (m_category == Category::Level) {
                addToggle(
                    options,
                    "NOCLIP",
                    state.noclip,
                    1,
                    330.f
                );

                addToggle(
                    options,
                    "SHOW HITBOXES",
                    state.showHitboxes,
                    2,
                    275.f
                );

                addToggle(
                    options,
                    "HITBOX ON DEATH",
                    state.showHitboxesOnDeath,
                    3,
                    220.f
                );

                addToggle(
                    options,
                    "HITBOX TRAIL",
                    state.hitboxTrail,
                    4,
                    165.f
                );

                addToggle(
                    options,
                    "SHOW TRAJECTORY",
                    state.showTrajectory,
                    5,
                    110.f
                );

                addToggle(
                    options,
                    "PATHFINDER",
                    state.pathfinder,
                    6,
                    55.f
                );

                addNote(
                    "UI controls are ready; gameplay hooks are not connected yet.",
                    18.f
                );
            }
            else if (m_category == Category::Counter) {
                addToggle(
                    options,
                    "FRAME PERFECT COUNTER",
                    state.framePerfect,
                    7,
                    330.f
                );

                addToggle(
                    options,
                    "ACCURATE PERCENTAGE",
                    state.accuratePercentage,
                    8,
                    275.f
                );

                addToggle(
                    options,
                    "MACRO RECORD",
                    state.macroRecord,
                    9,
                    220.f
                );

                addToggle(
                    options,
                    "MACRO REPLAY",
                    state.macroReplay,
                    10,
                    165.f
                );

                addNote(
                    "Window counting will be connected after macro playback.",
                    90.f
                );
            }
            else if (m_category == Category::Speed) {
                addToggle(
                    options,
                    "SPEED HACK",
                    state.speedHack,
                    11,
                    330.f
                );

                addToggle(
                    options,
                    "PRACTICE MUSIC SYNC",
                    state.practiceMusicSync,
                    12,
                    275.f
                );

                addNote(
                    "Physics TPS: 240",
                    210.f
                );

                addNote(
                    "Speed: 1.00x",
                    165.f
                );
            }
            else if (m_category == Category::Cosmetic) {
                addToggle(
                    options,
                    "ICON HACK",
                    state.iconHack,
                    13,
                    330.f
                );

                addToggle(
                    options,
                    "LAYOUT MOD",
                    state.layoutMod,
                    14,
                    275.f
                );

                addNote(
                    "Cosmetic hooks will be added separately.",
                    210.f
                );
            }
            else {
                addToggle(
                    options,
                    "SAFETY NET",
                    state.safetyNet,
                    15,
                    330.f
                );

                addNote(
                    "PATHFINDER DEATH LIMIT: 10",
                    275.f
                );

                addNote(
                    "ALLOWED LIST: AREDL + ILL",
                    225.f
                );

                addNote(
                    "Settings are not saved yet.",
                    175.f
                );
            }
        }

    public:
        static Panel* create() {
            auto result = new Panel();

            if (
                result &&
                result->initWithColor(
                    {12, 16, 28, 248},
                    PANEL_WIDTH,
                    PANEL_HEIGHT
                )
            ) {
                result->autorelease();
                return result;
            }

            CC_SAFE_DELETE(result);
            return nullptr;
        }

        bool initWithColor(
            ccColor4B color,
            float width,
            float height
        ) {
            if (
                !CCLayerColor::initWithColor(
                    color,
                    width,
                    height
                )
            ) {
                return false;
            }

            auto size = CCDirector::get()->getWinSize();

            this->setPosition({
                (size.width - width) / 2.f,
                (size.height - height) / 2.f
            });

            this->setID("autobot-panel");

            auto title = label(
                "AUTOBOT",
                0.62f
            );

            title->setPosition({
                22.f,
                height - 28.f
            });

            this->addChild(title);

            auto closeSprite =
                CCSprite::createWithSpriteFrameName(
                    "GJ_closeBtn_001.png"
                );

            CCSprite* closeButton = closeSprite;

            if (!closeButton) {
                closeButton = button(
                    "X",
                    0.38f
                );
            }

            auto closeItem = CCMenuItemSpriteExtra::create(
                closeButton,
                this,
                menu_selector(Panel::onClose)
            );

            closeItem->setPosition({
                width - 30.f,
                height - 28.f
            });

            auto topMenu = CCMenu::create(
                closeItem,
                nullptr
            );

            topMenu->setPosition({
                0.f,
                0.f
            });

            this->addChild(topMenu, 10);

            rebuild();

            return true;
        }

        void onClose(CCObject*) {
            this->removeFromParentAndCleanup(true);
        }

        void onCategory(CCObject* sender) {
            auto item = static_cast<CCMenuItem*>(sender);

            m_category = static_cast<Category>(
                item->getTag()
            );

            rebuild();
        }

        void onToggle(CCObject* sender) {
            auto item = static_cast<CCMenuItem*>(sender);

            switch (item->getTag()) {
                case 1:
                    state.noclip = !state.noclip;
                    break;

                case 2:
                    state.showHitboxes =
                        !state.showHitboxes;
                    break;

                case 3:
                    state.showHitboxesOnDeath =
                        !state.showHitboxesOnDeath;
                    break;

                case 4:
                    state.hitboxTrail =
                        !state.hitboxTrail;
                    break;

                case 5:
                    state.showTrajectory =
                        !state.showTrajectory;
                    break;

                case 6:
                    state.pathfinder =
                        !state.pathfinder;
                    break;

                case 7:
                    state.framePerfect =
                        !state.framePerfect;
                    break;

                case 8:
                    state.accuratePercentage =
                        !state.accuratePercentage;
                    break;

                case 9:
                    state.macroRecord =
                        !state.macroRecord;
                    state.macroReplay = false;
                    break;

                case 10:
                    state.macroReplay =
                        !state.macroReplay;
                    state.macroRecord = false;
                    break;

                case 11:
                    state.speedHack =
                        !state.speedHack;
                    break;

                case 12:
                    state.practiceMusicSync =
                        !state.practiceMusicSync;
                    break;

                case 13:
                    state.iconHack =
                        !state.iconHack;
                    break;

                case 14:
                    state.layoutMod =
                        !state.layoutMod;
                    break;

                case 15:
                    state.safetyNet =
                        !state.safetyNet;
                    break;

                default:
                    break;
            }

            rebuild();
        }
    };

    class DraggableButton final : public CCLayer {
    private:
        CCNode* m_host = nullptr;
        CCPoint m_startTouch = CCPointZero;
        CCPoint m_startPosition = CCPointZero;
        bool m_dragged = false;

    public:
        static DraggableButton* create(CCNode* host) {
            auto result = new DraggableButton();

            if (result && result->init(host)) {
                result->autorelease();
                return result;
            }

            CC_SAFE_DELETE(result);
            return nullptr;
        }

        bool init(CCNode* host) {
            if (!CCLayer::init()) {
                return false;
            }

            m_host = host;

            this->setContentSize({
                76.f,
                76.f
            });

            this->setTouchEnabled(true);
            this->setID("autobot-button-ui");

            auto icon =
                CCSprite::createWithSpriteFrameName(
                    "GJ_optionsBtn_001.png"
                );

            if (icon) {
                icon->setScale(0.55f);
            }
            else {
                icon = button(
                    "AB",
                    0.55f
                );
            }

            icon->setPosition({
                38.f,
                38.f
            });

            this->addChild(icon);

            auto size = CCDirector::get()->getWinSize();

            this->setPosition({
                size.width - 78.f,
                size.height * 0.52f
            });

            return true;
        }

        bool ccTouchBegan(
            CCTouch* touch,
            CCEvent*
        ) override {
            auto point =
                this->convertTouchToNodeSpace(touch);

            if (
                point.x < 0.f ||
                point.y < 0.f ||
                point.x > 76.f ||
                point.y > 76.f
            ) {
                return false;
            }

            m_startTouch = touch->getLocation();
            m_startPosition = this->getPosition();
            m_dragged = false;

            return true;
        }

        void ccTouchMoved(
            CCTouch* touch,
            CCEvent*
        ) override {
            auto now = touch->getLocation();
            auto delta = now - m_startTouch;

            if (delta.getLength() > 5.f) {
                m_dragged = true;
            }

            if (!m_host) {
                return;
            }

            auto size = CCDirector::get()->getWinSize();
            auto position = m_startPosition + delta;

            position.x = std::max(
                38.f,
                std::min(
                    size.width - 38.f,
                    position.x
                )
            );

            position.y = std::max(
                38.f,
                std::min(
                    size.height - 38.f,
                    position.y
                )
            );

            this->setPosition(position);
        }

        void ccTouchEnded(
            CCTouch*,
            CCEvent*
        ) override {
            if (
                m_dragged ||
                !m_host ||
                m_host->getChildByID("autobot-panel")
            ) {
                return;
            }

            auto panel = Panel::create();

            if (panel) {
                m_host->addChild(
                    panel,
                    Z_ORDER + 1
                );
            }
        }
    };

    inline void attachTo(CCNode* layer) {
        if (
            !layer ||
            layer->getChildByID("autobot-button-ui")
        ) {
            return;
        }

        auto button = DraggableButton::create(layer);

        if (button) {
            layer->addChild(
                button,
                Z_ORDER
            );
        }
    }
}

class $modify(AutoBotMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        AutoBotUI::attachTo(this);
        return true;
    }
};

class $modify(AutoBotLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject* searchObject) {
        if (!LevelBrowserLayer::init(searchObject)) {
            return false;
        }

        AutoBotUI::attachTo(this);
        return true;
    }
};

$on_mod(Loaded) {
    log::info(
        "AutoBot loaded for Geode 6.0.0 / Geometry Dash 2.2091"
    );
}
