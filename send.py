import json
import requests
import sys
import os
import signal

lis = []

for s in sys.argv[1:(len(sys.argv)-1)]:
	stri = "";
	a = s.split('-')
	for n in a:
		if(len(n)<2):
			stri = stri + "0" + n + "-"
		else:
			stri = stri + n + "-"
	lis.append(stri[:(len(stri)-1)])

pid = sys.argv[len(sys.argv)-1]

print(lis)
print(len(sys.argv))

url = "http://0.0.0.0:5000/api/purchase/"
data = {
	'items': lis
}
r = requests.put(url, json = data)

print(r.json())
try:
	if(r.ok==False):
		os.kill(int(pid),signal.SIGUSR1)
except:
	print("Erro ao enviar sinal de volta")
