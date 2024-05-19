import os, time

try: os.mkdir ('data')
except FileExistsError: pass

if os.fork (): os.system ('./supernova.out')
elif os.fork (): os.system ('python3 -m http.server -d http --cgi')
else:
    time.sleep (1)
    os.system ('./daemon.out')
