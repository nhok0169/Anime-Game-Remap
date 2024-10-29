# Fix Raiden Boss 2 Docs

[![Documentation Status](https://readthedocs.org/projects/fix-raiden-boss/badge/?version=latest)](https://fix-raiden-boss.readthedocs.io/en/latest/?badge=latest)


## Building the website

### Step 1:
On [CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), navigate to [this folder](https://github.com/nhok0169/Fix-Raiden-Boss/tree/nhok0169/Docs) where the 
[Makefile](https://github.com/nhok0169/Fix-Raiden-Boss/blob/nhok0169/Docs/Makefile) and [make.bat](https://github.com/nhok0169/Fix-Raiden-Boss/blob/nhok0169/Docs/make.bat) are located

### Step 2:

If you are using Windows, run this command:
```bash
./make.bat html
```

If you are using Linux, run this command:
```bash
make html
```

A folder called **build** should be created in the current directory with all the needed files for the website


## Running the website

Make sure you have [built the website](#building-the-website) first

### Step 1:
Go into the **build** folder that was created

### Step 2:
Click on **index.html** in the **build** folder
