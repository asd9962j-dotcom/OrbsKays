// Unlock All (solo local): hace que el juego responda "desbloqueado" a todo.
// No escribe nada en el guardado, asi que no hay nada que la nube pueda revertir.
#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>

using namespace geode::prelude;

$execute {
    log::info("UnlockAll cargado: hooks de GameManager y GameStatsManager activos");
}

// Iconos (cubo, nave, bola, ufo, wave, robot, spider, swing, jetpack),
// colores y glow.
class $modify(UnlockAllGameManager, GameManager) {
    bool isIconUnlocked(int id, IconType type) {
        return true;
    }

    bool isColorUnlocked(int id, UnlockType type) {
        return true;
    }
};

// Estelas, efectos de muerte, fuego de nave, items de tienda y resto de
// cosmeticos: el juego los consulta por UnlockType.
class $modify(UnlockAllStatsManager, GameStatsManager) {
    bool isItemUnlocked(UnlockType type, int id) {
        return true;
    }

    bool isStoreItemUnlocked(int index) {
        return true;
    }
};
