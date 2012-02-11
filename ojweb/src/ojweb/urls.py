from django.conf.urls.defaults import *
#from ojweb.web.views import *
from django.views.generic.simple import direct_to_template

# Uncomment the next two lines to enable the admin:
# from django.contrib import admin
# admin.autodiscover()

urlpatterns = patterns('',
    (r'^$', 'ojweb.web.views.index'),
    (r'^problems/$', 'ojweb.web.views.problem_list'),
    (r'^problems/(\d+)/$', 'ojweb.web.views.problem_list'),
    (r'^problem/(\d+)/$', 'ojweb.web.views.problem_detail'),
    (r'^submit/$', 'ojweb.web.views.submit'),
    (r'^submit/(\d+)/$', 'ojweb.web.views.submit'),
    (r'^onlinestatus/$', 'ojweb.web.views.onlie_status'),
    (r'^login/$', 'ojweb.web.views.log_in'),
    (r'^logout/$', 'ojweb.web.views.log_out'),
    (r'^register/$', 'ojweb.web.views.register'),
    (r'^onlinestatus/(\d+)$', 'ojweb.web.views.onlie_status'),
    (r'^codeview/(\d+)$', 'ojweb.web.views.code_view'),
    (r'^compileinfo/(\d+)$', 'ojweb.web.views.compile_info'),
    (r'^user/(\d+)$', 'ojweb.web.views.user_info'),
    (r'^userrank/$', 'ojweb.web.views.user_rank'),
    (r'^userrank/(\d+)$', 'ojweb.web.views.user_rank'),
    (r'^faq/$', direct_to_template, { 'template': 'faq.html' }),      #FAQ
    (r'^comment/$', 'ojweb.web.views.comment'),  #About
    (r'^about/$', direct_to_template, { 'template': 'about.html' }),  #About
    (r'^css/(?P<path>.*)$', 'django.views.static.serve',{'document_root':r'E:\ojweb\src\ojweb\web\templates\css'}),
    (r'^error/$', direct_to_template, { 'template': 'error.html' }),  #Error
    (r'^SyntaxHighlighter/Styles/(?P<path>.*)$', 'django.views.static.serve',{'document_root':r'E:\ojweb\src\ojweb\web\templates\SyntaxHighlighter\Styles'}),
    (r'^SyntaxHighlighter/Scripts/(?P<path>.*)$', 'django.views.static.serve',{'document_root':r'E:\ojweb\src\ojweb\web\templates\SyntaxHighlighter\Scripts'}),
    (r'^images/(?P<path>.*)$', 'django.views.static.serve',{'document_root':r'E:\ojweb\src\ojweb\web\templates'}),    # Example:
    # (r'^ojweb/', include('ojweb.foo.urls')),

    # Uncomment the admin/doc line below to enable admin documentation:
    # (r'^admin/doc/', include('django.contrib.admindocs.urls')),

    # Uncomment the next line to enable the admin:
    # (r'^admin/', include(admin.site.urls)),
)
