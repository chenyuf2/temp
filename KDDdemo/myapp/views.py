from django.shortcuts import render, render_to_response, redirect
from django.http import HttpResponse, HttpResponseRedirect
from .forms import ContactForm
import subprocess
# Create your views here.
def index(request):
    if request.method == 'POST':
        form = ContactForm(request.POST)
        if form.is_valid():
            input = form.cleaned_data['name']
            # email = form.cleaned_data['email']
            file1 = open("KDDdemo/static/src/papers/topics_dm.txt","w")
            name = input.split(",")
            for i in range(0,len(name)):
                temp = name[i].strip()
                temp.replace(" ", "_")
                file1.write(temp + '\n')
            file1.close()
            # output=subprocess.check_output(['bash', 'run.sh'],cwd="KDDdemo/static/src/c") ##This is the output after calling bash
            # output = str(output, 'utf-8','ignore') ## Before this line, the output is in byte format. We change it to the string so that we can modify it later
            subprocess.call(['bash', 'run.sh'],cwd="KDDdemo/static/src/c")
            next_link = 'table.html'
            print(12345789)

            return HttpResponseRedirect(next_link)


    form = ContactForm() ##Don't delete it
    return render(request, 'index.html', {'form': form})
def contact(request):
    if request.method == 'POST':
        form = ContactForm(request.POST)
        if form.is_valid():
            input = form.cleaned_data['name']
            # email = form.cleaned_data['email']
            file1 = open("KDDdemo/static/src/papers/topics_dm.txt","w")
            name = input.split(",")
            for i in range(0,len(name)):
                temp = name[i].strip()
                temp.replace(" ", "_")
                file1.write(temp + '\n')
            file1.close()
            # output=subprocess.check_output(['bash', 'run.sh'],cwd="KDDdemo/static/src/c") ##This is the output after calling bash
            # output = str(output, 'utf-8','ignore') ## Before this line, the output is in byte format. We change it to the string so that we can modify it later
            subprocess.call(['bash', 'run.sh'],cwd="KDDdemo/static/src/c")
            next_link = 'table.html'
            print(12345789)



    form = ContactForm() ##Don't delete it
    return render(request, 'table.html', {'form':form})
# def table(request):
#     return render(request, 'table.html')
