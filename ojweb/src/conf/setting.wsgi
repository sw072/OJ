import os 
import sys 
sys.stdout = sys.stderr 
from os.path import abspath, dirname, join 
from django.core.handlers.wsgi import WSGIHandler 
sys.path.insert(0, abspath(join(dirname(__file__), "../"))) 
os.environ["DJANGO_SETTINGS_MODULE"] = "ojweb.settings" #your settings module 
application = WSGIHandler()