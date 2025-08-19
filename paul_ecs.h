/* paul/paul_ecs.h -- https://github.com/takeiteasy/paul

 Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef PAUL_ECS_HEAD
#define PAUL_ECS_HEAD
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#define PAUL_NO_BLOCKS
#endif
#ifndef PAUL_NO_BLOCKS
#if !__has_extension(blocks)
#define PAUL_NO_BLOCKS
#else
#if __has_include(<Block.h>)
#include <Block.h>
#endif
#endif
#endif

typedef union entity {
    struct {
        uint32_t id;
        uint16_t version;
        uint8_t alive;
        uint8_t type;
    };
    uint64_t value;
} entity_t;

extern const uint64_t ecs_nil;
extern const entity_t ecs_nil_entity;

typedef struct world world_t;
#ifndef PAUL_NO_BLOCKS
typedef void(^system_t)(entity_t);
typedef int(^filter_system_t)(entity_t);
#else
typedef void(*system_t)(entity_t);
typedef int(*filter_system_t)(entity_t);
#endif

enum {
    ECS_ENTITY,
    ECS_COMPONENT,
    ECS_SYSTEM
};

world_t* ecs_world(void);
void ecs_world_destroy(world_t **world);

entity_t ecs_spawn(world_t *world);
entity_t ecs_component(world_t *world, size_t size_of_component);
entity_t ecs_system(world_t *world, system_t fn, int component_count, ...);
void ecs_delete(world_t *world, entity_t e);

int entity_isvalid(world_t *world, entity_t e);
int entity_isa(world_t *world, entity_t e, int type);
int entity_cmp(entity_t a, entity_t b);
int entity_isnil(entity_t e);
void* entity_give(world_t *world, entity_t e, entity_t c);
void entity_remove(world_t *world, entity_t e, entity_t c);
int entity_has(world_t *world, entity_t e, entity_t c);
void* entity_get(world_t *world, entity_t e, entity_t c);
void entity_set(world_t *world, entity_t e, entity_t c, void *data);

void ecs_step(world_t *world);
entity_t* ecs_find(world_t *world, filter_system_t filter, int *result_count, int component_count, ...);
void ecs_query(world_t *world, system_t fn, filter_system_t filter, int component_count, ...);

#ifdef __cplusplus
}
#endif
#endif // PAUL_ECS_HEAD

#ifdef PAUL_ECS_IMPLEMENTATION
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

const uint64_t ecs_nil = 0xFFFFFFFFull;
const entity_t ecs_nil_entity = {.id=ecs_nil};

typedef struct sparse {
    entity_t *sparse;
    entity_t *dense;
    size_t sizeOfSparse;
    size_t sizeOfDense;
} sparse_t;

static sparse_t* sparse(void) {
    sparse_t *result = (sparse_t*)malloc(sizeof(sparse_t));
    memset(result, 0, sizeof(sparse_t));
    return result;
}

static void sparse_destroy(sparse_t **sparse) {
    sparse_t *s = *sparse;
    if (s->sparse)
        free(s->sparse);
    if (s->dense)
        free(s->dense);
    free(s);
    *sparse = NULL;
}

static int sparse_has(sparse_t *sparse, entity_t e) {
    return e.id < sparse->sizeOfSparse && !entity_isnil(sparse->sparse[e.id]);
}

static void sparse_emplace(sparse_t *sparse, entity_t e) {
    if (e.id > sparse->sizeOfSparse) {
        size_t size = e.id + 1;
        sparse->sparse = (entity_t*)realloc(sparse->sparse, size * sizeof * sparse->sparse);
        for (size_t i = sparse->sizeOfSparse; i < size; i++)
            sparse->sparse[i] = ecs_nil_entity;
        sparse->sizeOfSparse = size;
    }
    sparse->sparse[e.id] = (entity_t){.id=(uint32_t)sparse->sizeOfDense};
    sparse->dense = (entity_t*)realloc(sparse->dense, (sparse->sizeOfDense + 1) * sizeof * sparse->dense);
    sparse->dense[sparse->sizeOfDense++] = e;
}

static size_t sparse_at(sparse_t *sparse, entity_t e) {
    return sparse->sparse[e.id].id;
}

static size_t sparse_remove(sparse_t *sparse, entity_t e) {
    assert(sparse_has(sparse, e));
    uint32_t pos = sparse_at(sparse, e);
    entity_t other = sparse->dense[sparse->sizeOfDense-1];
    sparse->sparse[other.id] = (entity_t){.id=pos};
    sparse->dense[pos] = other;
    sparse->sparse[e.id] = ecs_nil_entity;
    sparse->dense = (entity_t*)realloc(sparse->dense, --sparse->sizeOfDense * sizeof * sparse->dense);
    return pos;
}

typedef struct storage {
    entity_t componentId;
    void *data;
    size_t sizeOfData;
    size_t sizeOfComponent;
    sparse_t *sparse;
} storage_t;

static storage_t* storage(entity_t e, size_t sz) {
    storage_t *storage = (storage_t*)malloc(sizeof(storage_t));
    *storage = (storage_t) {
        .componentId = e,
        .sizeOfComponent = sz,
        .sizeOfData = 0,
        .data = NULL,
        .sparse = sparse()
    };
    return storage;
}

static void storage_destroy(storage_t **strg) {
    storage_t* s = *strg;
    if (s->sparse)
        sparse_destroy(&s->sparse);
    if (s->data)
        free(s->data);
    free(s);
    *strg = NULL;
}

static int storage_has(storage_t *strg, entity_t e) {
    return sparse_has(strg->sparse, e);
}

static void* storage_emplace(storage_t *strg, entity_t e) {
    strg->data = (void*)realloc(strg->data, ++strg->sizeOfData * sizeof(char) * strg->sizeOfComponent);
    void *result = &((char*)strg->data)[(strg->sizeOfData - 1) * sizeof(char) * strg->sizeOfComponent];
    sparse_emplace(strg->sparse, e);
    return result;
}

static void storage_remove(storage_t *strg, entity_t e) {
    size_t pos = sparse_remove(strg->sparse, e);
    memmove(&((char*)strg->data)[pos * sizeof(char) * strg->sizeOfComponent],
            &((char*)strg->data)[(strg->sizeOfData - 1) * sizeof(char) * strg->sizeOfComponent],
            strg->sizeOfComponent);
    strg->data = (void*)realloc(strg->data, --strg->sizeOfData * sizeof(char) * strg->sizeOfComponent);
}

static void* storage_get(storage_t *strg, entity_t e) {
    uint32_t pos = sparse_at(strg->sparse, e);
    return &((char*)strg->data)[pos * sizeof(char) * strg->sizeOfComponent];
}

struct world {
    storage_t **storages;
    storage_t *systems;
    size_t sizeOfStorages;
    entity_t *entities;
    size_t sizeOfEntities;
    uint32_t *recyclable;
    size_t sizeOfRecyclable;
};

typedef struct {
    uint32_t id;
    uint32_t component_count;
    entity_t *components;
    system_t callback;
} system_component_t;

static entity_t make_entity(world_t *world, uint8_t type) {
    if (world->sizeOfRecyclable) {
        uint32_t id = world->recyclable[world->sizeOfRecyclable-1];
        entity_t old = world->entities[id];
        entity_t new = (entity_t) {
            .id = old.id,
            .version = old.version,
            .alive = 1,
            .type = type
        };
        world->entities[id] = new;
        world->recyclable = (entity_t*)realloc(world->recyclable, --world->sizeOfRecyclable * sizeof(uint32_t));
        return new;
    } else {
        world->entities = (entity_t*)realloc(world->entities, ++world->sizeOfEntities * sizeof(entity_t));
        entity_t e = (entity_t) {
            .id = (uint32_t)world->sizeOfEntities - 1,
            .version = 0,
            .alive = 1,
            .type = type
        };
        world->entities[e.id] = e;
        return e;
    }
}

world_t* ecs_world(void) {
    world_t *result = (world_t*)malloc(sizeof(world_t));
    memset(result, 0, sizeof(world_t));
    entity_t e = make_entity(result, ECS_COMPONENT); // doesn't matter will always be first entity
    result->systems = storage(e, sizeof(system_component_t));
    return result;
}

void ecs_world_destroy(world_t **_world) {
    world_t *world = *_world;
    if (world->storages) {
        for (int i = 0; i < world->sizeOfStorages; i++)
            storage_destroy(&world->storages[i]);
        free(world->storages);
    }
    if (world->entities)
        free(world->entities);
    if (world->recyclable)
        free(world->recyclable);
    if (world->systems)
        storage_destroy(&world->systems);
    free(world);
    *_world = NULL;
}

static storage_t* find_storage(world_t *world, entity_t e) {
    for (int i = 0; i < world->sizeOfStorages; i++)
        if (entity_cmp(e, world->storages[i]->componentId))
            return world->storages[i];
    return NULL;
}

entity_t ecs_spawn(world_t *world) {
    return make_entity(world, ECS_ENTITY);
}

entity_t ecs_component(world_t *world, size_t sizeOfComponent) {
    entity_t e = make_entity(world, ECS_COMPONENT);
    storage_t *strg = find_storage(world, e);
    if (strg)
        return e;
    strg = storage(e, sizeOfComponent);
    world->storages = (storage_t*)realloc(world->storages, (world->sizeOfStorages + 1) * sizeof * world->storages);
    world->storages[world->sizeOfStorages++] = strg;
    return e;
}

static entity_t* vargs_components(world_t *world, int n, va_list args) {
    entity_t *result = (entity_t*)malloc(n * sizeof(entity_t));
    for (int i = 0; i < n; i++) {
        entity_t component = va_arg(args, entity_t);
        if (!entity_isa(world, component, ECS_COMPONENT)) {
            free(result);
            result = NULL;
            goto BAIL;
        }
        result[i] = component;
    }
BAIL:
    va_end(args);
    return result;
}

entity_t ecs_system(world_t *world, system_t system, int n, ...) {
    entity_t e = make_entity(world, ECS_SYSTEM);
    va_list args;
    va_start(args, n);
    system_component_t *system_data = (system_component_t*)malloc(sizeof(system_component_t));
    system_data->id = e.id;
    system_data->component_count = n;
    system_data->components = vargs_components(world, n, args);
    system_data->callback = system;
    void *ptr = storage_emplace(world->systems, e);
    memcpy(ptr, system_data, sizeof(system_component_t));
    return e;
}

void ecs_delete(world_t *world, entity_t e) {
    switch (e.type) {
        case ECS_ENTITY:
            for (size_t i = world->sizeOfStorages; i; --i)
                if (world->storages[i - 1] && sparse_has(world->storages[i - 1]->sparse, e))
                    storage_remove(world->storages[i - 1], e);
            world->entities[e.id] = (entity_t) {
                .id = e.id,
                .version = e.version + 1,
                .alive = 0,
                .type = 255
            };
            world->recyclable = (entity_t*)realloc(world->recyclable, ++world->sizeOfRecyclable * sizeof(uint32_t));
            world->recyclable[world->sizeOfRecyclable-1] = e.id;
            break;
        case ECS_COMPONENT:
            // TODO: !
            break;
        case ECS_SYSTEM:
            // TODO: !
            break;
    }
}

int entity_isvalid(world_t *world, entity_t e) {
    return world->sizeOfEntities > e.id && entity_cmp(world->entities[e.id], e);
}

int entity_isa(world_t *world, entity_t e, int type) {
    return entity_isvalid(world, e) && e.type == type;
}

int entity_cmp(entity_t a, entity_t b) {
    return a.value == b.value;
}

int entity_isnil(entity_t e) {
    return e.id == ecs_nil;
}

static storage_t* find_entity_storage(world_t *world, entity_t e, entity_t c) {
    assert(entity_isa(world, e, ECS_ENTITY));
    assert(entity_isa(world, c, ECS_COMPONENT));
    storage_t *strg = find_storage(world, c);
    assert(strg);
    return strg;
}

void* entity_give(world_t *world, entity_t e, entity_t c) {
    return storage_emplace(find_entity_storage(world, e, c), e);
}

void entity_remove(world_t *world, entity_t e, entity_t c) {
    storage_t *strg = find_entity_storage(world, e, c);
    assert(storage_has(strg, e));
    storage_remove(strg, e);
}

void* entity_get(world_t *world, entity_t e, entity_t c) {
    storage_t *strg = find_entity_storage(world, e, c);
    return storage_has(strg, e) ? storage_get(strg, e) : NULL;
}

void entity_set(world_t *world, entity_t e, entity_t c, void *data) {
    storage_t *strg = find_entity_storage(world, e, c);
    memcpy(storage_has(strg, e) ? storage_get(strg, e) : storage_emplace(strg, e),
           data,
           strg->sizeOfComponent);
}

int entity_has(world_t *world, entity_t e, entity_t c) {
    storage_t *strg = find_storage(world, c);
    if (!strg)
        return 0;
    return storage_has(strg, e);
}

entity_t* ecs_find(world_t *world, filter_system_t filter, int *result_count, int n, ...) {
    va_list args;
    va_start(args, n);
    entity_t *components = vargs_components(world, n, args);
    entity_t *result = NULL;
    int count = 0;
    for (int i = 0; i < world->sizeOfEntities; i++) {
        int match = 1;
        for (int j = 0; j < n; j++) {
            storage_t *strg = find_storage(world, components[j]);
            assert(strg);
            if (!storage_has(strg, world->entities[i])) {
                match = 0;
                break;
            }
        }
        if (match && !(filter && !filter(world->entities[i]))) {
            result = (entity_t*)realloc(result, count + 1 * sizeof(entity_t));
            result[count++] = world->entities[i];
        }
    }
    if (result_count)
        *result_count = count;
    return result;
}

void ecs_query(world_t *world, system_t fn, filter_system_t filter, int n, ...) {
    va_list args;
    va_start(args, n);
    entity_t *components = vargs_components(world, n, args);

    for (int i = 0; i < world->sizeOfEntities; i++) {
        int match = 1;
        for (int j = 0; j < n; j++) {
            storage_t *strg = find_storage(world, components[j]);
            assert(strg);
            if (!storage_has(strg, world->entities[i])) {
                match = 0;
                break;
            }
        }
        if (match && !(filter && !filter(world->entities[i])))
            fn(world->entities[i]);
    }
}

void ecs_step(world_t *world) {
    for (int i = 0; i < world->systems->sparse->sizeOfDense; i++) {
        system_component_t *system_data = storage_get(world->systems, world->systems->sparse->dense[i]);
        entity_t system_entity = world->entities[system_data->id];
        assert(entity_isa(world, system_entity, ECS_SYSTEM));
        if (!system_entity.alive)
            continue;

        for (int j = 0; j < world->sizeOfEntities; j++) {
            int match = 1;
            for (int k = 0; k < system_data->component_count; k++) {
                storage_t *strg = find_storage(world, system_data->components[k]);
                assert(strg);
                if (!storage_has(strg, world->entities[j])) {
                    match = 0;
                    break;
                }
            }
            if (match)
                system_data->callback(world->entities[j]);
        }
    }
}
#endif

