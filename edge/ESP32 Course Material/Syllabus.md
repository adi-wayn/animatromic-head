# Syllabus — MicroControllers Course (ESP32 for EE & CE), 2026

## Course Description

This course provides a theoretical introduction to microprocessors and microcontrollers in embedded systems, with a particular emphasis on the ESP32 as a reference platform. The course covers the architectural principles, internal organization, and operating mechanisms of modern microcontrollers, including registers, memory, digital and analog I/O, timers, interrupts, watchdog mechanisms, serial communication protocols, and real-time operating system concepts.

Although the ESP32 is used throughout the course as the main concrete example, the emphasis is on **conceptual understanding, system-level reasoning, and architectural analysis**, rather than laboratory implementation.

## Course Objectives

- Understand the difference between a microprocessor and a microcontroller
- Study the architecture of embedded systems through the ESP32 as a representative modern platform
- Understand the role of registers, memory organization, buses, and address mapping
- Analyze the operation of GPIO, ADC, PWM, timers, and interrupts
- Understand the principles behind UART, SPI, and I2C communication
- Learn the purpose and operation of watchdog timers and reliability mechanisms
- Gain a conceptual introduction to real-time systems and RTOS principles

## Why ESP32?

The ESP32 is a useful reference platform because it combines many important ideas from modern embedded systems in a single microcontroller family:

- General-purpose digital I/O
- Analog capabilities
- Hardware timers and PWM
- Interrupt support
- UART, SPI, and I2C communication
- Wireless connectivity such as Wi-Fi and Bluetooth
- Multi-core processing and RTOS support

For this reason, the ESP32 serves as an effective case study for understanding broader microcontroller concepts.

## Prerequisites

- Introduction to Programming
- Digital Logic Systems
- Electronic Lab 1 *(recommended)*

## Assessment

- **100% — Final exam**

## Important Notes

- This is a **theoretical course, not a laboratory course**.
- **No hardware project is required** (but the staff strongly recommend you play with the modules).

---

# Weekly Topics

## Week 1 — Programming Recap and Course Introduction
- Course goals, structure, expectations, and the role of ESP32 in the course
- Why programming foundations matter in embedded systems
- Variables and data types in C
- Memory representation of variables
- Pointers: addresses, dereferencing, and why pointers matter in low-level programming
- The connection between pointers and memory-mapped hardware access
- Bitwise operations: AND, OR, XOR, NOT, shifts
- Why bitwise operations are essential for register manipulation in embedded systems
- Reading and understanding simple low-level C code examples

## Week 2 — Microcontroller 101 and GPIO in the ESP32
- What is a microcontroller and how it differs from a general-purpose processor
- Basic structure of a microcontroller: CPU, memory, peripherals, buses
- The ESP32 as a modern example of a microcontroller platform
- Introduction to ESP32 documentation and data-sheets
- The role of Arduino as an accessible software layer
- GPIO fundamentals in the ESP32
- Output vs input pins
- Digital logic levels and signal interpretation
- Push buttons and switches as digital inputs
- Mechanical bouncing in buttons and why it matters
- Software and hardware ideas for debouncing
- Pull-up and pull-down resistors
- Internal vs external pull resistors in the ESP32

## Week 3 — ADC
- Analog vs digital signals
- Why real-world signals are often analog
- The role of the Analog-to-Digital Converter in embedded systems
- Sampling: what it means to measure a signal in discrete time
- Quantization: mapping a continuous range into discrete digital values
- Resolution and number of bits
- Voltage range and reference considerations
- Quantization error and basic measurement limitations
- Noise and other practical sources of inaccuracy
- ADC concepts in the context of the ESP32
- Reading sensor values conceptually through ADC channels

## Week 4 — Serial Communication, Part 1: UART
- Why microcontrollers need serial communication
- Parallel vs serial communication
- UART as asynchronous serial communication
- TX and RX lines
- Start bit, stop bit, and data framing
- Baud rate and synchronization
- Common UART use cases: debugging, logging, communicating with modules
- UART in the ESP32
- Conceptual reading of UART traffic
- Advantages and limitations of UART compared to other interfaces

## Week 5 — Serial Communication, Part 2: I2C and SPI
- Why different communication protocols exist
- Synchronous vs asynchronous communication
- I2C fundamentals: SDA, SCL, addressing, master-slave model
- Multi-device communication on a shared bus
- Advantages and limitations of I2C
- SPI fundamentals: MOSI, MISO, SCLK, CS
- Full-duplex communication in SPI
- Master-slave control in SPI
- Advantages and limitations of SPI
- Conceptual comparison between UART, I2C, and SPI
- Typical peripheral communication in the ESP32 using I2C and SPI

## Week 6 — Interrupts, Counters, and Timers
- Polling vs interrupts
- Why interrupts are central in embedded systems
- Interrupt service routine concept
- External vs internal interrupts
- Latency, priority, and responsiveness
- Timer fundamentals in microcontrollers
- Counters and timer-based events
- Prescalers, overflow, and periodic timing
- Measuring time and generating timed behavior
- Conceptual use of interrupts and timers in the ESP32

## Week 7 — Watchdog Timer and Sleep Modes
- Why reliability matters in embedded systems
- What a Watchdog Timer is and how it improves robustness
- Typical failure scenarios in embedded applications
- Reset and recovery concepts
- Watchdog behavior in ESP32-based systems
- Power management as a design consideration
- Why sleep modes are important in modern embedded devices
- Overview of idle, light sleep, and deep sleep ideas
- Tradeoffs between performance, responsiveness, and power saving
- Sleep mode concepts in the ESP32

## Week 8 — PWM
- What Pulse Width Modulation is
- Duty cycle and frequency
- Average power and control through switching
- Why PWM is useful for LED dimming, motor control, and signal generation
- Hardware support for PWM in microcontrollers
- Conceptual interpretation of PWM waveforms
- PWM resources in the ESP32
- Limits and design considerations in PWM-based control

## Week 9 — Wi-Fi Connectivity and IoT
- Why connectivity is important in modern embedded systems
- The ESP32 as a connected microcontroller
- Basic Wi-Fi concepts relevant to embedded systems
- Station mode and Access Point mode
- Sending data from devices to external systems
- Introduction to IoT concepts
- Sensors, devices, cloud services, and communication workflows
- Examples of IoT applications using ESP32
- System-level considerations: latency, reliability, power, and security

## Week 10 — FreeRTOS, Part 1
- Why real-time thinking matters in embedded systems
- What a real-time operating system is
- Bare-metal programming vs RTOS-based design
- Tasks and concurrent execution
- Scheduling concepts
- Priority-based execution
- Why ESP32 commonly uses FreeRTOS
- Core concepts of task-based software organization
- Benefits and tradeoffs of using an RTOS

## Week 11 — FreeRTOS, Part 2
- Task coordination and synchronization
- Queues and message passing
- Semaphores and mutexes
- Resource sharing in embedded systems
- Timing and periodic tasks
- Common conceptual pitfalls in concurrent systems
- Deadlocks, race conditions, and design discipline
- Understanding RTOS concepts in the context of the ESP32
- Bringing together peripherals, timing, and task organization into one system view
