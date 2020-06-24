import requests
import datetime

r = requests.get('http://localhost:8081/seinfield.json')

content = r.json()
for line in content:
    print(line)
