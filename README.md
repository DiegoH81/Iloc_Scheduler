# ILOC Scheduler

List scheduler for basic blocks of ILOC code, written in C++.

## Features

- Forward list scheduling (default)
- Backward list scheduling (`-b`)
- Register renaming to remove anti and output dependencies (`-r`)
- Random tie-breaking (`-rand`)
- RBF: best of 5 forward + 5 backward randomized passes (`-rbf`)

Supported instructions: `loadI`, `loadAI`, `storeAI`, `add`, `sub`, `mult`, `lshift`, `rshift`.

## Build

```bash
g++ -std=c++17 -O2 main.cpp -o main.exe
```

## Usage

```bash
./main.exe <units> [-r] [-b] [-rbf] [-rand] < file.iloc
```

PowerShell:

```powershell
Get-Content tests\t1.txt | .\main.exe 1
```
