import os
import subprocess
import yaml
import requests
import time

# Configure your LLM API key and endpoint
LLM_API_KEY = "YOUR_API_KEY"  # Replace with your actual API key
LLM_API_URL = "https://openrouter.ai/api/v1/chat/completions"

def get_cpp_files(project_path):
    """Finds all C++ source files in the project, excluding specified directories."""
    cpp_files = []
    excluded_dirs = ['third_party', 'test', 'build', '.venv']
    for root, dirs, files in os.walk(project_path):
        # Modify dirs in-place to prevent os.walk from descending into excluded directories
        dirs[:] = [d for d in dirs if d not in excluded_dirs]

        # Check if the current root is within an excluded directory
        if any(excluded_dir in root for excluded_dir in excluded_dirs):
            continue

        for file in files:
            if file.endswith(".cc") or file.endswith(".cpp"):
                cpp_files.append(os.path.join(root, file))
    return cpp_files

def read_yaml_instructions(file_path):
    """Reads the YAML instruction file."""
    with open(file_path, 'r') as f:
        return yaml.safe_load(f)

def call_llm(messages, max_retries=5, initial_delay=10):
    """Calls the LLM API with the given messages, with retry and exponential backoff."""
    headers = {
        "Authorization": f"Bearer {LLM_API_KEY}",
        "Content-Type": "application/json"
    }

    delay = initial_delay
    for i in range(max_retries):
        try:
            data = {
                "model": "deepseek/deepseek-r1-0528-qwen3-8b:free",
                "messages": messages
            }
            response = requests.post(LLM_API_URL, headers=headers, json=data)
            response.raise_for_status()  # Raise an exception for bad status codes
            return response.json()['choices'][0]['message']['content']
        except requests.exceptions.RequestException as e:
            if response.status_code == 429:  # Too Many Requests
                print(f"Rate limit hit. Retrying in {delay} seconds...")
                time.sleep(delay)
                delay *= 2  # Exponential backoff
            else:
                raise e  # Re-raise other exceptions
    raise requests.exceptions.RequestException(f"Failed after {max_retries} retries due to rate limiting.")

def generate_test_file(cpp_file_path, instructions):
    """Generates a unit test file using the LLM."""
    with open(cpp_file_path, 'r') as f:
        cpp_code = f.read()

    messages = [
        {
            "role": "system",
            "content": instructions['instruction']['role']
        },
        {
            "role": "user",
            "content": f"Generate a Google Test unit test file for the following C++ code:\n\n{cpp_code}"
        }
    ]

    return call_llm(messages)

def refine_test_file(test_file_path, instructions):
    """Refines a unit test file using the LLM."""
    with open(test_file_path, 'r') as f:
        test_code = f.read()

    messages = [
        {
            "role": "system",
            "content": instructions['instruction']['role']
        },
        {
            "role": "user",
            "content": f"Refine the following Google Test unit test file:\n\n{test_code}"
        }
    ]

    return call_llm(messages)

def debug_build_failure(test_file_path, build_logs, instructions):
    """Debugs a build failure using the LLM."""
    with open(test_file_path, 'r') as f:
        test_code = f.read()

    messages = [
        {
            "role": "system",
            "content": instructions['instruction']['role']
        },
        {
            "role": "user",
            "content": f"Fix the following C++ code based on the build logs:\n\nCode:\n{test_code}\n\nBuild Logs:\n{build_logs}"
        }
    ]

    return call_llm(messages)

def build_project(project_path):
    """Builds the C++ project."""
    build_dir = os.path.join(project_path, "build")
    os.makedirs(build_dir, exist_ok=True)
    
    # Run CMake and make with coverage flags
    try:
        subprocess.run(["cmake", "..", "-DCMAKE_CXX_FLAGS='-fprofile-arcs -ftest-coverage'", "-DCMAKE_C_FLAGS='-fprofile-arcs -ftest-coverage'"], cwd=build_dir, check=True, capture_output=True, text=True)
        subprocess.run(["make"], cwd=build_dir, check=True, capture_output=True, text=True)
        return True, ""
    except subprocess.CalledProcessError as e:
        print(f"Build Error (stdout):\n{e.stdout}")
        print(f"Build Error (stderr):\n{e.stderr}")
        return False, e.stderr

