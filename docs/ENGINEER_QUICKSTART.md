# ENGINEER_QUICKSTART.md

## What this package does

This package gives you a working Smart Pong intelligence module you can drop into a project and test.

It includes:

- the model input path
- the model output path
- fallback logic
- learning state
- a full example

## You do not need to know AI theory

To use this package safely, you mainly need to know:

- how to pass the 16 inputs
- when to call the model
- how to use the returned target
- how to fall back if inference is not trusted

## Fast start

1. Build the project
2. Run the console demo
3. Open `examples/full_smart_pong_example.cpp`
4. Replace the sample backend later if needed
5. Keep the fallback path enabled

## Safe changes

You can usually tune:

- confidence threshold
- style bias
- lead gain
- speed limits

## Unsafe changes

Do not change without re-versioning:

- feature order
- output order
- tensor size
- fallback behavior
