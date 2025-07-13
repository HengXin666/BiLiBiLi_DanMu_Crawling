import Include
from src.api import danMaKuApi as Api

if __name__ == '__main__':
    api = Api.DanMaKuApi(["f45db2c1%2C1755421234%2Cbc613%2A21CjCFD5eVfJiSSWn8O1FE9CR6lOcA1NKxzBJS7b5VO51ye-etSuITLEU8t13fX2lmhTESVlpRcjdhUElRQjdEUk40SzNaUmlHdXR0bkp2QTdZNVJPTU5abUhQbnp1VnpCOHhHdkNSbUFBN29jVDkzekVDX2cwd3VsMHFCQjdZeXFKSFlFZ1Fsc1ZBIIEC"])
    res = api.getBasDanMaKu(1176840)
    print(res)