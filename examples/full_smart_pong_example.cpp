#include "smart_pong_ai.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

static float clampf(float v, float lo, float hi)
{
    return std::max(lo, std::min(hi, v));
}

static void reset_ball(smart_pong_game_state_t& g, int direction)
{
    g.ball.x = 0.5f;
    g.ball.y = 0.5f;
    g.ball.z = 0.5f;
    g.ball.vx = 0.65f * (float)direction;
    g.ball.vy = 0.18f;
    g.ball.vz = -0.07f;
    g.last_hit_dy = 0.0f;
    g.last_hit_dz = 0.0f;
}

static void move_paddle_toward(smart_pong_paddle_t& p, const smart_pong_intercept_t& target, float dt, float max_speed)
{
    const float dy = target.y - p.y;
    const float dz = target.z - p.z;

    const float step_y = clampf(dy, -max_speed * dt, max_speed * dt);
    const float step_z = clampf(dz, -max_speed * dt, max_speed * dt);

    p.vy = step_y / dt;
    p.vz = step_z / dt;
    p.y = clampf(p.y + step_y, 0.0f, 1.0f);
    p.z = clampf(p.z + step_z, 0.0f, 1.0f);
}

static bool paddle_contact(const smart_pong_game_state_t& g, bool right_side)
{
    const float paddle_x = right_side ? 1.0f : 0.0f;
    const float x_err = std::fabs(g.ball.x - paddle_x);
    const float y_err = std::fabs(g.ball.y - (right_side ? g.paddle_r.y : g.paddle_l.y));
    const float z_err = std::fabs(g.ball.z - (right_side ? g.paddle_r.z : g.paddle_l.z));
    return (x_err < 0.03f && y_err < 0.10f && z_err < 0.10f);
}

static void update_ball(smart_pong_game_state_t& g, float dt)
{
    g.ball.x += g.ball.vx * dt;
    g.ball.y += g.ball.vy * dt;
    g.ball.z += g.ball.vz * dt;

    if (g.ball.y < 0.0f) { g.ball.y = 0.0f; g.ball.vy = -g.ball.vy; }
    if (g.ball.y > 1.0f) { g.ball.y = 1.0f; g.ball.vy = -g.ball.vy; }

    if (g.ball.z < 0.0f) { g.ball.z = 0.0f; g.ball.vz = -g.ball.vz; }
    if (g.ball.z > 1.0f) { g.ball.z = 1.0f; g.ball.vz = -g.ball.vz; }
}

int main()
{
    smart_pong_game_state_t game{};
    game.paddle_l.y = 0.5f; game.paddle_l.z = 0.5f;
    game.paddle_r.y = 0.5f; game.paddle_r.z = 0.5f;
    reset_ball(game, 1);

    smart_pong_ai_config_t cfg{};
    smart_pong_ai_default_config(&cfg);

    smart_pong_learning_state_t learning_r{};
    smart_pong_ai_default_learning_state(&learning_r);

    smart_pong_learning_state_t learning_l{};
    smart_pong_ai_default_learning_state(&learning_l);

    const float dt = 0.016f;
    const float left_speed = 1.8f;
    const float right_speed = cfg.max_speed_units_per_sec;

    for (int frame = 0; frame < 2000; ++frame)
    {
        smart_pong_ai_result_t right_ai{};
        smart_pong_ai_result_t left_ai{};

        smart_pong_ai_step(&game, true, &cfg, &learning_r, &right_ai);
        smart_pong_ai_step(&game, false, &cfg, &learning_l, &left_ai);

        move_paddle_toward(game.paddle_r, right_ai.final_target, dt, right_speed);
        move_paddle_toward(game.paddle_l, left_ai.analytic, dt, left_speed);

        update_ball(game, dt);

        if (paddle_contact(game, true) && game.ball.vx > 0.0f)
        {
            game.last_hit_dy = game.ball.y - game.paddle_r.y;
            game.last_hit_dz = game.ball.z - game.paddle_r.z;
            smart_pong_learning_on_hit(&learning_r, game.last_hit_dy, game.last_hit_dz);
            game.ball.vx = -std::fabs(game.ball.vx);
            game.ball.vy += (-game.last_hit_dy) * 0.3f;
            game.ball.vz += (-game.last_hit_dz) * 0.3f;
        }

        if (paddle_contact(game, false) && game.ball.vx < 0.0f)
        {
            game.last_hit_dy = game.ball.y - game.paddle_l.y;
            game.last_hit_dz = game.ball.z - game.paddle_l.z;
            smart_pong_learning_on_hit(&learning_l, game.last_hit_dy, game.last_hit_dz);
            game.ball.vx = std::fabs(game.ball.vx);
            game.ball.vy += (-game.last_hit_dy) * 0.3f;
            game.ball.vz += (-game.last_hit_dz) * 0.3f;
        }

        if (game.ball.x < 0.0f)
        {
            game.score.right++;
            smart_pong_learning_on_miss(&learning_l, 0.12f, 0.12f);
            reset_ball(game, 1);
        }
        else if (game.ball.x > 1.0f)
        {
            game.score.left++;
            smart_pong_learning_on_miss(&learning_r, 0.12f, 0.12f);
            reset_ball(game, -1);
        }

        if ((frame % 60) == 0)
        {
            std::printf(
                "frame=%d  score=%d:%d  ball=(%.2f,%.2f,%.2f)  Rtarget=(%.2f,%.2f,%.2f) used_model=%d conf=%.2f\n",
                frame,
                game.score.left, game.score.right,
                game.ball.x, game.ball.y, game.ball.z,
                right_ai.final_target.y, right_ai.final_target.z, right_ai.final_target.t,
                (int)right_ai.used_model,
                right_ai.model_confidence
            );
        }
    }

    smart_pong_learning_blob_t blob{};
    if (smart_pong_learning_pack(&learning_r, 1u, &blob))
    {
        smart_pong_learning_state_t restored{};
        if (smart_pong_learning_unpack(&blob, 1u, &restored))
        {
            std::printf("persistence ok: hits=%u misses=%u skill=%.2f\n",
                        restored.hit_count, restored.miss_count, restored.skill);
        }
    }

    return 0;
}
