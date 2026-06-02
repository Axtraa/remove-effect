#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCCircleWave.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

#ifdef GEODE_IS_WINDOWS
    #include <gl/GL.h>
#elif defined(GEODE_IS_MACOS)
    #include <OpenGL/gl.h>
#elif defined(GEODE_IS_ANDROID)
    #include <GLES2/gl2.h>
#elif defined(GEODE_IS_IOS)
    #include <OpenGLES/ES2/gl.h>
#endif

using namespace geode::prelude;

class $modify (PlayLayer) {
    struct Fields {
        bool m_autoTapTriggered = false;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        m_fields->m_autoTapTriggered = false;
    }

    void resetLevel() {
        m_fields->m_autoTapTriggered = false;
        PlayLayer::resetLevel();
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

        handleAutoTap();
    }

    void handleAutoTap() {
        if (!Mod::get()->getSettingValue<bool>("auto-tap-enabled")) {
            m_fields->m_autoTapTriggered = false;
            return;
        }

        auto currentPct = getCurrentPercent();
        auto targetPct = Mod::get()->getSettingValue<float>("auto-tap-percentage");

        if (currentPct >= targetPct && !m_fields->m_autoTapTriggered) {
            m_fields->m_autoTapTriggered = true;

            for (auto& kb : Mod::get()->getSettingValue<std::vector<Keybind>>("auto-tap-key")) {
                auto dispatcher = CCDirector::sharedDirector()->getKeyboardDispatcher();
                dispatcher->dispatchKeyboardMSG(kb.key, true, false, 0.0);
                dispatcher->dispatchKeyboardMSG(kb.key, false, false, 0.0);
            }
        }
    }

    void visit() {
        if (Mod::get()->getSettingValue<bool>("force-16-9")) {
            apply169Viewport();
        }
        PlayLayer::visit();
        if (Mod::get()->getSettingValue<bool>("force-16-9")) {
            restoreViewport();
        }
    }

    void apply169Viewport() {
        auto view = CCEGLView::sharedOpenGLView();
        auto frameSize = view->getFrameSize();
        constexpr float targetRatio = 16.0f / 9.0f;
        float currentRatio = frameSize.width / frameSize.height;

        if (std::abs(currentRatio - targetRatio) > 0.005f) {
            if (currentRatio > targetRatio) {
                int w = static_cast<int>(frameSize.height * targetRatio);
                int h = static_cast<int>(frameSize.height);
                int x = (static_cast<int>(frameSize.width) - w) / 2;
                glViewport(x, 0, w, h);
            } else {
                int w = static_cast<int>(frameSize.width);
                int h = static_cast<int>(frameSize.width / targetRatio);
                int y = (static_cast<int>(frameSize.height) - h) / 2;
                glViewport(0, y, w, h);
            }
        }
    }

    void restoreViewport() {
        auto view = CCEGLView::sharedOpenGLView();
        auto frameSize = view->getFrameSize();
        glViewport(0, 0, static_cast<int>(frameSize.width), static_cast<int>(frameSize.height));
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
