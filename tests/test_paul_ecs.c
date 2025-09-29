#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "test_common.h"

#define PAUL_NO_BLOCKS
#define PAUL_ECS_IMPLEMENTATION
#include "../paul_ecs.h"

typedef struct {
    int x, y;
} Position;

typedef struct {
    int dx, dy;
} Velocity;

static world_t *global_world;
static entity_t global_pos_comp;
static entity_t global_vel_comp;
static int query_counter = 0;

static void move_system(entity_t e) {
    Position *pos = (Position*)entity_get(global_world, e, global_pos_comp);
    Velocity *vel = (Velocity*)entity_get(global_world, e, global_vel_comp);
    if (pos && vel) {
        pos->x += vel->dx;
        pos->y += vel->dy;
    }
}

static void query_func(entity_t e) {
    query_counter++;
}

int main(void) {
    printf("1..29\n");

    world_t *world = ecs_world();
    global_world = world;
    expect_int("world_created", world != NULL, 1);

    entity_t pos_comp = ecs_component(world, sizeof(Position));
    global_pos_comp = pos_comp;
    expect_int("pos_component_created", entity_isa(world, pos_comp, ECS_COMPONENT), 1);

    entity_t vel_comp = ecs_component(world, sizeof(Velocity));
    global_vel_comp = vel_comp;
    expect_int("vel_component_created", entity_isa(world, vel_comp, ECS_COMPONENT), 1);

    entity_t ent1 = ecs_spawn(world);
    expect_int("entity1_spawned", entity_isa(world, ent1, ECS_ENTITY), 1);
    expect_int("entity1_valid", entity_isvalid(world, ent1), 1);

    entity_t ent2 = ecs_spawn(world);
    expect_int("entity2_spawned", entity_isa(world, ent2, ECS_ENTITY), 1);

    // Test entity_cmp
    expect_int("entity_cmp_same", entity_cmp(ent1, ent1), 1);
    expect_int("entity_cmp_different", entity_cmp(ent1, ent2), 0);

    // Test entity_isnil
    expect_int("entity_not_nil", entity_isnil(ent1), 0);
    expect_int("nil_entity_is_nil", entity_isnil(ecs_nil_entity), 1);

    // Give position to ent1
    Position *pos = (Position*)entity_give(world, ent1, pos_comp);
    expect_int("give_position", pos != NULL, 1);
    pos->x = 10;
    pos->y = 20;

    // Check has
    expect_int("has_position", entity_has(world, ent1, pos_comp), 1);
    expect_int("ent2_no_position", entity_has(world, ent2, pos_comp), 0);

    // Get position
    Position *got_pos = (Position*)entity_get(world, ent1, pos_comp);
    expect_int("get_position_x", got_pos->x, 10);
    expect_int("get_position_y", got_pos->y, 20);

    // Set position
    Position new_pos = {30, 40};
    entity_set(world, ent1, pos_comp, &new_pos);
    got_pos = (Position*)entity_get(world, ent1, pos_comp);
    expect_int("set_position_x", got_pos->x, 30);
    expect_int("set_position_y", got_pos->y, 40);

    // Give velocity to ent1
    Velocity *vel = (Velocity*)entity_give(world, ent1, vel_comp);
    expect_int("give_velocity", vel != NULL, 1);
    vel->dx = 1;
    vel->dy = 2;

    // Give position to ent2
    pos = (Position*)entity_give(world, ent2, pos_comp);
    pos->x = 100;
    pos->y = 200;

    // Test find entities with position
    int count;
    entity_t *found = ecs_find(world, NULL, &count, 1, pos_comp);
    expect_int("find_with_position_count", count, 2);

    free(found);

    // Test find with velocity
    found = ecs_find(world, NULL, &count, 1, vel_comp);
    expect_int("find_with_velocity_count", count, 1);
    expect_int("find_velocity_entity", entity_cmp(found[0], ent1), 1);

    free(found);

    // Test query
    query_counter = 0;
    ecs_query(world, query_func, NULL, 1, pos_comp);
    expect_int("query_position_count", query_counter, 2);

    // Remove velocity from ent1
    entity_remove(world, ent1, vel_comp);
    expect_int("remove_velocity", entity_has(world, ent1, vel_comp), 0);

    // Delete ent2
    ecs_delete(world, ent2);
    expect_int("delete_ent2", entity_isvalid(world, ent2), 0);

    // ent1 should still be valid
    expect_int("ent1_still_valid", entity_isvalid(world, ent1), 1);

    // Test system creation
    entity_t sys = ecs_system(world, move_system, 2, pos_comp, vel_comp);
    expect_int("system_created", entity_isa(world, sys, ECS_SYSTEM), 1);

    // Give velocity back to ent1 for system test
    vel = (Velocity*)entity_give(world, ent1, vel_comp);
    vel->dx = 5;
    vel->dy = 10;

    // Step world - should move ent1
    ecs_step(world);
    got_pos = (Position*)entity_get(world, ent1, pos_comp);
    expect_int("step_change_x", got_pos->x, 35);  // 30 + 5
    expect_int("step_change_y", got_pos->y, 50);  // 40 + 10

    // Destroy world
    ecs_world_destroy(&world);
    expect_int("world_destroyed", world == NULL, 1);

    if (test_failures() == 0) {
        fprintf(stderr, "# all ecs tests passed\n");
        return 0;
    }
    fprintf(stderr, "# %d ecs test(s) failed\n", test_failures());
    return 1;
}