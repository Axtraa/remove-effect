#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCCircleWave.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class $modify (PlayLayer) {
    struct Fields {
        // Auto tap state
        bool m_autoTapTriggered = false;
        bool m_autoTapRelease = false;
        enumKeyCodes m_autoTapReleaseKey = KEY_None;

        // 16:9 overlay
        CCNode* m_169overlay = nullptr;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        m_fields->m_autoTapTriggered = false;
        m_fields->m_autoTapRelease = false;
        update169Overlay();
    }

    void resetLevel() {
        m_fields->m_autoTapTriggered = false;
        m_fields->m_autoTapRelease = false;
        PlayLayer::resetLevel();
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

        auto scene = this->getParent();
        if (!scene) return;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float targetRatio = 16.0f / 9.0f;
        float currentRatio = winSize.width / winSize.height;

        if (std::abs(currentRatio - targetRatio) < 0.005f) return;

        auto overlay = CCNode::create();
        overlay->setID("169-overlay"_spr);

        if (currentRatio > targetRatio) {
            // Screen wider than 16:9 → side bars
            float barW = (winSize.width - winSize.height * targetRatio) / 2.0f;

            auto left = CCLayerColor::create(ccc4(0, 0, 0, 255), barW, winSize.height);
            left->setAnchorPoint({0.0f, 0.0f});
            left->setPosition(0.0f, 0.0f);
            overlay->addChild(left);

            auto right = CCLayerColor::create(ccc4(0, 0, 0, 255), barW, winSize.height);
            right->setAnchorPoint({0.0f, 0.0f});
            right->setPosition(winSize.width - barW, 0.0f);
            overlay->addChild(right);
        } else {
            // Screen taller than 16:9 → top/bottom bars
            float barH = (winSize.height - winSize.width / targetRatio) / 2.0f;

            auto top = CCLayerColor::create(ccc4(0, 0, 0, 255), winSize.width, barH);
            top->setAnchorPoint({0.0f, 0.0f});
            top->setPosition(0.0f, winSize.height - barH);
            overlay->addChild(top);

            auto bottom = CCLayerColor::create(ccc4(0, 0, 0, 255), winSize.width, barH);
            bottom->setAnchorPoint({0.0f, 0.0f});
            bottom->setPosition(0.0f, 0.0f);
            overlay->addChild(bottom);
        }

        scene->addChild(overlay, 999999);
        m_fields->m_169overlay = overlay;
    }

    virtual void updateVisibility(float dt) {
        auto fmod = FMODAudioEngine::get();
        auto pulse1 = fmod->m_pulse1;
        auto audioScale = m_audioEffectsLayer->m_audioScale;

        if (Mod::get()->getSettingValue<bool>("no-orb-pulse")) {
            auto orbPulseSize = Mod::get()->getSettingValue<float>("orb-pulse-size");
            fmod->m_pulse1 = orbPulseSize;
            m_audioEffectsLayer->m_audioScale = orbPulseSize;
        }

        PlayLayer::updateVisibility(dt);

        fmod->m_pulse1 = pulse1;
        m_audioEffectsLayer->m_audioScale = audioScale;

        // Handle 16:9 overlay dynamic toggling
        bool want169 = Mod::get()->getSettingValue<bool>("force-16-9");
        bool has169 = m_fields->m_169overlay != nullptr;
        if (want169 != has169) {
            if (want169)
                update169Overlay();
            else
                remove169Overlay();
        }

        // Release auto-tap key on the next frame (via keyboard dispatcher)
        if (m_fields->m_autoTapRelease) {
            m_fields->m_autoTapRelease = false;
            CCKeyboardDispatcher::get()->dispatchKeyboardMSG(
                m_fields->m_autoTapReleaseKey, false, false, 0.0
            );
        }

        handleAutoTap();
    }

    void handleAutoTap() {
        if (!Mod::get()->getSettingValue<bool>("auto-tap-enabled")) {
            m_fields->m_autoTapTriggered = false;
            return;
        }

        auto currentPct = getCurrentPercent();
        auto targetPct = Mod::get()->getSettingValue<float>("auto-tap-percentage");

        // GD's getCurrentPercent() returns 0.0–100.0
        // But some versions return 0.0–1.0 — normalise if needed
        if (currentPct <= 1.0f) {
            currentPct *= 100.0f;
        }

        if (!m_fields->m_autoTapTriggered && currentPct >= targetPct) {
            m_fields->m_autoTapTriggered = true;

            for (auto const& kb : Mod::get()->getSettingValue<std::vector<Keybind>>("auto-tap-key")) {
                CCKeyboardDispatcher::get()->dispatchKeyboardMSG(
                    kb.key, true, false, 0.0
                );
                m_fields->m_autoTapRelease = true;
                m_fields->m_autoTapReleaseKey = kb.key;
            }
        }
    }
};

class $modify (CCCircleWave) {
    virtual void draw() {
        if (Mod::get()->getSettingValue<bool>("no-circles")) {
            return;
        }
        CCCircleWave::draw();
    }
};

class $modify (RemoveEffectPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto menu = this->getChildByID("right-button-menu");
        if (!menu) return;

        auto buttonSprite = CircleButtonSprite::create(
            CCSprite::create("button.png"_spr)
        );
        buttonSprite->setScale(0.675f);

        auto button = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(RemoveEffectPauseLayer::onOpenSettings)
        );
        button->setID("remove-effect-settings-btn"_spr);

        menu->addChild(button);
        menu->updateLayout();
    }

    void onOpenSettings(CCObject* sender) {
        openSettingsPopup(Mod::get());
    }
};
