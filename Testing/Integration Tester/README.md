# AG Remap's Integration Tester

[![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/nhok0169/Anime-Game-Remap/integration-tests.yml?label=Integration%20Tests)](https://github.com/nhok0169/Anime-Game-Remap/actions/workflows/integration-tests.yml)


## How To Run
On [CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py [command name]
```

For the different command names see the list below

## Commands
| Command | Description |
| --- | --- |
| produceOutputs | Produces the expected outputs for the tests
| runSuite | Compares the ran results with the expected results of the test
| printOutputs | Prints out the expected outputs for the tests
| clearOutputs | Erases all output folders

<br>

**Note:** *When generating expected outputs using the `produceOutputs` command, run the command on a **Linux OS** so that the file seperator stays consistent with the CD automation in Github Actions*

<br>

## Command Options

Most of the options/arguments are based off the options/arguments from Python's [unittest](https://docs.python.org/3/library/unittest.html) package
except for the `--system` option and `command` argument

<br>

### Positional Arguments
| Argument Name | Description |
| --- | --- |
| command | The command to run the integration tester |
| tests | a list of any number of test modules, classes and test methods. |

<br>

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
python3 main.py [commandName] [TestSuiteName]
```

<br>

## Running a Specific Test

For easier debugging or to save time, you may only want to run a single test. You can do this by running the following command:

```bash
python3 main.py [commandName] [TestSuiteName].[TestName]
```
