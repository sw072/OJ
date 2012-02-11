from django.shortcuts import render_to_response
from django.core.paginator import Paginator
from ojweb.web.models import Problems, Submits, Contests, CompileInfo, UserProfile, Comments
from django.core.paginator import PageNotAnInteger, Paginator, InvalidPage, EmptyPage
from django.http import HttpResponseRedirect
from django.contrib.auth.models import User
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.decorators import login_required
from django.template import RequestContext
# Create your views here.
def index(request):
    return render_to_response("index.html", context_instance=RequestContext(request));

def problem_list(request, page_idx_str = '1'):
    num_per_page = 20
    try:
        page_idx = int(page_idx_str)
    except(EmptyPage,InvalidPage,PageNotAnInteger):
        page_idx = 1;
    if(page_idx < 1): page_idx = 1
    problems = Problems.objects.all()
    pages = Paginator(problems, num_per_page)
    if(page_idx > pages.num_pages):
        page_idx = pages.num_pages
    page = pages.page(page_idx)
    abstract = []
    for p in page.object_list:
        abstract.append({"no" : p.no, "title" : p.title, "ac" : p.ac,  
                         "submit" : p.ac + p.ce + p.wa + p.tl + p.ml + p.ol + p.rt + p.at + p.ie}) 
    return render_to_response("problems.html", { "problems" : abstract , "page_idx" : page_idx, "page_idx_arr" : range(1, pages.num_pages + 1) }, context_instance=RequestContext(request))

def problem_detail(request, pno):
    problem = Problems.objects.filter(no = pno)
    if len(problem) == 1:
        p = problem[0]
        total_submission = p.ac + p.ce + p.wa + p.tl + p.ml + p.ol + p.rt + p.at + p.ie
        return render_to_response("problem.html", { "problem" : p, "pno" : p.no , "total_submission" : total_submission }, context_instance=RequestContext(request))
    else:
        return render_to_response("/error", context_instance=RequestContext(request))

@login_required
def submit(request, pno = '0'):
    if(request.method == 'POST'):
        if pno == '0':
            pno = request.POST["pno"]
        submit = Submits()
        submit.problem_no = Problems.objects.get(no=pno)
        submit.contest = 0
        submit.user = request.user
        submit.lang = request.POST["lang"]
        submit.code = request.POST["code"]
        submit.code_len = len(submit.code)
        #print "%s\n%s\n%s\n%s\n" % pno, submit.lang, submit.code, submit.code_len
        if(not submit.code_len or submit.code_len > 4096):
            return HttpResponseRedirect('/error')
        submit.save()
        return HttpResponseRedirect('/onlinestatus')
    else:
        if pno == '0':
            pno = ''
        return render_to_response("submit.html", { "pno" : pno }, context_instance=RequestContext(request))

def onlie_status(request, page_idx_str = '1'):
    num_per_page = 20
    try:
        page_idx = int(page_idx_str)
    except(EmptyPage,InvalidPage,PageNotAnInteger):
        page_idx = 1;
    if(page_idx < 1): page_idx = 1
    submits = Submits.objects.all().order_by("-id")
    pages = Paginator(submits, num_per_page)
    if(page_idx > pages.num_pages):
        page_idx = pages.num_pages
    page = pages.page(page_idx)
    return render_to_response("onlinestatus.html", { "status" : page.object_list, "curr_page" : page_idx, "max_page" : pages.num_pages }, context_instance=RequestContext(request))

def compile_info(request, sid):
    s = Submits.objects.get(id=sid)
    compile_info = CompileInfo.objects.get(submit=s)
    if compile_info:
        return render_to_response("compileinfo.html", { "compile_info" : compile_info })
    else:
        return render_to_response("/error")

@login_required
def code_view(request, sid):
    s = Submits.objects.get(id=sid)
    code = s.code
    if s:
        return render_to_response("codeview.html", { "code" : code })
    else:
        return render_to_response("/error")

def user_info(request, uid):
    user = User.objects.get(pk=int(uid))
    u = {}
    if user:
        u["username"] = user.username
        u["email"] = user.email
        u["last_login"] = user.last_login
        u["motto"] = user.get_profile().motto
        u["school"] = user.get_profile().school
        u["homepage"] = user.get_profile().homepage
        ac = Submits.objects.filter(result_code=2).filter(user=user).distinct().order_by("problem_no")
        solved_problems = []
        for s in ac:
            if(s.problem_no.no not in solved_problems):
                solved_problems.append(s.problem_no.no)
        return render_to_response("userinfo.html", { "user" : u , "solved_problems" : solved_problems}, context_instance=RequestContext(request))
    else:
        return render_to_response("/error")

