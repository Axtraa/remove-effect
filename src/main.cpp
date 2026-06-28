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

// VK codes as raw hex values (cross-platform, matches Windows VK_*)
static constexpr int RK_F1 = 0x70, RK_F12 = 0x7B;
static constexpr int RK_SPACE = 0x20, RK_RETURN = 0x0D, RK_TAB = 0x09, RK_BACK = 0x08;
static constexpr int RK_DELETE = 0x2E, RK_INSERT = 0x2D, RK_ESCAPE = 0x1B;
static constexpr int RK_HOME = 0x24, RK_END = 0x23, RK_PRIOR = 0x21, RK_NEXT = 0x22;
static constexpr int RK_UP = 0x26, RK_DOWN = 0x28, RK_LEFT = 0x25, RK_RIGHT = 0x27;
static constexpr int RK_LSHIFT = 0xA0, RK_RSHIFT = 0xA1;
static constexpr int RK_LCTRL = 0xA2, RK_RCTRL = 0xA3;
static constexpr int RK_LALT = 0xA4, RK_RALT = 0xA5;
static constexpr int RK_CAPITAL = 0x14;
static constexpr int RK_NUMPAD0 = 0x60, RK_NUMPAD9 = 0x69;
static constexpr int RK_MULTIPLY = 0x6A, RK_ADD = 0x6B;
static constexpr int RK_SUBTRACT = 0x6D, RK_DECIMAL = 0x6E, RK_DIVIDE = 0x6F;

static std::string keyNameFromVK(int vk) {
    if (vk >= RK_F1 && vk <= RK_F12)
        return "F" + std::to_string(vk - RK_F1 + 1);
    if (vk >= 'A' && vk <= 'Z')
        return std::string(1, static_cast<char>(vk));
    if (vk >= '0' && vk <= '9')
        return std::string(1, static_cast<char>(vk));

    switch (vk) {
        case RK_SPACE:    return "Space";
        case RK_RETURN:   return "Enter";
        case RK_TAB:      return "Tab";
        case RK_BACK:     return "Backspace";
        case RK_DELETE:   return "Delete";
        case RK_INSERT:   return "Insert";
        case RK_HOME:     return "Home";
        case RK_END:      return "End";
        case RK_PRIOR:    return "PageUp";
        case RK_NEXT:     return "PageDown";
        case RK_UP:       return "Up";
        case RK_DOWN:     return "Down";
        case RK_LEFT:     return "Left";
        case RK_RIGHT:    return "Right";
        case RK_LSHIFT:   return "LShift";
        case RK_RSHIFT:   return "RShift";
        case RK_LCTRL:    return "LCtrl";
        case RK_RCTRL:    return "RCtrl";
        case RK_LALT:     return "LAlt";
        case RK_RALT:     return "RAlt";
        case RK_CAPITAL:  return "CapsLock";
        case RK_NUMPAD0:  return "Num0";
        case 0x61: return "Num1";
        case 0x62: return "Num2";
        case 0x63: return "Num3";
        case 0x64: return "Num4";
        case 0x65: return "Num5";
        case 0x66: return "Num6";
        case 0x67: return "Num7";
        case 0x68: return "Num8";
        case RK_NUMPAD9:  return "Num9";
        case RK_MULTIPLY: return "Num*";
        case RK_ADD:      return "Num+";
        case RK_SUBTRACT: return "Num-";
        case RK_DECIMAL:  return "Num.";
        case RK_DIVIDE:   return "Num/";
        default:          return "";
    }
}

