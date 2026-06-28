#ifdef GEODE_IS_WINDOWS
    #include <windows.h>
#endif

#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCCircleWave.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

// ============================================================
//  Key name ↔ Windows Virtual-Key code conversion
// ============================================================

static std::string keyNameFromVK(int vk) {
    if (vk >= VK_F1 && vk <= VK_F12)
        return "F" + std::to_string(vk - VK_F1 + 1);
    if (vk >= 'A' && vk <= 'Z')
        return std::string(1, static_cast<char>(vk));
    if (vk >= '0' && vk <= '9')
        return std::string(1, static_cast<char>(vk));

    switch (vk) {
        case VK_SPACE:      return "Space";
        case VK_RETURN:     return "Enter";
        case VK_TAB:        return "Tab";
        case VK_BACK:       return "Backspace";
        case VK_DELETE:     return "Delete";
        case VK_INSERT:     return "Insert";
        case VK_HOME:       return "Home";
        case VK_END:        return "End";
        case VK_PRIOR:      return "PageUp";
        case VK_NEXT:       return "PageDown";
        case VK_UP:         return "Up";
        case VK_DOWN:       return "Down";
        case VK_LEFT:       return "Left";
        case VK_RIGHT:      return "Right";
        case VK_LSHIFT:     return "LShift";
        case VK_RSHIFT:     return "RShift";
        case VK_LCONTROL:   return "LCtrl";
        case VK_RCONTROL:   return "RCtrl";
        case VK_LMENU:      return "LAlt";
        case VK_RMENU:      return "RAlt";
        case VK_CAPITAL:    return "CapsLock";
        case VK_NUMPAD0:    return "Num0";
        case VK_NUMPAD1:    return "Num1";
        case VK_NUMPAD2:    return "Num2";
        case VK_NUMPAD3:    return "Num3";
        case VK_NUMPAD4:    return "Num4";
        case VK_NUMPAD5:    return "Num5";
        case VK_NUMPAD6:    return "Num6";
        case VK_NUMPAD7:    return "Num7";
        case VK_NUMPAD8:    return "Num8";
        case VK_NUMPAD9:    return "Num9";
        case VK_MULTIPLY:   return "Num*";
        case VK_ADD:        return "Num+";
        case VK_SUBTRACT:   return "Num-";
        case VK_DECIMAL:    return "Num.";
        case VK_DIVIDE:     return "Num/";
        default:            return "";
    }
}

static int vkFromKeyName(const std::string& name) {
    if (name.empty()) return 0;

    if (name[0] == 'F' && name.size() > 1) {
        try {
            int n = std::stoi(name.substr(1));
            if (n >= 1 && n <= 12) return VK_F1 + n - 1;
        } catch (...) {}
        return 0;
    }

    if (name.size() == 1) {
        char c = name[0];
        if (c >= '0' && c <= '9') return c;
        if (c >= 'A' && c <= 'Z') return c;
        if (c >= 'a' && c <= 'z') return c - 32;
        return 0;
    }

    if (name == "Space")      return VK_SPACE;
    if (name == "Enter")      return VK_RETURN;
    if (name == "Tab")        return VK_TAB;
    if (name == "Backspace")  return VK_BACK;
    if (name == "Delete")     return VK_DELETE;
    if (name == "Insert")     return VK_INSERT;
    if (name == "Home")       return VK_HOME;
    if (name == "End")        return VK_END;
    if (name == "PageUp")     return VK_PRIOR;
    if (name == "PageDown")   return VK_NEXT;
    if (name == "Up")         return VK_UP;
    if (name == "Down")       return VK_DOWN;
    if (name == "Left")       return VK_LEFT;
    if (name == "Right")      return VK_RIGHT;
    if (name == "LShift")     return VK_LSHIFT;
    if (name == "RShift")     return VK_RSHIFT;
    if (name == "LCtrl")      return VK_LCONTROL;
    if (name == "RCtrl")      return VK_RCONTROL;
    if (name == "LAlt")       return VK_LMENU;
    if (name == "RAlt")       return VK_RMENU;
    if (name == "CapsLock")   return VK_CAPITAL;
    if (name == "Num0")       return VK_NUMPAD0;
    if (name == "Num1")       return VK_NUMPAD1;
    if (name == "Num2")       return VK_NUMPAD2;
    if (name == "Num3")       return VK_NUMPAD3;
    if (name == "Num4")       return VK_NUMPAD4;
    if (name == "Num5")       return VK_NUMPAD5;
    if (name == "Num6")       return VK_NUMPAD6;
    if (name == "Num7")       return VK_NUMPAD7;
    if (name == "Num8")       return VK_NUMPAD8;
    if (name == "Num9")       return VK_NUMPAD9;

    return 0;
}

