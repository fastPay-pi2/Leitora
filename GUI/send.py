import json
import requests
import sys

lis = []

for s in sys.argv[1:]:
	lis.append(s)

print(lis)

url = "http://localhost:5000/api/purchase/"
data = {
	'items': lis
}
r = requests.put(url, json = data)

print(r.json())
