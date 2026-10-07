#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include <string>

using namespace geode::prelude;

namespace {
	struct Resource {
		const char* name;
		const char* statId; // clave de GameStatsManager::getStat / setStat
		int amount;
	};

	// IDs deducidos (no hay lista oficial). Si alguno no coincide, solo cambia statId aqui.
	constexpr Resource RESOURCES[] = {
		{"Mana Orbs", "14", 3000000},
		{"Diamonds", "13", 50000},
		{"Demon Keys", "21", 5000},
        {"Diamond Shards", "28", 5000},  // Diamantes pequeños / Esquirlas para la tienda
	};

	// true  = solo sube: si ya tienes mas que el valor indicado, no lo toca.
	// false = asigna el valor exacto en cada arranque.
	constexpr bool NEVER_LOWER = false;

	// true = escribe en el log los stats 1..40 (solo lectura) para identificar IDs.
	constexpr bool DUMP_STATS = true;
}

class $modify(AutoBotMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) {
			return false;
		}

		// MenuLayer::init se ejecuta cada vez que vuelves al menu: aplicar solo una vez por sesion.
		static bool applied = false;
		if (applied) {
			return true;
		}
		applied = true;

		auto stats = GameStatsManager::sharedState();
		if (!stats) {
			return true;
		}

		if (DUMP_STATS) {
			for (int i = 1; i <= 40; ++i) {
				std::string key = std::to_string(i);
				log::info("AutoBot stat {} = {}", key, stats->getStat(key.c_str()));
			}
		}

		for (auto const& r : RESOURCES) {
			int before = stats->getStat(r.statId);
			if (NEVER_LOWER && before >= r.amount) {
				log::info("AutoBot: {} (stat {}) ya tiene {}, se deja igual", r.name, r.statId, before);
				continue;
			}
			stats->setStat(r.statId, r.amount);
			log::info("AutoBot: {} (stat {}): {} -> {}", r.name, r.statId, before, stats->getStat(r.statId));
		}

		GameManager::sharedState()->save();
		return true;
	}
};
