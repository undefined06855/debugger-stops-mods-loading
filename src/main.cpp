#include <Geode/Geode.hpp>
#include <debugapi.h>
#include <windows.h>

std::vector<std::string> g_devIDs = {};
std::vector<std::string> g_modIDs = {};
std::vector<std::string> g_modBlacklistIDs = {};

std::vector<std::string> splitString(const std::string& str, const char delimiter) {
    std::vector<std::string> ret;
    std::string part;
    
    for (char character : str) {
        if (character == delimiter) {
            // add current slice if delimiter found
            ret.push_back(part);
            part.clear();
        } else {
            part += character;
        }
    }

    ret.push_back(part); // add last part
    
    return ret;
}

bool check(const std::string id) {
    // if it's in either dev ids or mod ids and not in mod blacklist ids
    return \
        (
            std::find(g_devIDs.begin(), g_devIDs.end(), splitString(id, '.')[0]) != g_devIDs.end()
         || std::find(g_modIDs.begin(), g_modIDs.end(), id) != g_modIDs.end()
        )
     && std::find(g_modBlacklistIDs.begin(), g_modBlacklistIDs.end(), id) == g_modBlacklistIDs.end();
}

bool geode_Mod_shouldLoad(geode::Mod* self) {
    // check if this is just a mod that should be loaded
    if (check(self->getID())) {
        return self->shouldLoad();
    }

    // check if any dependants are mods that should be loaded
    for (auto mod : self->getDependants()) {
        if (check(mod->getID())) {
            return self->shouldLoad();
        }
    }

    // else DONT LOAD >:(
    return false;
}

$on_mod(Loaded) {
    // this mod used to be so that it shows a messagebox instead
    // but checking debugger is probably better 99% of the time
    // if (geode::Mod::get()->getSettingValue<bool>("msgbox")) {
    //     // messagebox option
    //     int res = MessageBoxA(
    //         NULL,
    //         "Do you want to load mods?",
    //         "Geometry Dash (Alt To Stop Mods Loading)",
    //         MB_YESNO | MB_ICONINFORMATION
    //     );

    //     if (res == IDYES) return;
    // } else {
    //     // alt option

    //     // VK_MENU is alt
    //     if (!(GetAsyncKeyState(VK_MENU) & (1 << 15))) return;
    // }

    if (!IsDebuggerPresent()) {
        geode::log::info("No debugger present!");
        return;
    }

    geode::log::info("Detected debugger!");

    (void)geode::Mod::get()->hook(
        reinterpret_cast<void*>(geode::addresser::getNonVirtual(&geode::Mod::shouldLoad)),
        &geode_Mod_shouldLoad,
        "geode::Mod::shouldLoad",
        tulip::hook::TulipConvention::Default
    );

    g_devIDs = splitString(geode::Mod::get()->getSettingValue<std::string>("devs"), ',');
    g_modIDs = splitString(geode::Mod::get()->getSettingValue<std::string>("mods"), ',');
    g_modBlacklistIDs = splitString(geode::Mod::get()->getSettingValue<std::string>("mods-blacklist"), ',');
}