static int vkFromKeyName(const std::string& name) {
    if (name.empty()) return 0;

    if (name[0] == 'F' && name.size() > 1) {
        try {
            int n = std::stoi(name.substr(1));
            if (n >= 1 && n <= 12) return RK_F1 + n - 1;
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

    if (name == "Space")      return RK_SPACE;
    if (name == "Enter")      return RK_RETURN;
    if (name == "Tab")        return RK_TAB;
    if (name == "Backspace")  return RK_BACK;
    if (name == "Delete")     return RK_DELETE;
    if (name == "Insert")     return RK_INSERT;
    if (name == "Home")       return RK_HOME;
    if (name == "End")        return RK_END;
    if (name == "PageUp")     return RK_PRIOR;
    if (name == "PageDown")   return RK_NEXT;
    if (name == "Up")         return RK_UP;
    if (name == "Down")       return RK_DOWN;
    if (name == "Left")       return RK_LEFT;
    if (name == "Right")      return RK_RIGHT;
    if (name == "LShift")     return RK_LSHIFT;
    if (name == "RShift")     return RK_RSHIFT;
    if (name == "LCtrl")      return RK_LCTRL;
    if (name == "RCtrl")      return RK_RCTRL;
    if (name == "LAlt")       return RK_LALT;
    if (name == "RAlt")       return RK_RALT;
    if (name == "CapsLock")   return RK_CAPITAL;
    if (name == "Num0")       return RK_NUMPAD0;
    if (name == "Num1")       return 0x61;
    if (name == "Num2")       return 0x62;
    if (name == "Num3")       return 0x63;
    if (name == "Num4")       return 0x64;
    if (name == "Num5")       return 0x65;
    if (name == "Num6")       return 0x66;
    if (name == "Num7")       return 0x67;
    if (name == "Num8")       return 0x68;
    if (name == "Num9")       return RK_NUMPAD9;

    return 0;
}

// ============================================================
//  Windows key simulation
// ============================================================

// Check if a VK code is an extended key (needs KEYEVENTF_EXTENDEDKEY)
static bool isExtendedKey(int vk) {
    return (vk == 0x2D || vk == 0x2E || // Insert, Delete
            vk == 0x24 || vk == 0x23 || // Home, End
            vk == 0x21 || vk == 0x22 || // PageUp, PageDown
            (vk >= 0x25 && vk <= 0x28) || // Arrow keys
            vk == 0xA1 || vk == 0xA3 || vk == 0xA5 || // RShift, RCtrl, RAlt
            (vk >= 0x60 && vk <= 0x6F)); // Numpad
}

static void simulateKeyPress(int vkCode) {
    #ifdef GEODE_IS_WINDOWS
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(vkCode);
        if (isExtendedKey(vkCode)) input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
        SendInput(1, &input, sizeof(INPUT));
    #endif
}

static void simulateKeyRelease(int vkCode) {
    #ifdef GEODE_IS_WINDOWS
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(vkCode);
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        if (isExtendedKey(vkCode)) input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
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

        // --- panel background ---
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
        hint->setScale(0.4f);
        hint->setPosition({win.width / 2, win.height / 2 - 65});
        hint->setOpacity(100);
        addChild(hint);

        // snapshot current key state so we only detect NEW presses
        #ifdef GEODE_IS_WINDOWS
            for (int i = 0; i < 256; i++)
                m_prevState[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
        #endif

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
        int m_autoClickVK = 0;
        float m_autoClickTimer = 0.f;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        m_fields->m_autoClickFired = false;
        m_fields->m_autoClickVK = 0;
        m_fields->m_autoClickTimer = 0.f;
        update169Overlay();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->m_autoClickFired = false;
        m_fields->m_autoClickVK = 0;
        m_fields->m_autoClickTimer = 0.f;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        // Toggle back on death
        if (Mod::get()->getSettingValue<bool>("auto-click")) {
            std::string keyName = Mod::get()->getSavedValue<std::string>("auto-click-key", "F2");
            int vk = vkFromKeyName(keyName);
            if (vk != 0) {
                simulateKeyPress(vk);
                simulateKeyRelease(vk);
            }
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        // Toggle back on level end
        if (Mod::get()->getSettingValue<bool>("auto-click")) {
            std::string keyName = Mod::get()->getSavedValue<std::string>("auto-click-key", "F2");
            int vk = vkFromKeyName(keyName);
            if (vk != 0) {
                simulateKeyPress(vk);
                simulateKeyRelease(vk);
            }
        }
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
        // Phase 1: release key after timer expires
        if (m_fields->m_autoClickVK != 0) {
            m_fields->m_autoClickTimer -= dt;
            if (m_fields->m_autoClickTimer <= 0.f) {
                simulateKeyRelease(m_fields->m_autoClickVK);
                m_fields->m_autoClickVK = 0;
            }
        }
        // Phase 2: detect percentage and press key
        if (!m_fields->m_autoClickFired && Mod::get()->getSettingValue<bool>("auto-click") && m_level && m_player1) {
            float playerX = m_player1->getPositionX();
            float levelLen = m_levelLength;
            if (playerX > 0.f && levelLen > 0.f) {
                float triggerPct = Mod::get()->getSettingValue<float>("auto-click-percent");
                float currentPct = (playerX / levelLen) * 100.f;
                if (currentPct >= triggerPct) {
                    std::string keyName = Mod::get()->getSavedValue<std::string>("auto-click-key", "F2");
                    int vk = vkFromKeyName(keyName);
                    if (vk != 0) {
                        simulateKeyPress(vk);
                        m_fields->m_autoClickVK = vk;
                        m_fields->m_autoClickTimer = 0.1f; // hold 100ms
                    }
                    m_fields->m_autoClickFired = true;
                }
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
        auto keyBtn = CCMenuItemSpriteExtra::create(
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
