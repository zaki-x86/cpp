// 03_ecs_world.cpp
// Minimal ECS (Entity Component System) world.
//
// Design:
//   Entity    — uint32_t ID
//   World     — typed component storage via template members, view queries
//   Systems   — free functions operating on World component views
//
// Component storage uses std::unordered_map<Entity, T> per component type.
// This is correct and readable for a tutorial; production ECS (EnTT, Flecs)
// uses sparse-sets + dense arrays for cache-optimal bulk iteration.
//
// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t16 03_ecs_world.cpp

#include <any>
#include <cstdint>
#include <iostream>
#include <typeindex>
#include <unordered_map>
#include <vector>

// ---- Entity type ---------------------------------------------------------

using Entity = uint32_t;

// ---- Component types -----------------------------------------------------

struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health   { int hp; };

// ---- World ---------------------------------------------------------------

class World {
    // One unordered_map<Entity, T> per component type, keyed by type_index.
    // std::any holds the map — we cast back to the concrete map type.
    std::unordered_map<std::type_index, std::any> storage_;
    Entity next_id_{0};

    // Return the component map for type T, creating it if absent.
    template<typename T>
    std::unordered_map<Entity, T>& store() {
        auto key = std::type_index(typeid(T));
        auto it = storage_.find(key);
        if (it == storage_.end()) {
            storage_.emplace(key, std::unordered_map<Entity, T>{});
            it = storage_.find(key);
        }
        return std::any_cast<std::unordered_map<Entity, T>&>(it->second);
    }

    template<typename T>
    const std::unordered_map<Entity, T>& store() const {
        auto key = std::type_index(typeid(T));
        return std::any_cast<const std::unordered_map<Entity, T>&>(
            storage_.at(key));
    }

public:
    // Create a new entity and return its ID.
    Entity create_entity() { return next_id_++; }

    // Add a component to an entity.
    template<typename T>
    void add_component(Entity e, T component) {
        store<T>().emplace(e, std::move(component));
    }

    // Get a reference to a component (entity must have the component).
    template<typename T>
    T& get_component(Entity e) {
        return store<T>().at(e);
    }

    template<typename T>
    const T& get_component(Entity e) const {
        return store<T>().at(e);
    }

    // Returns true if entity has component T.
    template<typename T>
    bool has_component(Entity e) const {
        auto key = std::type_index(typeid(T));
        auto it = storage_.find(key);
        if (it == storage_.end()) return false;
        const auto& m = std::any_cast<const std::unordered_map<Entity, T>&>(it->second);
        return m.count(e) > 0;
    }

    // Return all entities that have ALL of the listed component types.
    template<typename First, typename... Rest>
    std::vector<Entity> view() const {
        // Gather candidates from the First component's map.
        std::vector<Entity> result;
        auto key = std::type_index(typeid(First));
        auto it = storage_.find(key);
        if (it == storage_.end()) return result;

        const auto& first_map =
            std::any_cast<const std::unordered_map<Entity, First>&>(it->second);

        for (const auto& [e, _] : first_map) {
            // Check that entity also has all Rest components.
            if ((has_component<Rest>(e) && ...)) {
                result.push_back(e);
            }
        }
        return result;
    }
};

// ---- Systems (free functions) --------------------------------------------

void physics_system(World& world, float dt) {
    for (Entity e : world.view<Position, Velocity>()) {
        auto& pos = world.get_component<Position>(e);
        const auto& vel = world.get_component<Velocity>(e);
        pos.x += vel.dx * dt;
        pos.y += vel.dy * dt;
    }
}

void print_positions(const World& world) {
    for (Entity e : world.view<Position>()) {
        const auto& pos = world.get_component<Position>(e);
        std::cout << "  entity " << e
                  << ": pos=(" << pos.x << ", " << pos.y << ")\n";
    }
}

// ---- main ----------------------------------------------------------------

int main() {
    World world;

    // Create 3 entities.
    Entity e0 = world.create_entity();   // 0
    Entity e1 = world.create_entity();   // 1
    Entity e2 = world.create_entity();   // 2

    // Give all 3 entities Position and Velocity.
    world.add_component(e0, Position{0.0f, 0.0f});
    world.add_component(e1, Position{10.0f, 0.0f});
    world.add_component(e2, Position{0.0f, 5.0f});

    world.add_component(e0, Velocity{1.0f, 0.0f});
    world.add_component(e1, Velocity{0.0f, 2.0f});
    world.add_component(e2, Velocity{1.0f, 1.0f});

    // Only entity 0 has Health.
    world.add_component(e0, Health{100});

    // Show query results.
    std::cout << "view<Position,Velocity> count: "
              << world.view<Position, Velocity>().size() << " (expect 3)\n";
    std::cout << "view<Health> count: "
              << world.view<Health>().size() << " (expect 1)\n\n";

    // Run 3 physics ticks.
    const float dt = 1.0f;
    for (int tick = 1; tick <= 3; ++tick) {
        physics_system(world, dt);
        std::cout << "After tick " << tick << ":\n";
        print_positions(world);
        std::cout << "\n";
    }

    // Verify health is on e0 only.
    std::cout << "entity 0 hp: "
              << world.get_component<Health>(e0).hp << "\n";

    return 0;
}
