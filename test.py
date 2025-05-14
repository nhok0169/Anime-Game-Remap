import gdown
import pkg_resources
from packaging.version import Version
import requests


resultVers = pkg_resources.get_distribution("urllib3").version
print(f"VERSION: {resultVers} AND {type(resultVers)}")

resultVers = Version(resultVers)
actualVers = Version("1.26.5a1")

print(f"HELLO: {resultVers} AND {actualVers} AND {resultVers > actualVers}")



file_name = "test.dds"
fileRequest = requests.get(r'https://github.com/nhok0169/Anime-Game-Remap/raw/nhok0169/Testing/Integration%20Tester/IntegrationTester/Tests/APIDocsTests/inputs/CatGirl.dds')
with open(file_name, 'wb') as f:
    f.write(fileRequest.content)




# url = 'https://drive.google.com/uc?id=0B9P1L--7Wd2vNm9zMTJWOGxobkU' 
# https://drive.google.com/file/d/1MSsxnM0cIKniI83RhZpU9qcyVoFs9_nD/view?usp=sharing

"""
url = r"https://drive.google.com/file/d/1MSsxnM0cIKniI83RhZpU9qcyVoFs9_nD/view?usp=sharing"
id = r"1MSsxnM0cIKniI83RhZpU9qcyVoFs9D_n"
output = 'hutaoTest.dds'
gdown.download(url=url, output=output, fuzzy=True)"
"""