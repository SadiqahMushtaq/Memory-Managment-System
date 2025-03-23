# Multi-Threaded File Processor 🚀  

## Overview  
This project implements a **multi-threaded file processor** using the **POSIX Threads API** in C. The goal is to efficiently process large datasets while ensuring **thread safety** and **optimal resource utilization**. The project compares a **single-threaded** and **multi-threaded** approach to analyze performance differences.  

## Features  
✅ Implements both **single-threaded** and **multi-threaded** file processing  
✅ Uses **POSIX Threads (PThreads)** for concurrency  
✅ Optimized for **large dataset processing**  
✅ Supports **command-line arguments** for flexible execution  

## Repository Structure  
- **`ST.c`** – Single-threaded file processor  
- **`MT.c`** – Multi-threaded file processor  
- **`Makefile`** – Automates compilation and execution  
- **`data_tiny.txt`**, **`data_small.txt`**, **`data_medium.txt`**, **`data_large.txt`** – Sample datasets for testing  

## Compilation & Execution  
### Build the project  
Use the `Makefile` to compile both single-threaded and multi-threaded versions:  
```bash
make
