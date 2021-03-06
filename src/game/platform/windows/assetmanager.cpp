#include <game/core.h>

#if (CURRENT_PLATFORM == PLATFORM_WINDOWS)

#define STB_IMAGE_IMPLEMENTATION
#include "assetmanager.h"

namespace punkyoi::platform::windows {

    PlatformAssetManager::PlatformAssetManager() {
    }

    PlatformAssetManager::~PlatformAssetManager() {
        for (auto& it : m_assetMap) {
            delete it.second;
        }
    }

    void PlatformAssetManager::loadAssetMap(const std::string& filename) {
        const tao::json::value json = tao::json::from_file(filename);
        const tao::json::value path = json.at("path");
        m_root = path.get_string();
        auto& jsonMap = json.get_object();
        for (auto& it : jsonMap) {
            if (it.first != "path") {
                processJson(it.first, it.second);
            }
        }
    }

    ::punkyoi_api::IAsset& PlatformAssetManager::getAsset(const std::string& key) {
        if (!m_assetMap.count(key)) {
            throw ::punkyoi::common::exceptions::NoAssetException(key);
        }
        return *(m_assetMap[key]);
    }

    ::punkyoi::common::ImageAsset& PlatformAssetManager::getImage(const std::string& key) {
        return dynamic_cast<::punkyoi::common::ImageAsset&>(getAsset(key));
    }

    ::punkyoi::common::SoundAsset& PlatformAssetManager::getSound(const std::string& key) {
        return dynamic_cast<::punkyoi::common::SoundAsset&>(getAsset(key));
    }

    void PlatformAssetManager::processJson(const std::string& prefix, const tao::json::value& json) {
        std::string file;
        std::string type;
        try {
            file = json.at("file").get_string();
            type = json.at("type").get_string();
        }
        catch (...) {
            try {
                auto& jsonMap = json.get_object();
            }
            catch (std::exception& ex) {
                throw ::punkyoi::common::exceptions::BadAssetFormatException(prefix + ": " + ex.what());
            }
            for (auto& it : json.get_object()) {
                processJson(prefix + "." + it.first, it.second);
            }
            return;
        }
        
        log::console() << "Loading asset <" << prefix << "> as " << type << log::endl;

        if (m_assetMap.count(prefix)) {
            throw ::punkyoi::common::exceptions::DuplicateAssetException(prefix);
        }

        if (type == "sprite") {
            m_assetMap[prefix] = SpriteLoader::load(m_root + file + ".png");
        }
        if (type == "animation") {
            std::string source = m_root + file;
            int numFrames = 1;
            try {
                numFrames = json.at("frames").get_unsigned();
            }
            catch (std::exception& ex) {
                throw ::punkyoi::common::exceptions::BadAssetFormatException(prefix + ": " + ex.what());
            }
            try {
                for (int i = 0; i < numFrames; ++i) {
                    source = m_root + file + "/" + std::to_string(i) + ".png";
                    m_assetMap[prefix + std::to_string(i)] = SpriteLoader::load(source);
                }
            }
            catch (std::exception& ex) {
                throw ::punkyoi::common::exceptions::MissingAssetFileException(source);
            }
        }
        if (type == "sound") {
            m_assetMap[prefix] = SoundLoader::load(m_root + file + ".wav");
        }
    }

    ::punkyoi::common::ImageAsset* SpriteLoader::load(const std::string& source) {
        int width, height, channels;
        uint8_t* data = stbi_load(source.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (data == nullptr) {
            throw ::punkyoi::common::exceptions::MissingAssetFileException(source);
        }
        ::punkyoi::common::ImageAsset* asset = new ::punkyoi::common::ImageAsset(width, height, data);
        return asset;
    }

    ::punkyoi::common::SoundAsset* SoundLoader::load(const std::string& source) {
        return new ::punkyoi::common::SoundAsset(source);
    }
}

#endif