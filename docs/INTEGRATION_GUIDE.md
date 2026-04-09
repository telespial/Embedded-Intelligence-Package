# INTEGRATION_GUIDE.md

## 1. What this package provides

This package provides the full AI-side reference flow:

1. build features
2. mirror features for left-side inference when needed
3. compute analytic intercept
4. call model backend
5. decode result
6. compute confidence
7. blend analytic and model intercepts
8. apply learning adjustments
9. return final target

## 2. Core API

The primary entry point is:

```cpp
bool smart_pong_ai_step(
    const smart_pong_game_state_t* g,
    bool right_side,
    const smart_pong_ai_config_t* cfg,
    const smart_pong_learning_state_t* learning,
    smart_pong_ai_result_t* out);
```

## 3. Integration Steps

- include `smart_pong_ai.h`
- initialize config and learning state
- call `smart_pong_ai_step()` each update
- use `out.final_target` for the controlled paddle
- call learning hooks on hit and miss events
- optionally save and restore learning state

## 4. Backend Replacement

The file `src/smart_pong_model_backend.cpp` contains a deterministic sample backend.

To replace it with a real backend:

- preserve feature order
- preserve output meaning
- preserve confidence semantics

## 5. Required Safety Behavior

Do not remove:

- analytic fallback
- confidence gate
- output clamp
- final bounded target logic
