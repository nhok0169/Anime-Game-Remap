# FIX RAIDEN BOSS
[![PyPI](https://img.shields.io/pypi/pyversions/FixRaidenBoss2)](https://www.python.org/downloads/)
[![PyPI](https://img.shields.io/pypi/v/FixRaidenBoss2)](https://pypi.org/project/FixRaidenBoss2/)
[![PyPI](https://img.shields.io/pypi/dm/FixRaidenBoss2?label=pypi%20downloads)](https://pypi.org/project/FixRaidenBoss2/)
[![Documentation Status](https://readthedocs.org/projects/fix-raiden-boss/badge/?version=latest)](https://fix-raiden-boss.readthedocs.io/en/latest/?badge=latest)

<a href=""><img alt="" src="https://github.com/nhok0169/Fix-Raiden-Boss/blob/nhok0169/Docs/src/_static/images/raiden.jpg" style="width:750px; height: auto;"></a>
- Author Ideal [NK#1321](https://discordapp.com/users/277117247523389450)
- Thank [SilentNightSound#7430](https://github.com/SilentNightSound) for the logic rewrite
- Thank HazrateGolabi#1364 for combine and make final script
- Thank [Albert Gold#2696](https://github.com/Alex-Au1) for update the code for merged mods
## Requirements 
- [Python (version 3.6 and up)](https://www.python.org/downloads/)

## API Documentation
If you are a **tool developer** who wants to use the fix in **your own code**, visit the documentation here:
- https://fix-raiden-boss.readthedocs.io/en/latest/

## VIDEO TUTORIAL AND EXAMPLES:

### Quickstart
**Individual Mod:** https://www.youtube.com/watch?v=29FM0GywcWA  
**Merged Mods:** https://www.youtube.com/watch?v=nEyMYIHdrQM  
**Mega Merged Mods:** https://www.youtube.com/watch?v=08co5ct7zeg  

### More Features
[More examples here](https://github.com/nhok0169/Fix-Raiden-Boss/tree/nhok0169/Examples)

## How to Run
- Choose your pick of which way to run the script:

  1. [Quickstart!](#lets-start-)  &nbsp;&nbsp;(for beginners)
  2. [CMD WITHOUT a Script](#run-on-cmd-without-a-script) &nbsp;&nbsp; (recommended if you run by CMD)
  3. [CMD with a Script](#run-on-cmd-with-a-script) &nbsp;&nbsp; (the convention that other GIMI scripts follow)

## Let's Start !
### STEP 1:
- Copy [THIS](https://github.com/nhok0169/Fix-Raiden-Boss/blob/nhok0169/Fix-Raiden-Boss%202.0%20(for%20all%20user%20)/src/FixRaidenBoss2/FixRaidenBoss2.py) script in your Raiden Mod folder or GIMI's `Mod` folder.

*Make sure the `.ini` files contain the section named `[TextureOverrideRaidenShogunBlend]` or use the `--all` option to read all .ini files the program encounters*
### STEP 2:
- Double click on the script
### STEP 3:
- Open the game and enjoy it

## Run on CMD Without a Script
### STEP 1:
- Install the module onto your computer by [opening cmd](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd) and typing :
```python
python -m pip install -U FixRaidenBoss2
```
then enter

*( you can now run the program anywhere without copying a script! )*

### STEP 2:
- [open cmd](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd) in your Raiden Mod folder or GIMI's `Mod` folder and type:
```python
python -m FixRaidenBoss2
```
then enter

*Make sure the `.ini` files contain the section named `[TextureOverrideRaidenShogunBlend]` or use the `--all` option to read all .ini files the program encounters*
### STEP 3:
- Open the game and enjoy it

## Run on CMD With a Script
### STEP 1:
- Copy [THIS](https://github.com/nhok0169/Fix-Raiden-Boss/blob/nhok0169/Fix-Raiden-Boss%202.0%20(for%20all%20user%20)/src/FixRaidenBoss2/FixRaidenBoss2.py) script in your Raiden Mod folder or GIMI's `Mod` folder  

### STEP 2:
- [open cmd](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd) and type
```python
python FixRaidenBoss2.py
```
then enter

*Make sure the `.ini` files contain the section named `[TextureOverrideRaidenShogunBlend]` or use the `--all` option to read all .ini files the program encounters*
### STEP 3:
- Open the game and enjoy it

## Command Options
```
  -h, --help          show this help message and exit
  -s str, --src str   The path to the Raiden mod folder. If this option is not specified, then will
                      use the current directory as the mod folder.
  -d, --deleteBackup  deletes backup copies of the original .ini files
  -f, --fixOnly       only fixes the mod without cleaning any previous runs of the script
  -r, --revert        reverts back previous runs of the script
  -l, --log           Logs the printed out log into a seperate .txt file
  -a, --all           Parses all *.ini files that the program encounters instead of only parsing
                      *.ini files that have the section [TextureOverrideRaidenShogunBlend]
```

<br>

## API Usage

Tool developpers can now include the fix within their code!

### API Documentation
For more info about how to use the API, visit the documentation at https://fix-raiden-boss.readthedocs.io/en/latest/

<br>

### API Setup

<br>

*Make sure you first install the module by typing into [cmd](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd):*
```bash
python -m pip install -U FixRaidenBoss2
```
<br>

### API Examples

See the documentation for more detailed [examples](https://fix-raiden-boss.readthedocs.io/en/latest/apiExamples.html) on how to use the API.

<br>

Below is a ***preview*** that gives a feel of using the API

```python
import FixRaidenBoss2 as FRB

raidenBossFixService = FRB.RaidenBossFixService(path = r"my raiden folder path", log = True, verbose = False)
raidenBossFixService.fix()

print("The Raiden Mod is fixed!")
```
<br>

<details>
<summary>Example Result</summary>
<br>

```
Creating log file, RSFixLog.txt
The Raiden Mod is fixed!
```
</details>
<br>
