# C++ Unit Test Generator with LLM Integration


An automated unit test generator for C++ applications that uses Large Language Models (LLMs) to generate, refine, and optimize test cases.

## Features

- 🚀 Automatic generation of Google Test unit tests from C++ source code
- 🔄 Iterative refinement based on build errors and test coverage
- 📊 Integration with GNU coverage tools (lcov, gcovr)
- � YAML-based configuration for test generation rules
- 🤖 Support for both API-based and self-hosted LLMs
- 🏗️ CMake integration for easy build system support

## Prerequisites

- C++ compiler (g++ or clang++)
- CMake (version 3.10+)
- Google Test framework
- Python 3.8+
- LLM access (OpenAI API or self-hosted model)

## Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/cpp-test-generator.git
cd cpp-test-generator

# Install dependencies
sudo apt install g++ cmake lcov gcovr libgtest-dev python3 python3-pip

# Install Python requirements
pip install -r requirements.txt
