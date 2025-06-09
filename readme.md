# Multithreaded Radar Logger Simulation in C

This project simulates a real-time radar data flow system using multithreading in C. It mimics how radar sensors in automotive systems generate, transfer, and log data using shared memory and thread synchronization.

## Project Description

This is a multithreaded radar logger simulation in C that mimics real-time automotive data flow. It uses POSIX threads, shared memory, and logging to simulate:

- Radar data generation from a virtual radar unit
- Transfer of data across threads (preprocessing stage)
- Storage/logging of radar data into a file

## 🛠️ Build Instructions
gcc radar_logger.c -o radar_logger 

## To Run the Simulation
./radar_logger
