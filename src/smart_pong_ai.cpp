#include "smart_pong_ai.h"

#include <math.h>
#include <string.h>

static float sp_clampf(float v, float lo, float hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

static float sp_absf(float x)
{
    return (x < 0.0f) ? -x : x;
}

static float sp_hash_noise(float seed)
{
    float x = sinf(seed * 12.9898f) * 43758.5453f;
    x = x - floorf(x);
    return (x * 2.0f) - 1.0f;
}

void smart_pong_ai_default_config(smart_pong_ai_config_t* cfg)
{
    if (!cfg) return;
    cfg->confidence_threshold = 0.25f;
    cfg->max_blend_error_y = 0.35f;
    cfg->max_blend_error_z = 0.35f;
    cfg->max_speed_units_per_sec = 2.2f;
    cfg->y_min = 0.0f;
    cfg->y_max = 1.0f;
    cfg->z_min = 0.0f;
    cfg->z_max = 1.0f;
    cfg->t_min = 0.01f;
    cfg->t_max = 2.00f;
    cfg->noise_seed = 0.5f;
}

void smart_pong_ai_default_learning_state(smart_pong_learning_state_t* st)
{
    if (!st) return;
    memset(st, 0, sizeof(*st));
    st->skill = 0.5f;
}

void smart_pong_ai_build_features(const smart_pong_game_state_t* g, float f[SMART_PONG_FEATURE_COUNT])
{
    if (!g || !f) return;

    memset(f, 0, SMART_PONG_FEATURE_COUNT * sizeof(float));

    f[0] = g->ball.x;
    f[1] = g->ball.y;
    f[2] = g->ball.z;
    f[3] = g->ball.vx;
    f[4] = g->ball.vy;
    f[5] = g->ball.vz;

    f[6] = g->paddle_l.y;
    f[7] = g->paddle_l.z;
    f[8] = g->paddle_l.vy;
    f[9] = g->paddle_l.vz;

    f[10] = g->paddle_r.y;
    f[11] = g->paddle_r.z;

    int32_t sd = g->score.left - g->score.right;
    if (sd > 20) sd = 20;
    if (sd < -20) sd = -20;
    f[12] = (float)sd * (1.0f / 20.0f);

    f[13] = g->last_hit_dy;
    f[14] = g->last_hit_dz;
    f[15] = 1.0f;
}

void smart_pong_ai_mirror_features_for_left(float f[SMART_PONG_FEATURE_COUNT])
{
    if (!f) return;

    f[0] = 1.0f - f[0];
    f[3] = -f[3];

    const float l_y = f[6];
    const float l_z = f[7];

    f[6] = f[10];
    f[7] = f[11];
    f[8] = 0.0f;
    f[9] = 0.0f;
    f[10] = l_y;
    f[11] = l_z;
}

bool smart_pong_ai_ball_moving_toward_paddle(const smart_pong_game_state_t* g, bool right_side)
{
    if (!g) return false;
    return right_side ? (g->ball.vx > 0.0f) : (g->ball.vx < 0.0f);
}

bool smart_pong_ai_analytic_intercept(const smart_pong_game_state_t* g, bool right_side, const smart_pong_ai_config_t* cfg, smart_pong_intercept_t* out)
{
    if (!g || !cfg || !out) return false;

    const float target_x = right_side ? 1.0f : 0.0f;
    const float dx = target_x - g->ball.x;
    if (fabsf(g->ball.vx) < 1e-6f) return false;

    const float t = dx / g->ball.vx;
    if (t < cfg->t_min || t > cfg->t_max) return false;

    out->t = t;
    out->y = sp_clampf(g->ball.y + g->ball.vy * t, cfg->y_min, cfg->y_max);
    out->z = sp_clampf(g->ball.z + g->ball.vz * t, cfg->z_min, cfg->z_max);
    return true;
}

static float sp_compute_confidence(
    const smart_pong_intercept_t* analytic,
    const smart_pong_intercept_t* model,
    const smart_pong_ai_config_t* cfg,
    float backend_confidence)
{
    const float ey = sp_absf(model->y - analytic->y) / ((cfg->max_blend_error_y > 1e-6f) ? cfg->max_blend_error_y : 1.0f);
    const float ez = sp_absf(model->z - analytic->z) / ((cfg->max_blend_error_z > 1e-6f) ? cfg->max_blend_error_z : 1.0f);

    float distance_penalty = 0.5f * (ey + ez);
    if (distance_penalty > 1.0f) distance_penalty = 1.0f;

    float confidence = backend_confidence * (1.0f - distance_penalty);
    if (confidence < 0.0f) confidence = 0.0f;
    if (confidence > 1.0f) confidence = 1.0f;
    return confidence;
}

static smart_pong_intercept_t sp_blend_targets(
    const smart_pong_intercept_t* analytic,
    const smart_pong_intercept_t* model,
    float confidence)
{
    smart_pong_intercept_t out{};
    const float a = confidence;
    const float b = 1.0f - a;
    out.y = (analytic->y * b) + (model->y * a);
    out.z = (analytic->z * b) + (model->z * a);
    out.t = (analytic->t * b) + (model->t * a);
    return out;
}

static void sp_apply_learning_adjustments(
    smart_pong_intercept_t* t,
    const smart_pong_learning_state_t* learning,
    const smart_pong_ai_config_t* cfg)
{
    if (!t || !learning || !cfg) return;

    t->y += learning->lead_y + learning->style_bias_y;
    t->z += learning->lead_z + learning->style_bias_z;

    const float n = sp_hash_noise(cfg->noise_seed + (float)(learning->hit_count + learning->miss_count + 1U));
    t->y += n * learning->noise_gain * 0.02f;
    t->z -= n * learning->noise_gain * 0.02f;

    t->y = sp_clampf(t->y, cfg->y_min, cfg->y_max);
    t->z = sp_clampf(t->z, cfg->z_min, cfg->z_max);
    t->t = sp_clampf(t->t, cfg->t_min, cfg->t_max);
}

bool smart_pong_ai_step(
    const smart_pong_game_state_t* g,
    bool right_side,
    const smart_pong_ai_config_t* cfg,
    const smart_pong_learning_state_t* learning,
    smart_pong_ai_result_t* out)
{
    if (!g || !cfg || !learning || !out) return false;
    memset(out, 0, sizeof(*out));

    if (!smart_pong_ai_analytic_intercept(g, right_side, cfg, &out->analytic))
        return false;

    out->final_target = out->analytic;

    if (!smart_pong_ai_ball_moving_toward_paddle(g, right_side))
    {
        sp_apply_learning_adjustments(&out->final_target, learning, cfg);
        return true;
    }

    float features[SMART_PONG_FEATURE_COUNT];
    smart_pong_ai_build_features(g, features);
    if (!right_side)
        smart_pong_ai_mirror_features_for_left(features);

    smart_pong_prediction_t pred{};
    float backend_confidence = 0.0f;
    out->backend_ok = smart_pong_model_predict(features, &pred, &backend_confidence);

    if (!out->backend_ok)
    {
        sp_apply_learning_adjustments(&out->final_target, learning, cfg);
        return true;
    }

    out->model.y = sp_clampf(pred.y_hit, cfg->y_min, cfg->y_max);
    out->model.z = sp_clampf(pred.z_hit, cfg->z_min, cfg->z_max);
    out->model.t = sp_clampf(pred.t_hit, cfg->t_min, cfg->t_max);

    out->model_confidence = sp_compute_confidence(&out->analytic, &out->model, cfg, backend_confidence);

    if (out->model_confidence >= cfg->confidence_threshold)
    {
        out->final_target = sp_blend_targets(&out->analytic, &out->model, out->model_confidence);
        out->used_model = true;
    }

    sp_apply_learning_adjustments(&out->final_target, learning, cfg);
    return true;
}

void smart_pong_learning_on_hit(smart_pong_learning_state_t* st, float error_y, float error_z)
{
    if (!st) return;
    st->hit_count++;
    st->lead_y += (-error_y) * 0.02f;
    st->lead_z += (-error_z) * 0.02f;
    st->style_bias_y += (-error_y) * 0.01f;
    st->style_bias_z += (-error_z) * 0.01f;
    st->skill += 0.01f;
    if (st->skill > 1.0f) st->skill = 1.0f;
    if (st->noise_gain > 0.0f) st->noise_gain *= 0.99f;
}

void smart_pong_learning_on_miss(smart_pong_learning_state_t* st, float error_y, float error_z)
{
    if (!st) return;
    st->miss_count++;
    st->lead_y += error_y * 0.03f;
    st->lead_z += error_z * 0.03f;
    st->noise_gain += 0.01f;
    st->skill -= 0.01f;
    if (st->skill < 0.0f) st->skill = 0.0f;
}
