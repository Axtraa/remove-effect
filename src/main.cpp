#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCCircleWave.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class $modify (PlayLayer) {
    virtual void updateVisibility(float dt) {
        auto fmod = FMODAudioEngine::get();
        auto pulse1 = fmod->m_pulse1;
        auto audioScale = m_audioEffectsLayer->m_audioScale;

        if (Mod::get()->getSettingValue<bool>("no-orb-pulse")) {
            fmod->m_pulse1 = 0.7f;
            m_audioEffectsLayer->m_audioScale = 0.7f;
        }

        PlayLayer::updateVisibility(dt);

        fmod->m_pulse1 = pulse1;
        m_audioEffectsLayer->m_audioScale = audioScale;
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
        buttonSprite->setScale(0.45f);

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
