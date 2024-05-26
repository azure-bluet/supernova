import os, time

try: os.mkdir ('data')
except FileExistsError: pass

if os.fork (): os.system ('./supernova.out')
else: os.system ('python3 -m http.server -d http --cgi')