def register(request):
    new_user = {}
    new_user["user_name"] = ""
    new_user["user_pass1"] = ""
    new_user["user_pass2"] = ""
    new_user["user_email"] = ""
    new_user["user_school"] = ""
    new_user["user_motto"] = ""
    new_user["user_homepage"] = ""
    msgs = []
    if (request.method == 'POST'):
        new_user["user_name"] = request.POST.get("user_name", "")
        new_user["user_pass1"] = request.POST.get("user_pass1", "")
        new_user["user_pass2"] = request.POST.get("user_pass2", "")
        new_user["user_email"] = request.POST.get("user_email", "")
        new_user["user_school"] = request.POST.get("user_school", "")
        new_user["user_motto"] = request.POST.get("user_motto", "")
        new_user["user_homepage"] = request.POST.get("user_homepage", "")
        if(new_user["user_name"] == ""):
            msgs.append("user name can't be NULL")
        else:
            u = User.objects.filter(username=new_user["user_name"])
            if len(u):
                msgs.append("%s is already exsist" % new_user["user_name"])
        if(new_user["user_pass1"] == "" or new_user["user_pass2"] == ""):
            msgs.append("password can't be NULL")
        if(new_user["user_pass1"] != new_user["user_pass2"]):
            msgs.append("passwords are not the same")
        elif(len(new_user["user_pass1"]) <= 3):
            msgs.append("password is to short")
        if len(msgs) == 0:
            new = User(username=new_user["user_name"], email=new_user["user_email"])
            new.set_password(new_user["user_pass1"])
            new.save()
            profile = UserProfile(user=new, motto=new_user["user_motto"], school=new_user["user_school"], homepage=new_user["user_homepage"])
            profile.save()
            user = authenticate(username = new_user["user_name"], password = new_user["user_pass1"])
            if(user):
                login(request, user)
            return HttpResponseRedirect("/")
    else:
        pass
    return render_to_response("register.html", { "user_info" : new_user, "msgs" : msgs })

def user_rank(request, page_idx_str='1'):
    num_per_page = 20
    try:
        page_idx = int(page_idx_str)
    except(EmptyPage,InvalidPage,PageNotAnInteger):
        page_idx = 1;
    if(page_idx < 1): page_idx = 1
    raw_rank_list = User.objects.raw("select auth_user.id,count(submits.problem_no) as ac_num from auth_user left outer join (select distinct user_id,problem_no from web_submits where result_code=2) as submits on submits.user_id=auth_user.id group by auth_user.id order by ac_num desc")
    rank_list = []
    for r in raw_rank_list:
        rank_list.append(r)
    pages = Paginator(rank_list, num_per_page)
    if(page_idx > pages.num_pages):
        page_idx = pages.num_pages
    page = pages.page(page_idx)
    return render_to_response("userrank.html", { "rank_list" : page.object_list, "curr_page" : page_idx, "max_page" : pages.num_pages, "start_idx" : (page_idx - 1) * num_per_page }, context_instance=RequestContext(request))
    

def log_in(request):
    if(request.method == 'POST'):
        user_name = request.POST["user_name"]
        user_pass = request.POST["user_pass"]
        next_url = request.POST["next"]
        user = authenticate(username = user_name, password = user_pass)
        if user:
            login(request, user)
            return HttpResponseRedirect(next_url)
        else:
            return render_to_response("/error")
    else:
        next_url = request.GET.get("next")
        if not next_url:
            next_url = "/"
        return render_to_response("login.html", {"next" : next_url}, context_instance=RequestContext(request))

def log_out(request):
    logout(request)
    return HttpResponseRedirect("/")

def comment(request):
    comments = Comments.objects.all().order_by("-submit_time")
    msgs = []
    comment = {}
    comment["name"] = ""
    comment["content"] = ""
    if(request.method=='POST'):
        comment["name"] = request.POST["user_name"]
        comment["content"] = request.POST["content"]
        if(comment["name"] == ""):
            msgs.append("name can not be NULL")
        if(comment["content"] == ""):
            msgs.append("content can not be NULL")
        if not len(msgs):
            c = Comments()
            c.name = comment["name"]
            c.content = comment["content"]
            c.save()
            comment["name"] = ""
            comment["content"] = ""
    return render_to_response("comment.html", { "comments" : comments, "comment" : comment, "msgs" : msgs }, context_instance=RequestContext(request))
    
                