#include <Windows.h>
#include "Hooks.h"

bool bPlayerVertibirdExplosion = true;
bool bUseVertibirdExplosionProbability = false;
float fVertibirdExplosionProbability = 0.5f;

std::string GetINIOption(const char* section, const char* key) {
	std::string	result;
	char resultBuf[256] = { 0 };

	static const std::string& configPath = fmt::format("Data\\F4SE\\Plugins\\{}.ini", Version::PROJECT);
	GetPrivateProfileStringA(section, key, NULL, resultBuf, sizeof(resultBuf), configPath.c_str());
	return resultBuf;
}

void ReadINI() {
	std::string value;
	
	value = GetINIOption("Settings", "bPlayerVertibirdExplosion");
	if (!value.empty()) {
		try {
			bPlayerVertibirdExplosion = std::stoul(value);
		}
		catch (const std::exception& e) {
			logger::error("bPlayerVertibirdExplosion: {}. Use default value of true", e.what());
		}
	}
	logger::info("bPlayerVertibirdExplosion: {}", bPlayerVertibirdExplosion);

	value = GetINIOption("Settings", "bUseVertibirdExplosionProbability");
	if (!value.empty()) {
		try {
			bUseVertibirdExplosionProbability = std::stoul(value);
		}
		catch (const std::exception& e) {
			logger::error("bUseVertibirdExplosionProbability: {}. use default value of false", e.what());
		}
	}
	logger::info("bUseVertibirdExplosionProbability: {}", bUseVertibirdExplosionProbability);

	value = GetINIOption("Settings", "fVertibirdExplosionProbability");
	if (!value.empty()) {
		try {
			fVertibirdExplosionProbability = std::stof(value);
		}
		catch (const std::exception& e) {
			logger::error("fVertibirdExplosionProbability: {}. use default value of 0.5", e.what());
		}

		if (fVertibirdExplosionProbability < 0.0f || fVertibirdExplosionProbability > 1.0f) {
			logger::error("fVertibirdExplosionProbability: value is out of range. use default value of 0.5");
			fVertibirdExplosionProbability = 0.5f;
		}
	}
	logger::info("fVertibirdExplosionProbability: {}", fVertibirdExplosionProbability);
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface * a_f4se, F4SE::PluginInfo * a_info) {
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path)
		return false;

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("Global"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%^%l%$] %v"s);

	logger::info("{} v{}"sv, Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical("unsupported runtime v{}"sv, ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface * a_f4se) {
	F4SE::Init(a_f4se);

	ReadINI();
	Hooks::Install(bPlayerVertibirdExplosion, bUseVertibirdExplosionProbability, fVertibirdExplosionProbability);

	return true;
}
