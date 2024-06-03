# Fix Raiden Boss Linker

Explicitly links any needed data dependencies into the fix script.

We chose to inject the needed data into the script since:
- Want script to only be in 1 file, so people unfamiliar with Python only need to run/manage 1 file for simplicity
- Seperate data from the main fix script

<br>

## How to run
On [CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py
```

<br>

## Linked Dependencies
The following dependencies will be linked into the script:

- Hashes
- Indices
