# Fix Raiden Boss Integration Tester

[![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/nhok0169/Fix-Raiden-Boss/integration-tests.yml?label=Integration%20Tests)](https://github.com/nhok0169/Fix-Raiden-Boss/actions/workflows/integration-tests.yml)


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

**Note:** *When generating expected outputs using the `produceOutputs` command, run the command on a **Linux OS** so that the file seperator stays consistent with the CI automation in Github Actions*
