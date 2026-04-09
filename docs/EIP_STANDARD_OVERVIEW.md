# EIP Standard Overview

## 1. Definition

An **Embedded Intelligence Package (EIP)** is a deployable package that contains the machine-readable specification, documentation, and runtime code required to integrate an embedded intelligence component into a project.

An EIP is not just a model file and not just a datasheet.

An EIP shall contain:

- model specification
- integration documentation
- onboarding documentation
- runtime source code or generated source output
- example usage

## 2. Required Files

A conforming EIP package shall include at minimum:

- `model.eil.json`
- `MODEL_DATASHEET.md`
- `INTEGRATION_GUIDE.md`
- `ENGINEER_QUICKSTART.md`

A deployable EIP should also include:

- public headers
- runtime source implementation
- example code
- optional validation artifacts

## 3. Design Goals

The EIP format is intended to:

- reduce ambiguity in embedded AI integration
- make models understandable to tools and engineers
- preserve fallback and safety behavior
- support low-AI-knowledge firmware developers
- align with the systems-oriented philosophy used for MRD

## 4. Standard Principle

**MRD defines the hardware.  
EIP delivers the intelligence.  
EIL runs it safely.**