// ============================================================
//  Windows key simulation
// ============================================================

static void simulateKeyPress(int vkCode) {
    #ifdef GEODE_IS_WINDOWS
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(vkCode);
        SendInput(1, &input, sizeof(INPUT));
    #endif
}

static void simulateKeyRelease(int vkCode) {
    #ifdef GEODE_IS_WINDOWS
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(vkCode);
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    #endif
}

// ============================================================
//  Key Record Layer  –  press a key to bind it
// ============================================================

class KeyRecordLayer : public CCLayerColor {
protected:
    #ifdef GEODE_IS_WINDOWS
        bool m_prevState[256] = {};
    #endif

    bool init() override {
        if (!CCLayerColor::initWithColor({0, 0, 0, 180})) return false;

        auto win = CCDirector::sharedDirector()->getWinSize();
        setContentSize(win);

        // --- panel ---
        auto panel = CCLayerColor::create({25, 25, 25, 230}, 340, 220);
        panel->setPosition({(win.width - 340) / 2.f, (win.height - 220) / 2.f});
        addChild(panel);

        // title
        auto title = CCLabelBMFont::create("RECORD KEY", "goldFont.fnt");
        title->setScale(0.7f);
        title->setPosition({win.width / 2, win.height / 2 + 70});
        addChild(title);

        // instruction
        auto instr = CCLabelBMFont::create("Press any key...", "bigFont.fnt");
        instr->setScale(0.55f);
        instr->setPosition({win.width / 2, win.height / 2 + 15});
        addChild(instr);

        // current binding
        std::string cur = Mod::get()->getSavedValue<std::string>("auto-click-key", "F2");
        auto curLabel = CCLabelBMFont::create(("Current: " + cur).c_str(), "goldFont.fnt");
        curLabel->setScale(0.45f);
        curLabel->setPosition({win.width / 2, win.height / 2 - 20});
        curLabel->setOpacity(140);
        addChild(curLabel);

        // cancel hint
        auto hint = CCLabelBMFont::create("ESC to cancel", "goldFont.fnt");
        hint->setScale(0.35f);
        hint->setPosition({win.width / 2, win.height / 2 - 60});
        hint->setOpacity(90);
        addChild(hint);

        // snapshot current key state so we only detect NEW presses
        #ifdef GEODE_IS_WINDOWS
            for (int i = 0; i < 256; i++)
                m_prevState[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
        #endif

        // touch to cancel
        auto touch = cocos2d::EventListenerTouchOneByOne::create();
        touch->setSwallowTouches(true);
        touch->onTouchBegan = [this](cocos2d::Touch*, cocos2d::Event*) {
            this->removeFromParentAndCleanup(true);
            return true;
        };
        CCDirector::sharedDirector()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touch, this);

        scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        CCLayerColor::update(dt);
        #ifdef GEODE_IS_WINDOWS
            for (int k = 1; k < 256; k++) {
                bool down = (GetAsyncKeyState(k) & 0x8000) != 0;
                if (down && !m_prevState[k]) {
                    if (k == VK_ESCAPE) {
                        removeFromParentAndCleanup(true);
                        return;
                    }
                    std::string name = keyNameFromVK(k);
                    if (!name.empty()) {
                        Mod::get()->setSavedValue("auto-click-key", name);
                        removeFromParentAndCleanup(true);
                        return;
                    }
                }
                m_prevState[k] = down;
            }
        #endif
    }

public:
    static KeyRecordLayer* create() {
        auto* ret = new KeyRecordLayer();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ============================================================
//  PlayLayer  –  auto-click + orb pulse + 16:9 overlay
// ============================================================

class $modify (PlayLayer) {
    struct Fields {
        CCNode* m_169overlay = nullptr;
        bool m_autoClickFired = false;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        update169Overlay();
    }

    void remove169Overlay() {
        if (m_fields->m_169overlay) {
            m_fields->m_169overlay->removeFromParent();
            m_fields->m_169overlay = nullptr;
        }
    }

    void update169Overlay() {
        remove169Overlay();
        if (!Mod::get()->getSettingValue<bool>("force-16-9")) return;

        auto ui = m_uiLayer;
        if (!ui) return;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float target = 16.f / 9.f;
        float current = winSize.width / winSize.height;
        if (std::abs(current - target) < 0.005f) return;

        auto overlay = CCNode::create();
        overlay->setID("169-overlay"_spr);
        overlay->setPosition({0, 0});

        if (current > target) {
            float barW = (winSize.width - winSize.height * target) / 2.f;
            auto left  = CCLayerColor::create({0,0,0,255}, barW, winSize.height);
            left->setPosition({0, 0});
            overlay->addChild(left);
            auto right = CCLayerColor::create({0,0,0,255}, barW, winSize.height);
            right->setPosition({winSize.width - barW, 0});
            overlay->addChild(right);
        } else {
            float barH = (winSize.height - winSize.width / target) / 2.f;
            auto top    = CCLayerColor::create({0,0,0,255}, winSize.width, barH);
            top->setPosition({0, winSize.height - barH});
            overlay->addChild(top);
            auto bottom = CCLayerColor::create({0,0,0,255}, winSize.width, barH);
            bottom->setPosition({0, 0});
            overlay->addChild(bottom);
        }

        ui->addChild(overlay, -999999);
        m_fields->m_169overlay = overlay;
    }

    virtual void updateVisibility(float dt) {
        auto* fmod = FMODAudioEngine::get();
        auto pulse1 = fmod->m_pulse1;
        auto audioScale = m_audioEffectsLayer->m_audioScale;

        if (Mod::get()->getSettingValue<bool>("no-orb-pulse")) {
            auto s = Mod::get()->getSettingValue<float>("orb-pulse-size");
            fmod->m_pulse1 = s;
            m_audioEffectsLayer->m_audioScale = s;
        }

        // --- Auto Click ---
        if (Mod::get()->getSettingValue<bool>("auto-click") && !m_fields->m_autoClickFired) {
            float pct   = Mod::get()->getSettingValue<float>("auto-click-percent");
            float cur   = m_level->m_normalPercent;
            if (cur >= pct) {
                std::string keyName = Mod::get()->getSavedValue<std::string>("auto-click-key", "F2");
                int vk = vkFromKeyName(keyName);
                if (vk != 0) {
                    simulateKeyPress(vk);
                    simulateKeyRelease(vk);
                }
                m_fields->m_autoClickFired = true;
            }
        }

        PlayLayer::updateVisibility(dt);

        fmod->m_pulse1 = pulse1;
        m_audioEffectsLayer->m_audioScale = audioScale;

        bool want169 = Mod::get()->getSettingValue<bool>("force-16-9");
        bool has169  = m_fields->m_169overlay != nullptr;
        if (want169 != has169) {
            want169 ? update169Overlay() : remove169Overlay();
        }
    }
};

// ============================================================
//  CCCircleWave  –  suppress circle effects
// ============================================================

class $modify (CCCircleWave) {
    virtual void draw() {
        if (Mod::get()->getSettingValue<bool>("no-circles")) return;
        CCCircleWave::draw();
    }
};

// ============================================================
//  PauseLayer  –  settings button + key-record button
// ============================================================

class $modify (RemoveEffectPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto menu = this->getChildByID("right-button-menu");
        if (!menu) return;

        // --- Settings button ---
        auto btnSprite = CircleButtonSprite::create(
            CCSprite::create("button.png"_spr)
        );
        btnSprite->setScale(0.675f);
        auto settingsBtn = CCMenuItemSpriteExtra::create(
            btnSprite, this,
            menu_selector(RemoveEffectPauseLayer::onOpenSettings)
        );
        settingsBtn->setID("remove-effect-settings-btn"_spr);
        menu->addChild(settingsBtn);

        // --- Record Key button ---
        std::string curKey = Mod::get()->getSavedValue<std::string>("auto-click-key", "F2");
        auto keyLabel = CCLabelBMFont::create(curKey.c_str(), "goldFont.fnt");
        keyLabel->setScale(0.55f);
        auto keyBtn = CCMenuItemLabel::create(
            keyLabel, this,
            menu_selector(RemoveEffectPauseLayer::onRecordKey)
        );
        keyBtn->setID("remove-effect-record-key-btn"_spr);
        menu->addChild(keyBtn);

        menu->updateLayout();
    }

    void onOpenSettings(CCObject*) {
        openSettingsPopup(Mod::get());
    }

    void onRecordKey(CCObject*) {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (scene) scene->addChild(KeyRecordLayer::create(), 999);
    }
};
