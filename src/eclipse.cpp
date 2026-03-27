#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_WINDOWS
    #define ECLIPSE_DONT_DEFINE_IMPLS
#endif

#include <events.hpp>
#include <modules.hpp>
#include <components.hpp>

using namespace eclipse;

static std::shared_ptr<SettingV3<bool>> s_noOrbPulseSetting;
static std::shared_ptr<SettingV3<bool>> s_noCirclesSetting;

static void initEclipseAPI() {
    auto mod = Mod::get();
    
    s_noOrbPulseSetting = mod->getSettingObject<bool>("no-orb-pulse");
    s_noCirclesSetting = mod->getSettingObject<bool>("no-circles");
    
    const auto& api = eclipse::__internal__::getVTable();
    
    if (!api.CreateMenuTab) {
        return;
    }
    
    auto tab = MenuTab::find("Remove Effect");
    
    tab->addModSettingToggle(s_noOrbPulseSetting)
        ->setDescription("Disables orbs from pulsing");
    
    tab->addModSettingToggle(s_noCirclesSetting)
        ->setDescription("Disables all circle effects");
    
    eclipse::modules::registerCheat("No Orb Pulse", []() {
        return Mod::get()->getSettingValue<bool>("no-orb-pulse");
    });
    
    eclipse::modules::registerCheat("No Circles", []() {
        return Mod::get()->getSettingValue<bool>("no-circles");
    });
}

$execute {
    initEclipseAPI();
}
