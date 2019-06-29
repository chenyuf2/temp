from django.shortcuts import render, render_to_response, redirect
from django.http import HttpResponse, HttpResponseRedirect
from .forms import ContactForm
import subprocess
from django.contrib import messages

# Create your views here.
def index(request):
    if request.method == 'POST':
        form = ContactForm(request.POST)

        if form.is_valid():

            input = form.cleaned_data['name']
            # email = form.cleaned_data['email']
            file1 = open("KDDdemo/static/src/papers/topics_dm.txt","w")
            file2 = open( "KDDdemo/static/src/c/vocabs.txt", "r" )
            words = []
            for line in file2:
                words.append(line[0:-1])
            name = input.split(",")
            elements_not_in_words = ""
            for i in range(0,len(name)):
                temp = name[i].strip()
                new_string = ""
                for j in range(0,len(temp)):
                    if (temp[j]==" "):
                        new_string += "_"
                    else:
                        new_string += temp[j]
                if (new_string not in words):
                    elements_not_in_words += "'"+ temp+"', "
                file1.write(new_string + '\n')
            file1.close()
            file2.close()
            if (len(elements_not_in_words) > 0):
                print(elements_not_in_words[0])
                messages.info(request, elements_not_in_words[:-2])
                return render(request, 'index.html', {'form':form})
            else :
            # output=subprocess.check_output(['bash', 'run.sh'],cwd="KDDdemo/static/src/c") ##This is the output after calling bash
            # output = str(output, 'utf-8','ignore') ## Before this line, the output is in byte format. We change it to the string so that we can modify it later
                print(123123123)
                subprocess.call(['bash', 'run.sh'],cwd="KDDdemo/static/src/c")
                next_link = 'table.html'

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
            file2 = open( "KDDdemo/static/src/c/vocabs.txt", "r" )
            words = []
            for line in file2:
                words.append(line[0:-1])
            name = input.split(",")
            elements_not_in_words = ""
            for i in range(0,len(name)):
                temp = name[i].strip()
                new_string= ""
                for j in range(0,len(temp)):
                    if (temp[j] == " "):
                        new_string += "_"
                    else:
                        new_string += temp[j]
                if (new_string not in words):
                    elements_not_in_words += "'"+ temp+"', "
                file1.write(new_string + '\n')
            file1.close()
            file2.close()
            if (len(elements_not_in_words) > 0):
                print(elements_not_in_words[0])
                messages.info(request, elements_not_in_words[:-2])
                return render(request, 'table.html', {'form':form})
            else:
            # output=subprocess.check_output(['bash', 'run.sh'],cwd="KDDdemo/static/src/c") ##This is the output after calling bash
            # output = str(output, 'utf-8','ignore') ## Before this line, the output is in byte format. We change it to the string so that we can modify it later
                subprocess.call(['bash', 'run.sh'],cwd="KDDdemo/static/src/c")
                next_link = 'table.html'
                print(12345789)
    form = ContactForm() ##Don't delete it
    return render(request, 'table.html', {'form':form})
# def table(request):
#     return render(request, 'table.html')
