#include "scene.h"

namespace punkyoi::common {

    Scene::Scene() {
        m_eventBus = std::make_shared<punkyoi_api::events::EventBus>();
        m_eventBus->subscribeEventListener(this);
        m_isActive = true;
    }

    Scene::~Scene() {
        unload();
    }

    void Scene::close() {
        m_isActive = false;
    }

    bool Scene::isActive() {
        return m_isActive;
    }

    std::shared_ptr<punkyoi_api::events::EventBus>& Scene::getEventBus() {
        return m_eventBus;
    }

    void Scene::load(World&) {
        m_isActive = true;
    }

    void Scene::unload() {
        for (auto& entity : m_entities) {
            entity->onDeath();
            m_eventBus->unsubscribeEventListener(entity);
        }
    }

    void Scene::addEntity(const object<Entity>& entity) {
        m_entities.push_back(entity);
        m_eventBus->subscribeEventListener(entity);
    }

    void Scene::onEvent(punkyoi::events::TickEvent& event) {
        for (object<Entity>& entity : m_entities) {
            if (entity->isDead()) {
                entity->onDeath();
                m_eventBus->unsubscribeEventListener(entity);
            }
        }
        m_entities.remove_if([](object<Entity>& entity) { return entity->isDead(); });
    }
}