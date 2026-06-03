#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCCircleWave.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class $modify (PlayLayer) {
    struct Fields {
        CCNode* m_169overlay = nullptr;
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

        // Add overlay bars as children of UILayer at very low z-order:
        // UILayer renders AFTER PlayLayer in the scene, so the bars cover the level.
        // With negative z-order, they render BEFORE UILayer's normal children
        // (buttons, labels, Eclipse overlays, etc.), so the UI stays on top.
        auto ui = m_uiLayer;
        if (!ui) return;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float targetRatio = 16.0f / 9.0f;
        float currentRatio = winSize.width / winSize.height;

        if (std::abs(currentRatio - targetRatio) < 0.005f) return;

        auto overlay = CCNode::create();
        overlay->setID("169-overlay"_spr);
        overlay->setAnchorPoint({0.0f, 0.0f});
        overlay->setPosition(0.0f, 0.0f);

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

        ui->addChild(overlay, -999999);
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

        // Toggle 16:9 overlay dynamically when setting changes
        bool want169 = Mod::get()->getSettingValue<bool>("force-16-9");
        bool has169 = m_fields->m_169overlay != nullptr;
        if (want169 != has169) {
            if (want169)
                update169Overlay();
            else
                remove169Overlay();
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
