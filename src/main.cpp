#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

namespace AutoBotUI {
    constexpr int Z_ORDER = 1000;

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
        bool accuratePercentage = true;
        bool macroRecord = false;
        bool macroReplay = false;
        bool pathfinder = false;
        bool framePerfect = false;
        bool speedHack = false;
        bool practiceMusicSync = false;
        bool iconHack = false;
        bool layoutMod = false;
        bool safetyNet = true;
    };

    inline State state;

    class AutoBotPanel final : public CCLayerColor {
    private:
        Category m_category = Category::Level;
        CCNode* m_content = nullptr;

        static ButtonSprite* makeButton(
            char const* text,
            float scale = 0.52f
        ) {
            return ButtonSprite::create(
                text,
                "goldFont.fnt",
                "GJ_button_01.png",
                scale
            );
        }

        static CCLabelBMFont* makeLabel(
            char const* text,
            float scale = 0.55f
        ) {
            auto label = CCLabelBMFont::create(
                text,
                "bigFont.fnt"
            );

            label->setScale(scale);
            label->setAnchorPoint({0.f, 0.5f});

            return label;
        }

        void addCategoryButton(
            CCMenu* menu,
            char const* text,
            Category category,
            float y
        ) {
            auto button = makeButton(text, 0.48f);

            auto item = CCMenuItemSpriteExtra::create(
                button,
                this,
                menu_selector(AutoBotPanel::onCategory)
            );

            item->setTag(static_cast<int>(category));
            item->setPosition({125.f, y});

            menu->addChild(item);
        }

        void addToggle(
            CCMenu* menu,
            char const* text,
            bool value,
            int tag,
            float x,
            float y
        ) {
            auto label = makeLabel(text, 0.48f);
            label->setPosition({x, y});
            m_content->addChild(label);

            auto button = makeButton(
                value ? "ON" : "OFF",
                0.42f
            );

            auto item = CCMenuItemSpriteExtra::create(
                button,
                this,
                menu_selector(AutoBotPanel::onToggle)
            );

            item->setTag(tag);
            item->setPosition({x + 245.f, y});

            menu->addChild(item);
        }

        void addInfo(
            CCNode* parent,
            char const* text,
            float x,
            float y
        ) {
            auto label = makeLabel(text, 0.42f);
            label->setPosition({x, y});
            parent->addChild(label);
        }

        void buildContent() {
            if (m_content) {
                m_content->removeFromParentAndCleanup(true);
            }

            m_content = CCNode::create();
            m_content->setID("autobot-menu-content");
            this->addChild(m_content);

            auto categoryMenu = CCMenu::create();
            categoryMenu->setPosition({0.f, 0.f});
            m_content->addChild(categoryMenu);

            addCategoryButton(
                categoryMenu,
                "LEVEL",
                Category::Level,
                455.f
            );

            addCategoryButton(
                categoryMenu,
                "COUNTER",
                Category::Counter,
                390.f
            );

            addCategoryButton(
                categoryMenu,
                "SPEED",
                Category::Speed,
                325.f
            );

            addCategoryButton(
                categoryMenu,
                "COSMETIC",
                Category::Cosmetic,
                260.f
            );

            addCategoryButton(
                categoryMenu,
                "CONFIG",
                Category::Config,
                195.f
            );

            auto optionMenu = CCMenu::create();
            optionMenu->setPosition({0.f, 0.f});
            m_content->addChild(optionMenu);

            if (m_category == Category::Level) {
                addToggle(
                    optionMenu,
                    "NOCLIP",
                    state.noclip,
                    1,
                    380.f,
                    455.f
                );

                addToggle(
                    optionMenu,
                    "SHOW HITBOXES",
                    state.showHitboxes,
                    2,
                    380.f,
                    390.f
                );

                addToggle(
                    optionMenu,
                    "HITBOX ON DEATH",
                    state.showHitboxesOnDeath,
                    3,
                    380.f,
                    325.f
                );

                addToggle(
                    optionMenu,
                    "HITBOX TRAIL",
                    state.hitboxTrail,
                    4,
                    380.f,
                    260.f
                );

                addToggle(
                    optionMenu,
                    "SHOW TRAJECTORY",
                    state.showTrajectory,
                    5,
                    380.f,
                    195.f
                );

                addToggle(
                    optionMenu,
                    "PATHFINDER",
                    state.pathfinder,
                    6,
                    380.f,
                    130.f
                );

                addInfo(
                    m_content,
                    "Experimental features may fail on complex levels.",
                    380.f,
                    72.f
                );
            }
            else if (m_category == Category::Counter) {
                addToggle(
                    optionMenu,
                    "FRAME PERFECT COUNTER",
                    state.framePerfect,
                    7,
                    380.f,
                    455.f
                );

                addToggle(
                    optionMenu,
                    "ACCURATE PERCENTAGE",
                    state.accuratePercentage,
                    8,
                    380.f,
                    390.f
                );

                addToggle(
                    optionMenu,
                    "MACRO RECORD",
                    state.macroRecord,
                    9,
                    380.f,
                    325.f
                );

                addToggle(
                    optionMenu,
                    "MACRO REPLAY",
                    state.macroReplay,
                    10,
                    380.f,
                    260.f
                );

                addInfo(
                    m_content,
                    "Window counting will be added to the counter module.",
                    380.f,
                    195.f
                );
            }
            else if (m_category == Category::Speed) {
                addToggle(
                    optionMenu,
                    "SPEED HACK",
                    state.speedHack,
                    11,
                    380.f,
                    455.f
                );

                addToggle(
                    optionMenu,
                    "PRACTICE MUSIC SYNC",
                    state.practiceMusicSync,
                    12,
                    380.f,
                    390.f
                );

                addInfo(
                    m_content,
                    "Physics TPS: 240",
                    380.f,
                    325.f
                );

                addInfo(
                    m_content,
                    "Speed: 1.00x",
                    380.f,
                    260.f
                );
            }
            else if (m_category == Category::Cosmetic) {
                addToggle(
                    optionMenu,
                    "ICON HACK",
                    state.iconHack,
                    13,
                    380.f,
                    455.f
                );

                addToggle(
                    optionMenu,
                    "LAYOUT MOD",
                    state.layoutMod,
                    14,
                    380.f,
                    390.f
                );

                addInfo(
                    m_content,
                    "Cosmetic tools are English-only.",
                    380.f,
                    325.f
                );
            }
            else if (m_category == Category::Config) {
                addToggle(
                    optionMenu,
                    "SAFETY NET",
                    state.safetyNet,
                    15,
                    380.f,
                    455.f
                );

                addInfo(
                    m_content,
                    "Pathfinder death limit: 10",
                    380.f,
                    390.f
                );

                addInfo(
                    m_content,
                    "Allowed list: AREDL + ILL",
                    380.f,
                    325.f
                );

                addInfo(
                    m_content,
                    "Settings are saved in a later build.",
                    380.f,
                    260.f
                );
            }
        }

    public:
        static AutoBotPanel* create() {
            auto ret = new AutoBotPanel();

            if (
                ret &&
                ret->initWithColor(
                    {15, 18, 32, 245},
                    850.f,
                    540.f
                )
            ) {
                ret->autorelease();
                return ret;
            }

            CC_SAFE_DELETE(ret);
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

            this->setID("autobot-panel");

            auto windowSize = CCDirector::get()->getWinSize();

            this->setPosition({
                windowSize.width / 2.f - width / 2.f,
                windowSize.height / 2.f - height / 2.f
            });

            auto title = makeLabel(
                "AUTOBOT",
                0.72f
            );

            title->setPosition({
                32.f,
                height - 34.f
            });

            this->addChild(title);

            auto close = makeButton(
                "CLOSE",
                0.42f
            );

            auto closeItem = CCMenuItemSpriteExtra::create(
                close,
                this,
                menu_selector(AutoBotPanel::onClose)
            );

            closeItem->setPosition({
                width - 70.f,
                height - 32.f
            });

            auto topMenu = CCMenu::create(
                closeItem,
                nullptr
            );

            topMenu->setPosition({
                0.f,
                0.f
            });

            this->addChild(topMenu, 2);

            buildContent();

            return true;
        }

        void onCategory(CCObject* sender) {
            auto item = static_cast<CCMenuItem*>(sender);

            m_category = static_cast<Category>(
                item->getTag()
            );

            buildContent();
        }

        void onToggle(CCObject* sender) {
            auto item = static_cast<CCMenuItem*>(sender);

            switch (item->getTag()) {
                case 1:
                    state.noclip = !state.noclip;
                    break;

                case 2:
                    state.showHitboxes = !state.showHitboxes;
                    break;

                case 3:
                    state.showHitboxesOnDeath =
                        !state.showHitboxesOnDeath;
                    break;

                case 4:
                    state.hitboxTrail = !state.hitboxTrail;
                    break;

                case 5:
                    state.showTrajectory =
                        !state.showTrajectory;
                    break;

                case 6:
                    state.pathfinder = !state.pathfinder;
                    break;

                case 7:
                    state.framePerfect = !state.framePerfect;
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

            buildContent();
        }

        void onClose(CCObject*) {
            this->removeFromParentAndCleanup(true);
        }
    };

    class AutoBotButton final : public CCNode {
    private:
        CCNode* m_host = nullptr;

    public:
        static AutoBotButton* create(CCNode* host) {
            auto ret = new AutoBotButton();

            if (ret && ret->init(host)) {
                ret->autorelease();
                return ret;
            }

            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        bool init(CCNode* host) {
            if (!CCNode::init()) {
                return false;
            }

            m_host = host;

            auto menu = CCMenu::create();

            auto icon = CCSprite::createWithSpriteFrameName(
                "GJ_optionsBtn_001.png"
            );

            if (!icon) {
                icon = ButtonSprite::create(
                    "AB",
                    "goldFont.fnt",
                    "GJ_button_01.png",
                    0.62f
                );
            }
            else {
                icon->setScale(0.62f);
            }

            auto item = CCMenuItemSpriteExtra::create(
                icon,
                this,
                menu_selector(AutoBotButton::onPressed)
            );

            item->setID("autobot-button");

            menu->addChild(item);
            menu->setPosition({0.f, 0.f});
            this->addChild(menu);

            auto size = CCDirector::get()->getWinSize();

            this->setPosition({
                size.width - 48.f,
                size.height * 0.52f
            });

            this->setID("autobot-button-ui");

            return true;
        }

        void onPressed(CCObject*) {
            if (
                !m_host ||
                m_host->getChildByID("autobot-panel")
            ) {
                return;
            }

            auto panel = AutoBotPanel::create();

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

        auto button = AutoBotButton::create(layer);

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
