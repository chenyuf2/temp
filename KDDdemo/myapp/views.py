from django.shortcuts import render, render_to_response, redirect
from django.http import HttpResponse, HttpResponseRedirect
from .forms import ContactForm
import subprocess
from django.contrib import messages
import threading
import time
# Create your views here.
def emptytxt():
    tempfile = open("KDDdemo/static/src/papers/emb_dm_w_tmp.txt","w")
    tempfile.write("")
    tempfile.close()
    return
def index(request):
    tempfile =  open("KDDdemo/static/js/loadingornot.txt","w")
    tempfile.write("false")
    tempfile.close()
    if request.method == "POST":
        country_list = ('data mining,text mining,machine learning', 'physics, computer science, electrical engineering')
        form = ContactForm(request.POST, data_list = country_list)
        if form.is_valid():
            input = form.cleaned_data['temp']
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
                    if (temp[j] == " "):
                        new_string += "_"
                    else:
                        new_string += temp[j]
                if (new_string not in words):
                    elements_not_in_words += "'" + temp + "', "
                file1.write(new_string + '\n')
            file1.close()
            file2.close()
            if (len(elements_not_in_words) > 0):
                print(elements_not_in_words[0])
                messages.info(request, elements_not_in_words[:-2])
                return render(request, 'index.html', {'form':form})
            else:
    # output=subprocess.check_output(['bash', 'run.sh'],cwd="KDDdemo/static/src/c") ##This is the output after calling bash
    # output = str(output, 'utf-8','ignore') ## Before this line, the output is in byte format. We change it to the string so that we can modify it later
                # t1 = threading.Thread(target=jump, args=(request,form,))
                # t2 = threading.Thread(target=callfunc)
                # t1.start()
                # t2.start()
                # print(t1.join())
                # print(t2.join())
                # return redirect('table.html')

                # return jump(request, form)
                tempfile =  open("KDDdemo/static/js/loadingornot.txt","w")
                tempfile.write("true")
                tempfile.close()
                emptytxt()
                # subprocess.call(['bash', 'run.sh'],cwd="KDDdemo/static/src/c")
                print(12345789)
                tempfile =  open("KDDdemo/static/js/loadingornot.txt","w")
                tempfile.write("false")
                tempfile.close()
                return redirect('loader.html')
    country_list = ('data mining,text mining,machine learning', 'physics, computer science, electrical engineering')
    form = ContactForm(data_list = country_list)
    return render(request, 'index.html', {'form': form})
def contact(request):
    tempfile =  open("KDDdemo/static/js/loadingornot.txt","w")
    tempfile.write("false")
    tempfile.close()
    if request.method == 'POST':
        country_list = ('data mining,text mining,machine learning', 'physics, computer science, electrical engineering')
        form = ContactForm(request.POST, data_list = country_list)
        if form.is_valid():
            input = form.cleaned_data['temp']
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
                tempfile =  open("KDDdemo/static/js/loadingornot.txt","w")
                tempfile.write("true")
                tempfile.close()
                emptytxt()
                subprocess.call(['bash', 'run.sh'],cwd="KDDdemo/static/src/c")
                print(12345789)
                tempfile =  open("KDDdemo/static/js/loadingornot.txt","w")
                tempfile.write("false")
                tempfile.close()
                return render(request,'table.html', {'form':form})
    country_list = ('data mining,text mining,machine learning', 'physics, computer science, electrical engineering')
    form = ContactForm(data_list = country_list)
    # form = ContactForm() ##Don't delete it
    return render(request, 'table.html', {'form':form})
# def table(request):
#     return render(request, 'table.html')
def loader(request):
    return render(request, "loader.html")
