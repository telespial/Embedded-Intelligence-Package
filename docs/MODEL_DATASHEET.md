# MODEL_DATASHEET.md

## 1. Scope

This document defines the Smart Pong intercept model package and the associated reference runtime implementation.

## 2. Model Identity

**Model Name:** Smart Pong Intercept Predictor  
**Model ID:** `eil.smartpong.v1`  
**Version:** `1.1.0`  
**Package Type:** EIP  
**Runtime Class:** EIL hybrid advisory model  
**Target Class:** MCU with optional NPU acceleration

## 3. Purpose

The model predicts:

- `y_hit`
- `z_hit`
- `t_hit`

These values are used to help determine where the controlled paddle should move to meet the ball.

## 4. Hybrid Control Role

This model is not direct end-to-end control.

The final target is produced by combining:

- analytic intercept prediction
- model output
- confidence-gated blending
- learned lead and style bias
- controlled noise
- speed limiting

## 5. Input Contract

The model consumes a 16-element floating-point feature vector with fixed ordering.

## 6. Output Contract

The model produces a 3-value prediction:

- `y_hit`
- `z_hit`
- `t_hit`

## 7. Runtime Policy

The following behavior shall be preserved:

- model inference only when the ball is approaching the controlled paddle
- analytic intercept always available
- disagreement reduces model influence
- low-confidence output reduces model influence
- output remains clamped and bounded

## 8. Learning

Learning hooks are exposed for:

- hit events
- miss events

These update lightweight state used for lead, style, and noise behavior.

## 9. Persistence

Learning state may be packed into a persistence blob with:

- magic value
- version
- CRC validation

## 10. Intended Audience

This package is intended for:

- firmware engineers
- validation engineers
- EIL plugin authors
- platform teams
