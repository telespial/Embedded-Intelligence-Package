#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SMART_PONG_FEATURE_COUNT 16U
#define SMART_PONG_PERSIST_MAGIC 0x45495031u /* "EIP1" */

typedef struct
{
    float x;
    float y;
    float z;
    float vx;
    float vy;
    float vz;
} smart_pong_ball_t;

typedef struct
{
    float y;
    float z;
    float vy;
    float vz;
} smart_pong_paddle_t;

typedef struct
{
    int32_t left;
    int32_t right;
} smart_pong_score_t;

typedef struct
{
    smart_pong_ball_t ball;
    smart_pong_paddle_t paddle_l;
    smart_pong_paddle_t paddle_r;
    smart_pong_score_t score;
    float last_hit_dy;
    float last_hit_dz;
} smart_pong_game_state_t;

typedef struct
{
    float y_hit;
    float z_hit;
    float t_hit;
} smart_pong_prediction_t;

typedef struct
{
    float y;
    float z;
    float t;
} smart_pong_intercept_t;

typedef struct
{
    float lead_y;
    float lead_z;
    float style_bias_y;
    float style_bias_z;
    float noise_gain;
    float skill;
    uint32_t hit_count;
    uint32_t miss_count;
} smart_pong_learning_state_t;

typedef struct
{
    float confidence_threshold;
    float max_blend_error_y;
    float max_blend_error_z;
    float max_speed_units_per_sec;
    float y_min;
    float y_max;
    float z_min;
    float z_max;
    float t_min;
    float t_max;
    float noise_seed;
} smart_pong_ai_config_t;

typedef struct
{
    bool used_model;
    bool backend_ok;
    float model_confidence;
    smart_pong_intercept_t analytic;
    smart_pong_intercept_t model;
    smart_pong_intercept_t final_target;
} smart_pong_ai_result_t;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    smart_pong_learning_state_t state;
    uint32_t crc32;
} smart_pong_learning_blob_t;

void smart_pong_ai_default_config(smart_pong_ai_config_t* cfg);
void smart_pong_ai_default_learning_state(smart_pong_learning_state_t* st);

void smart_pong_ai_build_features(const smart_pong_game_state_t* g, float f[SMART_PONG_FEATURE_COUNT]);
void smart_pong_ai_mirror_features_for_left(float f[SMART_PONG_FEATURE_COUNT]);

bool smart_pong_ai_ball_moving_toward_paddle(const smart_pong_game_state_t* g, bool right_side);
bool smart_pong_ai_analytic_intercept(const smart_pong_game_state_t* g, bool right_side, const smart_pong_ai_config_t* cfg, smart_pong_intercept_t* out);

bool smart_pong_model_predict(const float features[SMART_PONG_FEATURE_COUNT], smart_pong_prediction_t* out, float* backend_confidence);

bool smart_pong_ai_step(
    const smart_pong_game_state_t* g,
    bool right_side,
    const smart_pong_ai_config_t* cfg,
    const smart_pong_learning_state_t* learning,
    smart_pong_ai_result_t* out);

void smart_pong_learning_on_hit(smart_pong_learning_state_t* st, float error_y, float error_z);
void smart_pong_learning_on_miss(smart_pong_learning_state_t* st, float error_y, float error_z);

uint32_t smart_pong_crc32(const void* data, uint32_t len);
bool smart_pong_learning_pack(const smart_pong_learning_state_t* st, uint32_t version, smart_pong_learning_blob_t* blob);
bool smart_pong_learning_unpack(const smart_pong_learning_blob_t* blob, uint32_t expected_version, smart_pong_learning_state_t* st);

#ifdef __cplusplus
}
#endif