def generate_coverage_report(project_path):
    """Runs tests and generates a code coverage report."""
    build_dir = os.path.join(project_path, "build")
    coverage_output_dir = os.path.join(project_path, "coverage_report")
    os.makedirs(coverage_output_dir, exist_ok=True)

    print("  Running tests for coverage...")
    try:
        # Find and run all test executables
        test_executables = []
        for root, _, files in os.walk(build_dir):
            for file in files:
                if file.startswith("test_") and os.access(os.path.join(root, file), os.X_OK):
                    test_executables.append(os.path.join(root, file))
        
        if not test_executables:
            print("  No test executables found to run for coverage.")
            return

        for test_exe in test_executables:
            subprocess.run([test_exe], cwd=build_dir, check=True, capture_output=True, text=True)

        print("  Capturing coverage data with lcov...")
        # Initialize lcov
        subprocess.run(["lcov", "--directory", build_dir, "--zerocounters"], check=True, capture_output=True, text=True)
        # Capture coverage data
        subprocess.run(["lcov", "--capture", "--directory", build_dir, "--output-file", os.path.join(coverage_output_dir, "coverage.info")], check=True, capture_output=True, text=True)
        # Generate HTML report
        subprocess.run(["genhtml", os.path.join(coverage_output_dir, "coverage.info"), "--output-directory", coverage_output_dir], check=True, capture_output=True, text=True)
        
        print(f"  Code coverage report generated at: {coverage_output_dir}/index.html")
    except subprocess.CalledProcessError as e:
        print(f"Coverage generation failed (stdout):\n{e.stdout}")
        print(f"Coverage generation failed (stderr):\n{e.stderr}")
    except FileNotFoundError as e:
        print(f"Error: {e}. Make sure lcov and genhtml are installed.")

def main():
    """Main function to drive the test generation process."""
    project_path = os.path.dirname(os.path.abspath(__file__))
    test_dir = os.path.join(project_path, "tests")
    generation_instructions = read_yaml_instructions(os.path.join(project_path, "test_generation_instructions.yaml"))
    refinement_instructions = read_yaml_instructions(os.path.join(project_path, "test_refinement_instructions.yaml"))
    debugging_instructions = read_yaml_instructions(os.path.join(project_path, "build_debugging_instructions.yaml"))

    cpp_files = get_cpp_files(project_path)
    print(f"Found C++ files: {cpp_files}")

    for cpp_file in cpp_files:
        test_file_name = f"test_{os.path.basename(cpp_file)}"
        test_file_path = os.path.join(test_dir, test_file_name)

        print(f"Processing {cpp_file}...")
        try:
            # Initial test generation
            generated_test = generate_test_file(cpp_file, generation_instructions)
            with open(test_file_path, 'w') as f:
                f.write(generated_test)
            print(f"  Successfully generated test file: {test_file_path}")

            # Refinement loop
            for i in range(3):  # Refine up to 3 times
                print(f"  Refining test file (iteration {i+1})...")
                refined_test = refine_test_file(test_file_path, refinement_instructions)
                with open(test_file_path, 'w') as f:
                    f.write(refined_test)
                print(f"  Successfully refined test file.")

                # Build and debug
                build_success, build_logs = build_project(project_path)
                if build_success:
                    print("  Build successful!")
                    generate_coverage_report(project_path) # Generate coverage report on successful build
                    break  # Exit refinement loop if build succeeds
                else:
                    print("  Build failed. Debugging...")
                    fixed_test = debug_build_failure(test_file_path, build_logs, debugging_instructions)
                    with open(test_file_path, 'w') as f:
                        f.write(fixed_test)
                    print("  Applied potential fix from LLM.")
            else:
                print(f"  Failed to fix the build after 3 attempts.")

        except requests.exceptions.RequestException as e:
            print(f"Error processing {cpp_file}: {e}")

if __name__ == "__main__":
    main()
