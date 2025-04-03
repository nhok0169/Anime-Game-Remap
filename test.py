import gdown
import pkg_resources
from packaging.version import Version


resultVers = pkg_resources.get_distribution("urllib3").version
print(f"VERSION: {resultVers} AND {type(resultVers)}")

resultVers = Version(resultVers)
actualVers = Version("1.26.5a1")

print(f"HELLO: {resultVers} AND {actualVers} AND {resultVers > actualVers}")




# url = 'https://drive.google.com/uc?id=0B9P1L--7Wd2vNm9zMTJWOGxobkU' 
# https://drive.google.com/file/d/1MSsxnM0cIKniI83RhZpU9qcyVoFs9_nD/view?usp=sharing

"""
url = r"https://drive.google.com/file/d/1MSsxnM0cIKniI83RhZpU9qcyVoFs9_nD/view?usp=sharing"
id = r"1MSsxnM0cIKniI83RhZpU9qcyVoFs9D_n"
output = 'hutaoTest.dds'
gdown.download(url=url, output=output, fuzzy=True)"
"""