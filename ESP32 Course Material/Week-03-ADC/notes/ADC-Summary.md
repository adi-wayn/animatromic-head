# ADC Fundamentals for Embedded Systems — ESP32 Course Notes

*Student handout for a 3rd-year Electrical Engineering microcontrollers course.*

## What this handout covers
This document explains the core ADC ideas needed before programming analog inputs on the ESP32-S3: analog vs digital signals, why the physical world is usually analog, the role of the ADC, sampling, sample-and-hold, SAR conversion, quantization, resolution, voltage range and reference, quantization error, noise, and practical measurement limits. It also connects signal frequency to acquisition time and conversion time, which is one of the most important concepts in real ADC design.

## Learning goals
- Understand the difference between analog and digital representations.
- Explain why most sensors produce analog signals.
- Describe the purpose of the ADC inside an embedded system.
- Explain sampling in time and quantization in amplitude.
- Understand resolution, LSB size, and why bits are not the same as accuracy.
- Explain sample-and-hold behavior and why source impedance matters.
- Describe how a SAR ADC finds a code step by step.
- Reason about acquisition time, conversion time, and what happens when the input changes during sampling.
- Identify practical error sources such as noise, reference drift, offset, gain error, and poor settling.

## 1. Analog vs digital signals

### Simple overview
An analog signal can vary continuously. In principle, between two values there are infinitely many possible intermediate values. A sensor voltage that can be 0.970 V, 0.971 V, 0.9713 V, and so on is analog.

A digital signal uses a finite set of allowed levels, usually two in embedded systems: logic 0 and logic 1. The system does not care about the exact voltage as long as it is interpreted as low or high.

> *Note: the course page exported only the opening of this handout (sections continue with
> sampling, sample-and-hold, SAR conversion, quantization, resolution, etc., per the learning
> goals above). A "SAR ADC Video Explanation" is embedded on the page for the rest.*
