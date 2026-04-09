#include "smart_pong_ai.h"

#include <math.h>

bool smart_pong_model_predict(const float features[SMART_PONG_FEATURE_COUNT], smart_pong_prediction_t* out, float* backend_confidence)
{
    if (!features || !out || !backend_confidence) return false;

    const float ball_y = features[1];
    const float ball_z = features[2];
    const float ball_vx = features[3];
    const float ball_vy = features[4];
    const float ball_vz = features[5];
    const float paddle_y = features[10];
    const float paddle_z = features[11];
    const float score_bias = features[12];
    const float last_hit_dy = features[13];
    const float last_hit_dz = features[14];

    float t = 0.5f;
    if (fabsf(ball_vx) > 1e-5f)
        t = (1.0f - features[0]) / fabsf(ball_vx);

    if (t < 0.01f) t = 0.01f;
    if (t > 2.0f) t = 2.0f;

    float y = ball_y + ball_vy * t;
    float z = ball_z + ball_vz * t;

    y += 0.08f * (paddle_y - y);
    z += 0.08f * (paddle_z - z);
    y += 0.03f * score_bias;
    z += 0.02f * score_bias;
    y += 0.05f * last_hit_dy;
    z += 0.05f * last_hit_dz;

    if (y < 0.0f) y = 0.0f;
    if (y > 1.0f) y = 1.0f;
    if (z < 0.0f) z = 0.0f;
    if (z > 1.0f) z = 1.0f;

    out->y_hit = y;
    out->z_hit = z;
    out->t_hit = t;

    float dy = fabsf(y - paddle_y);
    float dz = fabsf(z - paddle_z);
    float c = 1.0f - 0.5f * (dy + dz);
    if (c < 0.0f) c = 0.0f;
    if (c > 1.0f) c = 1.0f;
    *backend_confidence = c;

    return true;
}
