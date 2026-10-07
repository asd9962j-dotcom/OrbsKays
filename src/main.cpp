// Unlock All (solo local) + recordar la skin equipada entre reinicios.
// - Los hooks de "unlock" no escriben nada en el guardado del juego.
// - La seleccion equipada se guarda en el save data del propio mod
//   (no depende de que el juego la considere comprada).
#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/binding/GJGarageLayer.hpp>

using namespace geode::prelude;

$execute {
    log::info("UnlockAll cargado: unlock + persistencia de skin equipada");
}

// ---------------------------------------------------------------------------
// Estado equipado: se guarda en el save data del mod
// ---------------------------------------------------------------------------
namespace Keep {
    struct Slot {
        char const* key;
        int (*get)(GameManager*);
        void (*set)(GameManager*, int);
    };

#define KEEP_SLOT(key, Getter, Setter) \
    { key, [](GameManager* g) -> int { return (int)g->Getter(); }, \
           [](GameManager* g, int v) { g->Setter(v); } }

    inline Slot const SLOTS[] = {
        KEEP_SLOT("frame",       getPlayerFrame,       setPlayerFrame),
        KEEP_SLOT("ship",        getPlayerShip,        setPlayerShip),
        KEEP_SLOT("ball",        getPlayerBall,        setPlayerBall),
        KEEP_SLOT("ufo",         getPlayerBird,        setPlayerBird),
        KEEP_SLOT("wave",        getPlayerDart,        setPlayerDart),
        KEEP_SLOT("robot",       getPlayerRobot,       setPlayerRobot),
        KEEP_SLOT("spider",      getPlayerSpider,      setPlayerSpider),
        KEEP_SLOT("swing",       getPlayerSwing,       setPlayerSwing),
        KEEP_SLOT("jetpack",     getPlayerJetpack,     setPlayerJetpack),
        KEEP_SLOT("color1",      getPlayerColor,       setPlayerColor),
        KEEP_SLOT("color2",      getPlayerColor2,      setPlayerColor2),
        KEEP_SLOT("glowcolor",   getPlayerGlowColor,   setPlayerColor3),
        KEEP_SLOT("streak",      getPlayerStreak,      setPlayerStreak),
        KEEP_SLOT("shipfire",    getPlayerShipFire,    setPlayerShipStreak),
        KEEP_SLOT("deatheffect", getPlayerDeathEffect, setPlayerDeathEffect),
        KEEP_SLOT("glow",        getPlayerGlow,        setPlayerGlow),
    };
#undef KEEP_SLOT

    inline bool wasInGarage = false;
    inline float timer = 0.f;

    inline void restore(GameManager* gm) {
        if (!gm) return;
        auto mod = Mod::get();
        for (auto const& s : SLOTS) {
            if (mod->hasSavedValue(s.key)) {
                s.set(gm, mod->getSavedValue<int>(s.key));
            }
        }
        if (mod->hasSavedValue("icontype")) {
            gm->m_playerIconType = (IconType)mod->getSavedValue<int>("icontype");
        }
    }

    inline void snapshot(GameManager* gm) {
        auto mod = Mod::get();
        bool changed = false;
        for (auto const& s : SLOTS) {
            int cur = s.get(gm);
            if (!mod->hasSavedValue(s.key) || mod->getSavedValue<int>(s.key) != cur) {
                mod->setSavedValue<int>(s.key, cur);
                changed = true;
            }
        }
        int t = (int)gm->m_playerIconType;
        if (!mod->hasSavedValue("icontype") || mod->getSavedValue<int>("icontype") != t) {
            mod->setSavedValue<int>("icontype", t);
            changed = true;
        }
        // Escribir ya a disco: en Android el juego suele cerrarse sin avisar.
        if (changed) (void)mod->saveData();
    }

    // Solo se guarda mientras el Icon Kit esta abierto (y una vez al salir), asi un reseteo
    // interno del juego nunca pisa lo que ya teniamos guardado.
    inline void tick(GameManager* gm, float dt) {
        auto scene = CCDirector::get()->getRunningScene();
        bool inGarage = scene && scene->getChildByType<GJGarageLayer>(0) != nullptr;
        timer += dt;
        if ((inGarage && timer >= 0.5f) || (wasInGarage && !inGarage)) {
            timer = 0.f;
            snapshot(gm);
        }
        wasInGarage = inGarage;
    }
}

// ---------------------------------------------------------------------------
// Desbloquear todo
// ---------------------------------------------------------------------------
class $modify(UnlockAllGameManager, GameManager) {
    bool isIconUnlocked(int id, IconType type) {
        return true;
    }

    bool isColorUnlocked(int id, UnlockType type) {
        return true;
    }

    // -----------------------------------------------------------------------
    // Persistencia de lo equipado
    // -----------------------------------------------------------------------
    void dataLoaded(DS_Dictionary* dict) {
        GameManager::dataLoaded(dict);
        Keep::restore(this);   // el juego pudo resetear iconos "no comprados" al cargar
    }

    void update(float dt) {
        GameManager::update(dt);
        Keep::tick(this, dt);
    }
};

class $modify(UnlockAllStatsManager, GameStatsManager) {
    bool isItemUnlocked(UnlockType type, int id) {
        return true;
    }

    bool isStoreItemUnlocked(int index) {
        return true;
    }
};

// Reaplicar una vez al abrir el menu (antes de crearlo, para que se vea el icono correcto).
class $modify(UnlockAllMenuLayer, MenuLayer) {
    bool init() {
        static bool once = false;
        if (!once) {
            once = true;
            Keep::restore(GameManager::get());
        }
        return MenuLayer::init();
    }
};
