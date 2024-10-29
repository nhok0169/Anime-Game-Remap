# AG Remap's Unit Tester

[![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/nhok0169/Anime-Game-Remap/unit-tests.yml?label=Unit%20Tests)](https://github.com/nhok0169/Anime-Game-Remap/actions/workflows/unit-tests.yml)


## How To Run
On [CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py
```

## Command Options

Most of the options/arguments are based off the options/arguments from Python's [unittest](https://docs.python.org/3/library/unittest.html) package
except for the `--system` option

### Positional Arguments
| Argument Name | Description |
| --- | --- |
| tests | a list of any number of test modules, classes and test methods. |

### Options
| Options | Description |
| --- | --- |
| -h, --help | show this help message and exit |
| -v, --verbose | Verbose output |
| -q, --quiet | Quiet output |
| --locals | Show local variables in tracebacks |
| -f, --failfast | Stop on first fail or error |
| -c, --catch | Catch Ctrl-C and display results so far |
| -b, --buffer | Buffer stdout and stderr during tests |
| -k TESTNAMEPATTERNS | Only run tests which match the given substring |
| -s SYSTEM, --system SYSTEM | The system to perform the tests on. The available systems are: {'script', 'api'} |

<br>

## Running a Specific Test Suite

Sometimes, you only want to verify whether a single module is working correctly. You can do this by running the following command:

```bash
python3 main.py [TestSuiteName]
```

<br>

## Running a Specific Test

For easier debugging or to save time, you may only want to run a single test. You can do this by running the following command:

```bash
python3 main.py [TestSuiteName].[TestName]
```
