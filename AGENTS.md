# Agent instructions

## Project and C style

- This is a small C/CMake file-based calculator. Read README.md and the nearby
  implementation before changing behavior. Keep the code easy to follow and
  avoid frameworks, unrelated refactors, and generated IDE/build metadata.
- Store whole numbers as decimal character sequences with a separate sign.
  Implement carrying, borrowing, school multiplication, and long division.
  Never convert a whole operand to a machine numeric type or use a big number
  library. Native integers are appropriate for one digit, a carry, or an index.
- Use dynamically sized number and line buffers. Check allocation failures,
  size arithmetic, bounds, ownership, and cleanup on every error path.
- Keep arithmetic separate from expression parsing and file input/output.
  Prefer small functions, explicit status codes, and a simple public header.
- Preserve signed arithmetic, canonical zero, division toward zero, and the
  documented input/output contract. Keep README.md and regression coverage in
  sync with behavior changes. Preserve the user's existing changes.

## CLion MCP access

Prefer tools exposed by the CLion instance for this project. In this runtime
they are available through `mcp__jetbrains_air_ide__execute_tool`, whose
`command` uses `tool_name --parameter value` syntax, for example:

```text
list_directory_tree --directoryPath . --maxDepth 2 --projectPath /Users/ernest.molczan/CLionProjects/Calculator
read_file --file_path main.c --projectPath /Users/ernest.molczan/CLionProjects/Calculator
get_compiler_info --filePath main.c --projectPath /Users/ernest.molczan/CLionProjects/Calculator
```

Pass the actual project root as `projectPath`. Array/object parameters use
quoted JSON values. Discover the currently exposed tools and their parameters
before relying on them; different JetBrains instances expose different sets.
Do not route Calculator requests to a separate IntelliJ monorepo instance.
If a tool is unavailable or cannot analyze C, say so and use a focused local
compiler/CMake command instead. Never claim a check succeeded without a result.

### 1. Project orientation and source reading

- `list_directory_tree`: inspect the project structure before adding files.
- `read_file`: read numbered source, headers, CMake setup, and tests.
- `search_file`: locate files by glob; `search_text` and `search_regex`: locate
  exact definitions, constants, and related cases.
- Use `context search` for an unfamiliar semantic question when available;
  after locating the relevant area, use direct reads and exact searches. If
  semantic search cannot index this project, continue with CLion/file tools.
- `get_repositories` and `git_status`: inspect version-control state when a
  repository exists. This starter was created without a Git repository; do not
  initialize one unless requested. Local `rg` and `git diff` are useful fallbacks.

### 2. C symbols, compiler settings, and inspections

- `get_project_status`: check that indexing and project loading are ready.
- `search_symbol`: locate C functions, structs, and declarations.
- `get_compiler_info` with `filePath`: verify the actual compiler, C standard,
  include paths, and compilation options instead of guessing.
- `get_file_problems`: inspect changed `.c`, `.h`, and CMake files for errors and
  warnings after edits. Check both source and header declarations.
- `get_diagnostic_info`: investigate CMake/toolchain/project-loading problems.
- Use `get_symbol_info` if that specific CLion connection exposes it; otherwise
  inspect declarations with `search_symbol` and `read_file`. The current AIR
  tool set does not expose `get_symbol_info` or `build_project`.

### 3. Small, reviewable edits

- `create_new_file`: add headers, source, tests, and documentation without
  overwriting existing files.
- `apply_patch`: make focused edits and inspect the resulting files.
- `reformat_file`: format only the changed files when needed; do not reformat
  the whole project. Keep existing naming and indentation conventions.
- Use local `apply_patch` when it is simpler or the IDE edit tool is unavailable.
  Do not change `.idea`, caches, or generated build files by hand.

### 4. Build, run, and regression verification

- `get_run_configurations`: discover the exact CMake executable/test
  configuration names; use its capability flags before setting launch overrides.
- `execute_run_configuration`: build/run a discovered configuration and check
  the exit code. Use temporary arguments for input/output paths only when
  `supportsDynamicLaunchOverrides` is true.
- `execute_terminal_command`: run CMake/CTest or focused compiler commands in
  CLion when available. A local terminal is an acceptable fallback.
- Build with compiler warnings enabled, then run `ctest --test-dir build
  --output-on-failure`. For buffer/ownership changes, also run a focused
  AddressSanitizer/UndefinedBehaviorSanitizer build when supported.
- Cover carries and borrows across many digits, signed operations, leading
  zeros, multiplication, long division, division by zero, malformed input,
  long lines, file failures, and input/output aliases. Use independent expected
  results. External numerical tools may serve as a test oracle only.
- State which IDE inspections, build, and tests actually ran and any limits.

### 5. Native debugging when a failure needs runtime evidence

- `xdebug_set_breakpoint`, then `xdebug_start_debugger_session`: pause a
  discovered CMake configuration at the failing arithmetic or input path.
- `xdebug_get_debugger_status`: obtain the current session ID.
- `xdebug_get_stack`, `xdebug_get_frame_values`, `xdebug_get_value_by_path`, and
  `xdebug_evaluate_expression`: inspect digit buffers, lengths, indexes, and
  carries. Keep evaluated expressions free of side effects.
- `xdebug_control_session`: step, resume, or stop; refresh frame indexes after
  stepping. `xdebug_get_fault_status` helps investigate native crashes.
- `xdebug_read_memory` and `xdebug_get_registers`: use only when ordinary source
  inspection is insufficient. Do not mutate live variables or IDE settings to
  conceal a bug. Remove only breakpoints created for the task.

Database, SQL, notebook, Python-environment, and Rust tools are outside this
project's normal workflow.
