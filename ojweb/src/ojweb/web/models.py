from django.db import models
from django.contrib.auth.models import User
from django import forms

# Create your models here.
class UserProfile(models.Model):
    user = models.OneToOneField(User)
    motto = models.CharField(max_length = 256, null = True)
    school = models.CharField(max_length = 128, null = True)
    homepage = models.URLField(null = True)
    
class Problems(models.Model):
    no = models.IntegerField(unique = True)
    title = models.CharField(max_length = 128)
    desc = models.TextField()
    input = models.TextField()
    output = models.TextField()
    samplein = models.TextField()
    sampleout = models.TextField()
    hint = models.TextField(null = True)
    source = models.CharField(max_length = 512, null = True)
    timelmt = models.IntegerField(default = 1000)
    memlmt = models.IntegerField(default = 32)
    ac = models.IntegerField(default = 0)
    ce = models.IntegerField(default = 0)
    wa = models.IntegerField(default = 0)
    tl = models.IntegerField(default = 0)
    ol = models.IntegerField(default = 0)
    ml = models.IntegerField(default = 0)
    rf = models.IntegerField(default = 0)
    rt = models.IntegerField(default = 0)
    at = models.IntegerField(default = 0)
    ie = models.IntegerField(default = 0)
    
class Discussion(models.Model):
    user = models.ForeignKey(User)
    problem_no = models.ForeignKey(Problems, to_field = "no", db_column = "problem_no")
    tile = models.CharField(max_length = 128)
    content = models.TextField()
    submit_time = models.DateTimeField(auto_now_add = True)
    
class Contests(models.Model):
    title = models.CharField(max_length = 128)
    desc = models.TextField(null = True)
    problems = models.CommaSeparatedIntegerField(max_length = 32)   #1001,1002,1003
    begin_time = models.DateTimeField()
    end_time = models.DateTimeField()
    is_running = models.NullBooleanField(default = False)
    
class Submits(models.Model):
    problem_no = models.ForeignKey(Problems, to_field = "no", db_column = "problem_no")
    contest = models.IntegerField(default = 0)      #contest id
    user = models.ForeignKey(User)
    lang = models.IntegerField()
    code = models.CharField(max_length = 4096)
    code_len = models.IntegerField()
    result_code = models.IntegerField(default = 0)
    result_text = models.CharField(max_length = 128, default = "Waiting")
    run_time = models.IntegerField(default = 0)
    run_mem = models.IntegerField(default = 0)
    submit_time = models.DateTimeField(auto_now_add = True)
    
class CompileInfo(models.Model):
    submit = models.OneToOneField(Submits, primary_key = True)
    info = models.TextField()

class Comments(models.Model):
    name = models.CharField(max_length=32)
    content = models.CharField(max_length=256)
    submit_time = models.DateTimeField(auto_now_add=True)